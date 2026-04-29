from fastmcp import FastMCP
import argparse
from typing import Dict, List, Optional

import simplepyble


class BleState:
    def __init__(self) -> None:
        self.adapters: List[simplepyble.Adapter] = simplepyble.Adapter.get_adapters()
        self.adapter: Optional[simplepyble.Adapter] = self.adapters[0] if self.adapters else None
        self.peripherals: Dict[str, simplepyble.Peripheral] = {}
        self.scan_results: List[simplepyble.Peripheral] = []
        self.notifications: Dict[str, List[Dict]] = {}

    def refresh_adapters(self) -> None:
        self.adapters = simplepyble.Adapter.get_adapters()
        if self.adapters and not self.adapter:
            self.adapter = self.adapters[0]


ble_state = BleState()
mcp = FastMCP(name="SimpleBLE MCP Server")


@mcp.tool(
    name="bluetooth_enabled",
    description="Check if Bluetooth is enabled on the host system.",
    tags={"adapter", "ble", "read", "status"},
    annotations={
        "title": "Bluetooth Enabled",
        "readOnlyHint": True,
        "idempotentHint": True,
        "openWorldHint": True,
    },
    meta={"version": "1.0", "role": "discovery"},
)
def bluetooth_enabled() -> Dict[str, bool]:
    """Check if Bluetooth is enabled and available."""
    enabled = simplepyble.Adapter.bluetooth_enabled()
    return {"enabled": enabled}


@mcp.tool(
    name="get_adapters",
    description="List available Bluetooth adapters on the host.",
    tags={"adapter", "ble", "read"},
    annotations={
        "title": "Get Adapters",
        "readOnlyHint": True,
        "idempotentHint": True,
        "openWorldHint": True,
    },
    meta={"version": "1.0", "role": "discovery"},
)
def get_adapters() -> List[Dict[str, str]]:
    """List all available Bluetooth adapters."""
    ble_state.refresh_adapters()
    return [{"identifier": a.identifier(), "address": a.address()} for a in ble_state.adapters]


@mcp.tool(
    name="scan_for",
    description="Scan for nearby BLE peripherals using the first available adapter.",
    tags={"adapter", "scan", "ble", "read"},
    annotations={
        "title": "Scan For",
        "readOnlyHint": True,
        "idempotentHint": False,
        "openWorldHint": True,
    },
    meta={"version": "1.0", "role": "discovery"},
)
def scan_for(timeout_ms: int = 5000) -> List[Dict]:
    """Scan for nearby BLE devices using the first available adapter."""
    ble_state.refresh_adapters()
    if not ble_state.adapter:
        raise RuntimeError("No Bluetooth adapter available")

    ble_state.adapter.scan_for(timeout_ms)
    ble_state.scan_results = ble_state.adapter.scan_get_results()

    results: List[Dict] = []
    for peripheral in ble_state.scan_results:
        manufacturer_data: Dict[str, str] = {}
        for company_id, data in peripheral.manufacturer_data().items():
            manufacturer_data[str(company_id)] = data.hex()

        results.append(
            {
                "identifier": peripheral.identifier(),
                "address": peripheral.address(),
                "rssi": peripheral.rssi(),
                "connectable": peripheral.is_connectable(),
                "manufacturer_data": manufacturer_data,
            }
        )
    return results


@mcp.tool(
    name="connect",
    description="Connect to a peripheral previously discovered in the last scan.",
    tags={"peripheral", "connect", "ble"},
    annotations={
        "title": "Connect",
        "readOnlyHint": False,
        "destructiveHint": False,
        "idempotentHint": False,
        "openWorldHint": True,
    },
    meta={"version": "1.0", "role": "connection"},
)
def connect(address: str) -> Dict[str, str]:
    """Connect to a peripheral from the last scan results."""
    if address in ble_state.peripherals:
        peripheral = ble_state.peripherals[address]
        if peripheral.is_connected():
            return {"message": f"Already connected to {peripheral.identifier()}", "address": address}

    target: Optional[simplepyble.Peripheral] = None
    for peripheral in ble_state.scan_results:
        if peripheral.address() == address:
            target = peripheral
            break

    if not target:
        if address in ble_state.peripherals:
            target = ble_state.peripherals[address]
        else:
            raise RuntimeError("Device not found in scan results. Please scan first.")

    try:
        target.connect()
    except Exception as exc:
        raise RuntimeError(f"Failed to connect: {exc}") from exc

    ble_state.peripherals[address] = target
    ble_state.notifications.setdefault(address, [])
    return {"message": f"Connected to {target.identifier()}", "address": address}


@mcp.tool(
    name="disconnect",
    description="Disconnect from a connected peripheral.",
    tags={"peripheral", "disconnect", "ble"},
    annotations={
        "title": "Disconnect",
        "readOnlyHint": False,
        "destructiveHint": False,
        "idempotentHint": False,
        "openWorldHint": True,
    },
    meta={"version": "1.0", "role": "connection"},
)
def disconnect(address: str) -> Dict[str, str]:
    """Disconnect from a connected peripheral."""
    if address not in ble_state.peripherals:
        raise RuntimeError("Device not found")

    peripheral = ble_state.peripherals[address]
    try:
        peripheral.disconnect()
    except Exception as exc:
        raise RuntimeError(f"Failed to disconnect: {exc}") from exc

    ble_state.notifications.pop(address, None)
    return {"message": f"Disconnected from {peripheral.identifier()}", "address": address}


@mcp.tool(
    name="services",
    description="List services and characteristics on a connected peripheral.",
    tags={"peripheral", "gatt", "ble", "read"},
    annotations={
        "title": "Services",
        "readOnlyHint": True,
        "idempotentHint": True,
        "openWorldHint": True,
    },
    meta={"version": "1.0", "role": "gatt"},
)
def services(address: str) -> Dict:
    """List services and characteristics for a connected device."""
    if address not in ble_state.peripherals:
        raise RuntimeError("Device not found or not connected")

    peripheral = ble_state.peripherals[address]
    if not peripheral.is_connected():
        return {"address": address, "connected": False}

    services = []
    for service in peripheral.services():
        characteristics = [char.uuid() for char in service.characteristics()]
        services.append({"uuid": service.uuid(), "characteristics": characteristics})

    return {
        "identifier": peripheral.identifier(),
        "address": address,
        "connected": peripheral.is_connected(),
        "mtu": peripheral.mtu(),
        "services": services,
    }


@mcp.tool(
    name="read",
    description="Read a characteristic value from a connected peripheral.",
    tags={"peripheral", "gatt", "ble", "read"},
    annotations={
        "title": "Read",
        "readOnlyHint": True,
        "idempotentHint": False,
        "openWorldHint": True,
    },
    meta={"version": "1.0", "role": "gatt"},
)
def read(address: str, service_uuid: str, char_uuid: str) -> Dict[str, str]:
    """Read a characteristic value from a connected device."""
    if address not in ble_state.peripherals:
        raise RuntimeError("Device not found")

    peripheral = ble_state.peripherals[address]
    if not peripheral.is_connected():
        raise RuntimeError("Device not connected")

    try:
        data = peripheral.read(service_uuid, char_uuid)
    except Exception as exc:  # pragma: no cover - backend-specific errors
        raise RuntimeError(f"Read failed: {exc}") from exc

    return {
        "service_uuid": service_uuid,
        "char_uuid": char_uuid,
        "data_hex": data.hex(),
        "data_utf8": data.decode("utf-8", errors="ignore"),
    }


@mcp.tool(
    name="write_request",
    description="Write data to a characteristic with response (write request). Data must be a hex string.",
    tags={"peripheral", "gatt", "ble", "write"},
    annotations={
        "title": "Write Request",
        "readOnlyHint": False,
        "destructiveHint": False,
        "idempotentHint": False,
        "openWorldHint": True,
    },
    meta={"version": "1.0", "role": "gatt"},
)
def write_request(address: str, service_uuid: str, char_uuid: str, data: str) -> Dict[str, str]:
    """Write data to a characteristic with response. Data is a hex string."""
    if address not in ble_state.peripherals:
        raise RuntimeError("Device not found")

    peripheral = ble_state.peripherals[address]
    if not peripheral.is_connected():
        raise RuntimeError("Device not connected")

    try:
        data_bytes = bytes.fromhex(data)
    except ValueError as exc:
        raise RuntimeError(f"Invalid hex string: {exc}") from exc

    try:
        peripheral.write_request(service_uuid, char_uuid, data_bytes)
    except Exception as exc:  # pragma: no cover - backend-specific errors
        raise RuntimeError(f"Write failed: {exc}") from exc

    return {"message": "Write successful"}


@mcp.tool(
    name="write_command",
    description="Write data to a characteristic without response (write command). Data must be a hex string.",
    tags={"peripheral", "gatt", "ble", "write"},
    annotations={
        "title": "Write Command",
        "readOnlyHint": False,
        "destructiveHint": False,
        "idempotentHint": False,
        "openWorldHint": True,
    },
    meta={"version": "1.0", "role": "gatt"},
)
def write_command(address: str, service_uuid: str, char_uuid: str, data: str) -> Dict[str, str]:
    """Write data to a characteristic without response. Data is a hex string."""
    if address not in ble_state.peripherals:
        raise RuntimeError("Device not found")

    peripheral = ble_state.peripherals[address]
    if not peripheral.is_connected():
        raise RuntimeError("Device not connected")

    try:
        data_bytes = bytes.fromhex(data)
    except ValueError as exc:
        raise RuntimeError(f"Invalid hex string: {exc}") from exc

    try:
        peripheral.write_command(service_uuid, char_uuid, data_bytes)
    except Exception as exc:  # pragma: no cover - backend-specific errors
        raise RuntimeError(f"Write command failed: {exc}") from exc

    return {"message": "Write command successful"}


@mcp.tool(
    name="notify",
    description="Subscribe to notifications on a characteristic. Data is buffered and can be retrieved with get_notifications. Call unsubscribe when done.",
    tags={"peripheral", "gatt", "ble", "notify"},
    annotations={
        "title": "Notify",
        "readOnlyHint": False,
        "destructiveHint": False,
        "idempotentHint": False,
        "openWorldHint": True,
    },
    meta={"version": "1.0", "role": "gatt"},
)
def notify(address: str, service_uuid: str, char_uuid: str) -> Dict[str, str]:
    """Subscribe to notifications. Data is buffered in the background."""
    if address not in ble_state.peripherals:
        raise RuntimeError("Device not found")

    peripheral = ble_state.peripherals[address]
    if not peripheral.is_connected():
        raise RuntimeError("Device not connected")

    def notification_callback(data: bytes) -> None:
        if address in ble_state.notifications:
            ble_state.notifications[address].append(
                {
                    "service": service_uuid,
                    "characteristic": char_uuid,
                    "data_hex": data.hex(),
                    "data_utf8": data.decode("utf-8", errors="ignore"),
                    "type": "notification",
                }
            )

    try:
        peripheral.notify(service_uuid, char_uuid, notification_callback)
    except Exception as exc:  # pragma: no cover - backend-specific errors
        raise RuntimeError(f"Notify failed: {exc}") from exc

    return {"message": "Subscribed to notifications"}


@mcp.tool(
    name="indicate",
    description="Subscribe to indications on a characteristic. Data is buffered and can be retrieved with get_notifications. Call unsubscribe when done.",
    tags={"peripheral", "gatt", "ble", "indicate"},
    annotations={
        "title": "Indicate",
        "readOnlyHint": False,
        "destructiveHint": False,
        "idempotentHint": False,
        "openWorldHint": True,
    },
    meta={"version": "1.0", "role": "gatt"},
)
def indicate(address: str, service_uuid: str, char_uuid: str) -> Dict[str, str]:
    """Subscribe to indications. Data is buffered in the background."""
    if address not in ble_state.peripherals:
        raise RuntimeError("Device not found")

    peripheral = ble_state.peripherals[address]
    if not peripheral.is_connected():
        raise RuntimeError("Device not connected")

    def indication_callback(data: bytes) -> None:
        if address in ble_state.notifications:
            ble_state.notifications[address].append(
                {
                    "service": service_uuid,
                    "characteristic": char_uuid,
                    "data_hex": data.hex(),
                    "data_utf8": data.decode("utf-8", errors="ignore"),
                    "type": "indication",
                }
            )

    try:
        peripheral.indicate(service_uuid, char_uuid, indication_callback)
    except Exception as exc:  # pragma: no cover - backend-specific errors
        raise RuntimeError(f"Indicate failed: {exc}") from exc

    return {"message": "Subscribed to indications"}


@mcp.tool(
    name="get_notifications",
    description="Retrieve and clear all buffered notifications and indications for a connected peripheral.",
    tags={"peripheral", "gatt", "ble", "read"},
    annotations={
        "title": "Get Notifications",
        "readOnlyHint": False,
        "destructiveHint": False,
        "idempotentHint": False,
        "openWorldHint": True,
    },
    meta={"version": "1.0", "role": "gatt"},
)
def get_notifications(address: str) -> List[Dict[str, str]]:
    """Return all buffered notifications/indications and clear the buffer."""
    if address not in ble_state.notifications:
        return []

    samples = ble_state.notifications[address]
    ble_state.notifications[address] = []
    return samples


@mcp.tool(
    name="unsubscribe",
    description="Unsubscribe from notifications or indications on a characteristic.",
    tags={"peripheral", "gatt", "ble"},
    annotations={
        "title": "Unsubscribe",
        "readOnlyHint": False,
        "destructiveHint": False,
        "idempotentHint": False,
        "openWorldHint": True,
    },
    meta={"version": "1.0", "role": "gatt"},
)
def unsubscribe(address: str, service_uuid: str, char_uuid: str) -> Dict[str, str]:
    """Unsubscribe from notifications or indications on a characteristic."""
    if address not in ble_state.peripherals:
        raise RuntimeError("Device not found")

    peripheral = ble_state.peripherals[address]
    if not peripheral.is_connected():
        raise RuntimeError("Device not connected")

    try:
        peripheral.unsubscribe(service_uuid, char_uuid)
    except Exception as exc:  # pragma: no cover - backend-specific errors
        raise RuntimeError(f"Unsubscribe failed: {exc}") from exc

    return {"message": "Unsubscribed"}


def main() -> None:
    parser = argparse.ArgumentParser(description="SimpleAIBLE MCP Server")
    parser.add_argument("--transport", default="stdio", choices=["stdio", "http"])
    parser.add_argument("--host", default="127.0.0.1", help="Host to bind to")
    parser.add_argument("--port", type=int, default=8000, help="Port to bind to")
    args = parser.parse_args()

    if args.transport == "http":
        mcp.run(transport="http", host=args.host, port=args.port)
    else:
        mcp.run()


if __name__ == "__main__":
    main()
