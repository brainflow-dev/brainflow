#pragma once

#include <stdexcept>
#include <string>

namespace SimpleDBus {

namespace Exception {

class BaseException : public std::exception {};

class NotInitialized : public BaseException {
  public:
    NotInitialized();
    const char* what() const noexcept override;
};

class DBusException : public BaseException {
  public:
    DBusException(const std::string& err_name, const std::string& err_message);
    const char* what() const noexcept override;

  private:
    std::string _message;
};

class SendFailed : public BaseException {
  public:
    SendFailed(const std::string& err_name, const std::string& err_message, const std::string& msg_str);
    const char* what() const noexcept override;

  private:
    std::string _message;
};

class InterfaceNotFoundException : public BaseException {
  public:
    InterfaceNotFoundException(const std::string& path, const std::string& interface);
    const char* what() const noexcept override;

  private:
    std::string _message;
};

class PathNotFoundException : public BaseException {
  public:
    PathNotFoundException(const std::string& path, const std::string& subpath);
    const char* what() const noexcept override;

  private:
    std::string _message;
};

}  // namespace Exception

}  // namespace SimpleDBus
