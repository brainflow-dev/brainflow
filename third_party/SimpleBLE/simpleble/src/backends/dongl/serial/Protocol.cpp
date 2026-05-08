#include "Protocol.h"

#include <cstring>
#include "fmt/base.h"
#include "protocol/simpleble.pb.h"

using namespace SimpleBLE::Dongl::Serial;

Protocol::Protocol(const std::string& device_path) : ProtocolBase(device_path) {}

Protocol::~Protocol() {}

basic_WhoamiRsp Protocol::basic_whoami() {
    dongl_Command command = dongl_Command_init_zero;
    command.which_cmd = dongl_Command_basic_tag;
    command.cmd.basic.which_cmd = basic_Command_whoami_tag;
    basic_WhoamiCmd whoami_cmd = basic_WhoamiCmd_init_default;
    command.cmd.basic.cmd.whoami = whoami_cmd;

    dongl_Response response = exchange(command);
    return response.rsp.basic.rsp.whoami;
}

basic_ResetRsp Protocol::basic_reset() {
    dongl_Command command = dongl_Command_init_zero;
    command.which_cmd = dongl_Command_basic_tag;
    command.cmd.basic.which_cmd = basic_Command_reset_tag;
    basic_ResetCmd reset_cmd = basic_ResetCmd_init_default;
    command.cmd.basic.cmd.reset = reset_cmd;

    dongl_Response response = exchange(command);
    return response.rsp.basic.rsp.reset;
}

basic_DfuStartRsp Protocol::basic_dfu_start() {
    dongl_Command command = dongl_Command_init_zero;
    command.which_cmd = dongl_Command_basic_tag;
    command.cmd.basic.which_cmd = basic_Command_dfu_start_tag;
    basic_DfuStartCmd dfu_start_cmd = basic_DfuStartCmd_init_default;
    command.cmd.basic.cmd.dfu_start = dfu_start_cmd;

    dongl_Response response = exchange(command);
    return response.rsp.basic.rsp.dfu_start;
}

simpleble_InitRsp Protocol::simpleble_init() {
    dongl_Command command = dongl_Command_init_zero;
    command.which_cmd = dongl_Command_simpleble_tag;
    command.cmd.simpleble.which_cmd = simpleble_Command_init_tag;
    simpleble_InitCmd init_cmd = simpleble_InitCmd_init_default;
    command.cmd.simpleble.cmd.init = init_cmd;

    dongl_Response response = exchange(command);
    return response.rsp.simpleble.rsp.init;
}

simpleble_ScanStartRsp Protocol::simpleble_scan_start() {
    dongl_Command command = dongl_Command_init_zero;
    command.which_cmd = dongl_Command_simpleble_tag;
    command.cmd.simpleble.which_cmd = simpleble_Command_scan_start_tag;
    simpleble_ScanStartCmd scan_start_cmd = simpleble_ScanStartCmd_init_default;
    command.cmd.simpleble.cmd.scan_start = scan_start_cmd;

    dongl_Response response = exchange(command);
    return response.rsp.simpleble.rsp.scan_start;
}

simpleble_ScanStopRsp Protocol::simpleble_scan_stop() {
    dongl_Command command = dongl_Command_init_zero;
    command.which_cmd = dongl_Command_simpleble_tag;
    command.cmd.simpleble.which_cmd = simpleble_Command_scan_stop_tag;
    simpleble_ScanStopCmd scan_stop_cmd = simpleble_ScanStopCmd_init_default;
    command.cmd.simpleble.cmd.scan_stop = scan_stop_cmd;

    dongl_Response response = exchange(command);
    return response.rsp.simpleble.rsp.scan_stop;
}

simpleble_ConnectRsp Protocol::simpleble_connect(simpleble_BluetoothAddressType address_type, const std::string& address) {
    dongl_Command command = dongl_Command_init_zero;
    command.which_cmd = dongl_Command_simpleble_tag;
    command.cmd.simpleble.which_cmd = simpleble_Command_connect_tag;
    simpleble_ConnectCmd connect_cmd = simpleble_ConnectCmd_init_default;
    command.cmd.simpleble.cmd.connect = connect_cmd;

    command.cmd.simpleble.cmd.connect.address_type = address_type;
    strncpy(command.cmd.simpleble.cmd.connect.address, address.c_str(), sizeof(command.cmd.simpleble.cmd.connect.address));

    dongl_Response response = exchange(command);
    return response.rsp.simpleble.rsp.connect;
}

simpleble_DisconnectRsp Protocol::simpleble_disconnect(uint16_t conn_handle) {
    dongl_Command command = dongl_Command_init_zero;
    command.which_cmd = dongl_Command_simpleble_tag;
    command.cmd.simpleble.which_cmd = simpleble_Command_disconnect_tag;
    simpleble_DisconnectCmd disconnect_cmd = simpleble_DisconnectCmd_init_default;
    command.cmd.simpleble.cmd.disconnect = disconnect_cmd;

    dongl_Response response = exchange(command);
    return response.rsp.simpleble.rsp.disconnect;
}

simpleble_ReadRsp Protocol::simpleble_read(uint16_t conn_handle, uint16_t handle) {
    dongl_Command command = dongl_Command_init_zero;
    command.which_cmd = dongl_Command_simpleble_tag;
    command.cmd.simpleble.which_cmd = simpleble_Command_read_tag;
    simpleble_ReadCmd read_cmd = simpleble_ReadCmd_init_default;
    command.cmd.simpleble.cmd.read = read_cmd;
    command.cmd.simpleble.cmd.read.conn_handle = conn_handle;
    command.cmd.simpleble.cmd.read.handle = handle;

    fmt::print("simpleble_read: conn_handle: {}, handle: {}\n", conn_handle, handle);
    dongl_Response response = exchange(command);
    fmt::print("simpleble_read: response: {}\n", response.rsp.simpleble.rsp.read.ret_code);
    return response.rsp.simpleble.rsp.read;
}

simpleble_WriteRsp Protocol::simpleble_write(uint16_t conn_handle, uint16_t handle, simpleble_WriteOperation operation, const std::vector<uint8_t>& data) {
    dongl_Command command = dongl_Command_init_zero;
    command.which_cmd = dongl_Command_simpleble_tag;
    command.cmd.simpleble.which_cmd = simpleble_Command_write_tag;
    simpleble_WriteCmd write_cmd = simpleble_WriteCmd_init_default;
    command.cmd.simpleble.cmd.write = write_cmd;
    command.cmd.simpleble.cmd.write.conn_handle = conn_handle;
    command.cmd.simpleble.cmd.write.handle = handle;
    command.cmd.simpleble.cmd.write.op = operation;
    command.cmd.simpleble.cmd.write.data.size = data.size();
    memcpy(command.cmd.simpleble.cmd.write.data.bytes, data.data(), data.size());

    dongl_Response response = exchange(command);
    return response.rsp.simpleble.rsp.write;
}