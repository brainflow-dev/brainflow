#pragma once

#include <simpledbus/base/Holder.h>
#include <simpledbus/base/Message.h>

#include <functional>
#include <string>

namespace SimpleDBus {

namespace Interfaces {

class PropertyHandler {
  private:
    std::string _path;

  protected:
    virtual void add_option(std::string option_name, Holder value) = 0;
    virtual void remove_option(std::string option_name) = 0;

  public:
    PropertyHandler(std::string path);
    virtual ~PropertyHandler();

    void set_options(Holder changed_properties);
    void set_options(Holder changed_properties, Holder invalidated_properties);

    std::function<void(std::string interface, Holder changed, Holder invalidated)> PropertiesChanged;
    bool process_received_signal(Message& message);

};
}  // namespace Interfaces

}  // namespace SimpleDBus
