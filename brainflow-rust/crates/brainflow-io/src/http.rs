//! Simple HTTP client abstraction.
//!
//! Provides minimal HTTP client functionality for WiFi-based biosensor devices.

use alloc::string::String;
use alloc::vec::Vec;

use crate::IoError;

/// HTTP response.
#[derive(Debug, Clone)]
pub struct HttpResponse {
    /// HTTP status code.
    pub status_code: u16,
    /// Response body.
    pub body: Vec<u8>,
    /// Content-Type header value.
    pub content_type: Option<String>,
}

impl HttpResponse {
    /// Get the body as a UTF-8 string.
    pub fn body_str(&self) -> Option<&str> {
        core::str::from_utf8(&self.body).ok()
    }

    /// Check if the response indicates success (2xx status).
    pub fn is_success(&self) -> bool {
        (200..300).contains(&self.status_code)
    }
}

/// HTTP client trait.
pub trait HttpClient: Send + Sync {
    /// Perform a GET request.
    fn get(&self, url: &str) -> Result<HttpResponse, IoError>;

    /// Perform a POST request with JSON body.
    fn post(&self, url: &str, body: &str) -> Result<HttpResponse, IoError>;

    /// Perform a POST request with form data.
    fn post_form(&self, url: &str, data: &[(&str, &str)]) -> Result<HttpResponse, IoError>;

    /// Set the request timeout in milliseconds.
    fn set_timeout(&mut self, timeout_ms: u32);
}

/// Standard library HTTP client using std::net.
#[cfg(feature = "std")]
pub mod std_http {
    use super::*;
    use alloc::string::ToString;
    use std::format;
    use std::io::{Read, Write};
    use std::net::TcpStream;
    use std::time::Duration;

    /// Simple HTTP client using raw TCP sockets.
    pub struct SimpleHttpClient {
        timeout_ms: u32,
    }

    impl SimpleHttpClient {
        /// Create a new HTTP client.
        pub fn new() -> Self {
            Self { timeout_ms: 5000 }
        }

        /// Parse a URL into host, port, and path components.
        pub fn parse_url(url: &str) -> Option<(String, u16, String)> {
            let url = url.strip_prefix("http://").unwrap_or(url);

            let (host_port, path) = if let Some(slash_pos) = url.find('/') {
                (&url[..slash_pos], &url[slash_pos..])
            } else {
                (url, "/")
            };

            let (host, port) = if let Some(colon_pos) = host_port.find(':') {
                let port_str = &host_port[colon_pos + 1..];
                let port = port_str.parse().ok()?;
                (&host_port[..colon_pos], port)
            } else {
                (host_port, 80u16)
            };

            Some((host.to_string(), port, path.to_string()))
        }

        /// Parse HTTP response.
        fn parse_response(data: &[u8]) -> Option<HttpResponse> {
            let data_str = core::str::from_utf8(data).ok()?;

            // Find headers/body separator
            let header_end = data_str.find("\r\n\r\n")?;
            let headers = &data_str[..header_end];
            let body = &data[header_end + 4..];

            // Parse status line
            let status_line = headers.lines().next()?;
            let parts: Vec<&str> = status_line.split_whitespace().collect();
            if parts.len() < 2 {
                return None;
            }
            let status_code: u16 = parts[1].parse().ok()?;

            // Parse Content-Type
            let content_type = headers
                .lines()
                .find(|line| line.to_lowercase().starts_with("content-type:"))
                .map(|line| line[13..].trim().to_string());

            Some(HttpResponse {
                status_code,
                body: body.to_vec(),
                content_type,
            })
        }
    }

    impl Default for SimpleHttpClient {
        fn default() -> Self {
            Self::new()
        }
    }

    impl HttpClient for SimpleHttpClient {
        fn get(&self, url: &str) -> Result<HttpResponse, IoError> {
            let (host, port, path) = Self::parse_url(url).ok_or(IoError::InvalidAddress)?;

            let addr = format!("{}:{}", host, port);
            let mut stream = TcpStream::connect(&addr).map_err(|_| IoError::ConnectionFailed)?;
            stream
                .set_read_timeout(Some(Duration::from_millis(self.timeout_ms as u64)))
                .ok();
            stream
                .set_write_timeout(Some(Duration::from_millis(self.timeout_ms as u64)))
                .ok();

            let request = format!(
                "GET {} HTTP/1.1\r\nHost: {}\r\nConnection: close\r\n\r\n",
                path, host
            );

            stream
                .write_all(request.as_bytes())
                .map_err(|_| IoError::WriteFailed)?;

            let mut response = Vec::new();
            stream
                .read_to_end(&mut response)
                .map_err(|_| IoError::ReadFailed)?;

            Self::parse_response(&response).ok_or(IoError::ReadFailed)
        }

        fn post(&self, url: &str, body: &str) -> Result<HttpResponse, IoError> {
            let (host, port, path) = Self::parse_url(url).ok_or(IoError::InvalidAddress)?;

            let addr = format!("{}:{}", host, port);
            let mut stream = TcpStream::connect(&addr).map_err(|_| IoError::ConnectionFailed)?;
            stream
                .set_read_timeout(Some(Duration::from_millis(self.timeout_ms as u64)))
                .ok();

            let request = format!(
                "POST {} HTTP/1.1\r\nHost: {}\r\nContent-Type: application/json\r\nContent-Length: {}\r\nConnection: close\r\n\r\n{}",
                path, host, body.len(), body
            );

            stream
                .write_all(request.as_bytes())
                .map_err(|_| IoError::WriteFailed)?;

            let mut response = Vec::new();
            stream
                .read_to_end(&mut response)
                .map_err(|_| IoError::ReadFailed)?;

            Self::parse_response(&response).ok_or(IoError::ReadFailed)
        }

        fn post_form(&self, url: &str, data: &[(&str, &str)]) -> Result<HttpResponse, IoError> {
            let body: String = data
                .iter()
                .map(|(k, v)| format!("{}={}", k, v))
                .collect::<Vec<_>>()
                .join("&");

            let (host, port, path) = Self::parse_url(url).ok_or(IoError::InvalidAddress)?;

            let addr = format!("{}:{}", host, port);
            let mut stream = TcpStream::connect(&addr).map_err(|_| IoError::ConnectionFailed)?;
            stream
                .set_read_timeout(Some(Duration::from_millis(self.timeout_ms as u64)))
                .ok();

            let request = format!(
                "POST {} HTTP/1.1\r\nHost: {}\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: {}\r\nConnection: close\r\n\r\n{}",
                path, host, body.len(), body
            );

            stream
                .write_all(request.as_bytes())
                .map_err(|_| IoError::WriteFailed)?;

            let mut response = Vec::new();
            stream
                .read_to_end(&mut response)
                .map_err(|_| IoError::ReadFailed)?;

            Self::parse_response(&response).ok_or(IoError::ReadFailed)
        }

        fn set_timeout(&mut self, timeout_ms: u32) {
            self.timeout_ms = timeout_ms;
        }
    }
}

/// No-std stub implementation.
#[cfg(not(feature = "std"))]
pub mod nostd_http {
    use super::*;

    /// Stub HTTP client for no_std environments.
    pub struct StubHttpClient;

    impl StubHttpClient {
        pub fn new() -> Self {
            Self
        }
    }

    impl HttpClient for StubHttpClient {
        fn get(&self, _url: &str) -> Result<HttpResponse, IoError> {
            Err(IoError::NotSupported)
        }

        fn post(&self, _url: &str, _body: &str) -> Result<HttpResponse, IoError> {
            Err(IoError::NotSupported)
        }

        fn post_form(&self, _url: &str, _data: &[(&str, &str)]) -> Result<HttpResponse, IoError> {
            Err(IoError::NotSupported)
        }

        fn set_timeout(&mut self, _timeout_ms: u32) {}
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_http_response() {
        let response = HttpResponse {
            status_code: 200,
            body: b"Hello World".to_vec(),
            content_type: Some("text/plain".into()),
        };

        assert!(response.is_success());
        assert_eq!(response.body_str(), Some("Hello World"));
    }

    #[test]
    fn test_http_response_error() {
        let response = HttpResponse {
            status_code: 404,
            body: b"Not Found".to_vec(),
            content_type: None,
        };

        assert!(!response.is_success());
    }

    #[cfg(feature = "std")]
    #[test]
    fn test_parse_url() {
        use std_http::SimpleHttpClient;

        let result = SimpleHttpClient::parse_url("http://example.com:8080/api/test");
        assert!(result.is_some());
        let (host, port, path) = result.unwrap();
        assert_eq!(host, "example.com");
        assert_eq!(port, 8080);
        assert_eq!(path, "/api/test");

        let result = SimpleHttpClient::parse_url("192.168.1.1/command");
        assert!(result.is_some());
        let (host, port, path) = result.unwrap();
        assert_eq!(host, "192.168.1.1");
        assert_eq!(port, 80);
        assert_eq!(path, "/command");
    }
}
