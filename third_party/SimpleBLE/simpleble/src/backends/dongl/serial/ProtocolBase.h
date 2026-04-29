#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include <chrono>
#include <condition_variable>
#include <mutex>

#include "Wire.h"

#include "protocol/d2h.pb.h"
#include "protocol/h2d.pb.h"

namespace SimpleBLE {
namespace Dongl {
namespace Serial {

class ProtocolBase {
  public:
    ProtocolBase(const std::string& device_path);
    ~ProtocolBase();

    /**
     * @brief Sends a command synchronously and waits for the response.
     * Only one exchange can be pending at a time.
     *
     * @param command The command to send.
     * @return The response when it arrives.
     * @throws std::runtime_error if another exchange is already pending or if timeout occurs.
     */
    dongl_Response exchange(const dongl_Command& command);

    /**
     * @brief Sets the callback for received events.
     *
     * @param callback Function to call when an event is received.
     */
    void set_event_callback(std::function<void(const dongl_Event&)> callback);

  private:
    std::unique_ptr<Wire> _wire;
    std::function<void(const dongl_Event&)> _event_callback;

    std::optional<dongl_Response> _pending_response;
    std::condition_variable _response_cv;
    std::mutex _pending_mutex;
};

}  // namespace Serial
}  // namespace Dongl
}  // namespace SimpleBLE
