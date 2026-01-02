//! TCP and UDP socket communication abstractions.
//!
//! Provides cross-platform socket communication for network-based biosensor devices.

use alloc::collections::VecDeque;
use alloc::string::String;
use alloc::vec::Vec;

use crate::IoError;

/// Socket configuration.
#[derive(Debug, Clone)]
pub struct SocketConfig {
    /// Connection/read timeout in milliseconds
    pub timeout_ms: u32,
    /// Receive buffer size
    pub recv_buffer_size: usize,
    /// Send buffer size
    pub send_buffer_size: usize,
    /// Disable Nagle's algorithm (TCP_NODELAY)
    pub no_delay: bool,
}

impl Default for SocketConfig {
    fn default() -> Self {
        Self {
            timeout_ms: 5000,
            recv_buffer_size: 65536 * 4,
            send_buffer_size: 65536,
            no_delay: true,
        }
    }
}

/// TCP client socket trait.
pub trait TcpClient: Send {
    /// Connect to a remote server.
    fn connect(&mut self, addr: &str, port: u16, config: &SocketConfig) -> Result<(), IoError>;

    /// Check if connected.
    fn is_connected(&self) -> bool;

    /// Send data.
    fn send(&mut self, data: &[u8]) -> Result<usize, IoError>;

    /// Receive data.
    fn recv(&mut self, buf: &mut [u8]) -> Result<usize, IoError>;

    /// Close the connection.
    fn close(&mut self) -> Result<(), IoError>;

    /// Get the remote address.
    fn remote_addr(&self) -> Option<(&str, u16)>;
}

/// TCP server socket trait.
pub trait TcpServer: Send {
    /// Bind to a local address and port.
    fn bind(&mut self, addr: &str, port: u16, config: &SocketConfig) -> Result<(), IoError>;

    /// Accept an incoming connection (blocking).
    fn accept(&mut self) -> Result<(), IoError>;

    /// Check if a client is connected.
    fn has_client(&self) -> bool;

    /// Send data to connected client.
    fn send(&mut self, data: &[u8]) -> Result<usize, IoError>;

    /// Receive data from connected client.
    fn recv(&mut self, buf: &mut [u8]) -> Result<usize, IoError>;

    /// Close the server and any client connections.
    fn close(&mut self) -> Result<(), IoError>;

    /// Get local port.
    fn local_port(&self) -> u16;
}

/// UDP socket trait.
pub trait UdpSocket: Send {
    /// Bind to a local port.
    fn bind(&mut self, port: u16) -> Result<(), IoError>;

    /// Connect to a remote address (for send/recv without specifying address).
    fn connect(&mut self, addr: &str, port: u16) -> Result<(), IoError>;

    /// Send data to connected address.
    fn send(&mut self, data: &[u8]) -> Result<usize, IoError>;

    /// Send data to a specific address.
    fn send_to(&mut self, data: &[u8], addr: &str, port: u16) -> Result<usize, IoError>;

    /// Receive data.
    fn recv(&mut self, buf: &mut [u8]) -> Result<usize, IoError>;

    /// Receive data with sender address.
    fn recv_from(&mut self, buf: &mut [u8]) -> Result<(usize, String, u16), IoError>;

    /// Set receive timeout.
    fn set_timeout(&mut self, timeout_ms: u32) -> Result<(), IoError>;

    /// Close the socket.
    fn close(&mut self) -> Result<(), IoError>;

    /// Get local port.
    fn local_port(&self) -> u16;
}

/// Standard library socket implementations.
#[cfg(feature = "std")]
pub mod std_sockets {
    use super::*;
    use alloc::string::ToString;
    use std::io::{Read, Write as IoWrite};
    use std::net::{TcpListener, TcpStream, UdpSocket as NetUdpSocket, ToSocketAddrs};
    use std::time::Duration;
    use std::format;

    /// TCP client using std::net.
    pub struct StdTcpClient {
        stream: Option<TcpStream>,
        remote_addr: String,
        remote_port: u16,
        buffer: VecDeque<u8>,
    }

    impl StdTcpClient {
        /// Create a new TCP client.
        pub fn new() -> Self {
            Self {
                stream: None,
                remote_addr: String::new(),
                remote_port: 0,
                buffer: VecDeque::new(),
            }
        }
    }

    impl Default for StdTcpClient {
        fn default() -> Self {
            Self::new()
        }
    }

    impl TcpClient for StdTcpClient {
        fn connect(&mut self, addr: &str, port: u16, config: &SocketConfig) -> Result<(), IoError> {
            let addr_str = format!("{}:{}", addr, port);
            let socket_addr = addr_str
                .to_socket_addrs()
                .map_err(|_| IoError::InvalidAddress)?
                .next()
                .ok_or(IoError::InvalidAddress)?;

            let stream = TcpStream::connect_timeout(
                &socket_addr,
                Duration::from_millis(config.timeout_ms as u64),
            )
            .map_err(|_| IoError::ConnectionFailed)?;

            stream
                .set_read_timeout(Some(Duration::from_millis(config.timeout_ms as u64)))
                .map_err(|_| IoError::ConfigError)?;
            stream
                .set_write_timeout(Some(Duration::from_millis(config.timeout_ms as u64)))
                .map_err(|_| IoError::ConfigError)?;
            stream
                .set_nodelay(config.no_delay)
                .map_err(|_| IoError::ConfigError)?;

            self.stream = Some(stream);
            self.remote_addr = addr.to_string();
            self.remote_port = port;
            Ok(())
        }

        fn is_connected(&self) -> bool {
            self.stream.is_some()
        }

        fn send(&mut self, data: &[u8]) -> Result<usize, IoError> {
            let stream = self.stream.as_mut().ok_or(IoError::NotConnected)?;
            stream.write(data).map_err(|_| IoError::WriteFailed)
        }

        fn recv(&mut self, buf: &mut [u8]) -> Result<usize, IoError> {
            let stream = self.stream.as_mut().ok_or(IoError::NotConnected)?;
            stream.read(buf).map_err(|e| {
                if e.kind() == std::io::ErrorKind::WouldBlock
                    || e.kind() == std::io::ErrorKind::TimedOut
                {
                    IoError::Timeout
                } else {
                    IoError::ReadFailed
                }
            })
        }

        fn close(&mut self) -> Result<(), IoError> {
            if let Some(stream) = self.stream.take() {
                stream.shutdown(std::net::Shutdown::Both).ok();
            }
            Ok(())
        }

        fn remote_addr(&self) -> Option<(&str, u16)> {
            if self.stream.is_some() {
                Some((&self.remote_addr, self.remote_port))
            } else {
                None
            }
        }
    }

    /// TCP server using std::net.
    pub struct StdTcpServer {
        listener: Option<TcpListener>,
        client: Option<TcpStream>,
        local_port: u16,
        config: SocketConfig,
    }

    impl StdTcpServer {
        /// Create a new TCP server.
        pub fn new() -> Self {
            Self {
                listener: None,
                client: None,
                local_port: 0,
                config: SocketConfig::default(),
            }
        }
    }

    impl Default for StdTcpServer {
        fn default() -> Self {
            Self::new()
        }
    }

    impl TcpServer for StdTcpServer {
        fn bind(&mut self, addr: &str, port: u16, config: &SocketConfig) -> Result<(), IoError> {
            let addr_str = format!("{}:{}", addr, port);
            let listener = TcpListener::bind(&addr_str).map_err(|_| IoError::BindFailed)?;

            // Set non-blocking for accept with timeout
            listener.set_nonblocking(false).ok();

            self.listener = Some(listener);
            self.local_port = port;
            self.config = config.clone();
            Ok(())
        }

        fn accept(&mut self) -> Result<(), IoError> {
            let listener = self.listener.as_ref().ok_or(IoError::NotConnected)?;
            let (stream, _) = listener.accept().map_err(|_| IoError::ConnectionFailed)?;

            stream
                .set_read_timeout(Some(Duration::from_millis(self.config.timeout_ms as u64)))
                .ok();
            stream
                .set_write_timeout(Some(Duration::from_millis(self.config.timeout_ms as u64)))
                .ok();
            stream.set_nodelay(self.config.no_delay).ok();

            self.client = Some(stream);
            Ok(())
        }

        fn has_client(&self) -> bool {
            self.client.is_some()
        }

        fn send(&mut self, data: &[u8]) -> Result<usize, IoError> {
            let client = self.client.as_mut().ok_or(IoError::NotConnected)?;
            client.write(data).map_err(|_| IoError::WriteFailed)
        }

        fn recv(&mut self, buf: &mut [u8]) -> Result<usize, IoError> {
            let client = self.client.as_mut().ok_or(IoError::NotConnected)?;
            client.read(buf).map_err(|e| {
                if e.kind() == std::io::ErrorKind::WouldBlock
                    || e.kind() == std::io::ErrorKind::TimedOut
                {
                    IoError::Timeout
                } else {
                    IoError::ReadFailed
                }
            })
        }

        fn close(&mut self) -> Result<(), IoError> {
            if let Some(client) = self.client.take() {
                client.shutdown(std::net::Shutdown::Both).ok();
            }
            self.listener = None;
            Ok(())
        }

        fn local_port(&self) -> u16 {
            self.local_port
        }
    }

    /// UDP socket using std::net.
    pub struct StdUdpSocket {
        socket: Option<NetUdpSocket>,
        local_port: u16,
        connected_addr: Option<String>,
        connected_port: u16,
    }

    impl StdUdpSocket {
        /// Create a new UDP socket.
        pub fn new() -> Self {
            Self {
                socket: None,
                local_port: 0,
                connected_addr: None,
                connected_port: 0,
            }
        }
    }

    impl Default for StdUdpSocket {
        fn default() -> Self {
            Self::new()
        }
    }

    impl UdpSocket for StdUdpSocket {
        fn bind(&mut self, port: u16) -> Result<(), IoError> {
            let addr = format!("0.0.0.0:{}", port);
            let socket = NetUdpSocket::bind(&addr).map_err(|_| IoError::BindFailed)?;
            self.socket = Some(socket);
            self.local_port = port;
            Ok(())
        }

        fn connect(&mut self, addr: &str, port: u16) -> Result<(), IoError> {
            let socket = self.socket.as_ref().ok_or(IoError::NotConnected)?;
            let addr_str = format!("{}:{}", addr, port);
            socket.connect(&addr_str).map_err(|_| IoError::ConnectionFailed)?;
            self.connected_addr = Some(addr.to_string());
            self.connected_port = port;
            Ok(())
        }

        fn send(&mut self, data: &[u8]) -> Result<usize, IoError> {
            let socket = self.socket.as_ref().ok_or(IoError::NotConnected)?;
            socket.send(data).map_err(|_| IoError::WriteFailed)
        }

        fn send_to(&mut self, data: &[u8], addr: &str, port: u16) -> Result<usize, IoError> {
            let socket = self.socket.as_ref().ok_or(IoError::NotConnected)?;
            let addr_str = format!("{}:{}", addr, port);
            socket.send_to(data, &addr_str).map_err(|_| IoError::WriteFailed)
        }

        fn recv(&mut self, buf: &mut [u8]) -> Result<usize, IoError> {
            let socket = self.socket.as_ref().ok_or(IoError::NotConnected)?;
            socket.recv(buf).map_err(|e| {
                if e.kind() == std::io::ErrorKind::WouldBlock
                    || e.kind() == std::io::ErrorKind::TimedOut
                {
                    IoError::Timeout
                } else {
                    IoError::ReadFailed
                }
            })
        }

        fn recv_from(&mut self, buf: &mut [u8]) -> Result<(usize, String, u16), IoError> {
            let socket = self.socket.as_ref().ok_or(IoError::NotConnected)?;
            let (len, addr) = socket.recv_from(buf).map_err(|e| {
                if e.kind() == std::io::ErrorKind::WouldBlock
                    || e.kind() == std::io::ErrorKind::TimedOut
                {
                    IoError::Timeout
                } else {
                    IoError::ReadFailed
                }
            })?;
            Ok((len, addr.ip().to_string(), addr.port()))
        }

        fn set_timeout(&mut self, timeout_ms: u32) -> Result<(), IoError> {
            let socket = self.socket.as_ref().ok_or(IoError::NotConnected)?;
            let timeout = if timeout_ms > 0 {
                Some(Duration::from_millis(timeout_ms as u64))
            } else {
                None
            };
            socket.set_read_timeout(timeout).map_err(|_| IoError::ConfigError)?;
            socket.set_write_timeout(timeout).map_err(|_| IoError::ConfigError)?;
            Ok(())
        }

        fn close(&mut self) -> Result<(), IoError> {
            self.socket = None;
            self.connected_addr = None;
            Ok(())
        }

        fn local_port(&self) -> u16 {
            self.local_port
        }
    }
}

/// No-std stub implementations.
#[cfg(not(feature = "std"))]
pub mod nostd_sockets {
    use super::*;

    /// Stub TCP client for no_std.
    pub struct StubTcpClient;

    impl StubTcpClient {
        pub fn new() -> Self {
            Self
        }
    }

    impl TcpClient for StubTcpClient {
        fn connect(&mut self, _addr: &str, _port: u16, _config: &SocketConfig) -> Result<(), IoError> {
            Err(IoError::NotSupported)
        }
        fn is_connected(&self) -> bool { false }
        fn send(&mut self, _data: &[u8]) -> Result<usize, IoError> { Err(IoError::NotSupported) }
        fn recv(&mut self, _buf: &mut [u8]) -> Result<usize, IoError> { Err(IoError::NotSupported) }
        fn close(&mut self) -> Result<(), IoError> { Ok(()) }
        fn remote_addr(&self) -> Option<(&str, u16)> { None }
    }

    /// Stub UDP socket for no_std.
    pub struct StubUdpSocket;

    impl StubUdpSocket {
        pub fn new() -> Self {
            Self
        }
    }

    impl UdpSocket for StubUdpSocket {
        fn bind(&mut self, _port: u16) -> Result<(), IoError> { Err(IoError::NotSupported) }
        fn connect(&mut self, _addr: &str, _port: u16) -> Result<(), IoError> { Err(IoError::NotSupported) }
        fn send(&mut self, _data: &[u8]) -> Result<usize, IoError> { Err(IoError::NotSupported) }
        fn send_to(&mut self, _data: &[u8], _addr: &str, _port: u16) -> Result<usize, IoError> { Err(IoError::NotSupported) }
        fn recv(&mut self, _buf: &mut [u8]) -> Result<usize, IoError> { Err(IoError::NotSupported) }
        fn recv_from(&mut self, _buf: &mut [u8]) -> Result<(usize, String, u16), IoError> { Err(IoError::NotSupported) }
        fn set_timeout(&mut self, _timeout_ms: u32) -> Result<(), IoError> { Err(IoError::NotSupported) }
        fn close(&mut self) -> Result<(), IoError> { Ok(()) }
        fn local_port(&self) -> u16 { 0 }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_socket_config_default() {
        let config = SocketConfig::default();
        assert_eq!(config.timeout_ms, 5000);
        assert!(config.no_delay);
    }
}
