#pragma once

#include <stdexcept>
#include <string>

#include <simpledbus/base/Exceptions.h>

namespace SimpleBluez {

namespace Exception {

class BaseException : public std::exception {};

class ServiceNotFoundException : public BaseException {
  public:
    ServiceNotFoundException(const std::string& service);
    const char* what() const noexcept override;

  private:
    std::string _message;
};

class CharacteristicNotFoundException : public BaseException {
  public:
    CharacteristicNotFoundException(const std::string& characteristic);
    const char* what() const noexcept override;

  private:
    std::string _message;
};

class DescriptorNotFoundException : public BaseException {
  public:
    DescriptorNotFoundException(const std::string& descriptor);
    const char* what() const noexcept override;

  private:
    std::string _message;
};

}  // namespace Exception

}  // namespace SimpleBluez
