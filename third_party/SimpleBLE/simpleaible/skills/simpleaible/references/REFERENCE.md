# SimpleAIBLE Reference

## Tool Catalog

### Adapter Management

- `get_adapters`: Lists all Bluetooth adapters. Returns `identifier` and `address`.
- `scan_for(timeout_ms)`: Scans for devices using the first available adapter. Returns `identifier`, `address`, `rssi`, `connectable`, and `manufacturer_data`.

### Connection Management

- `connect(address)`: Establishes a connection.
- `disconnect(address)`: Terminates the connection.

### GATT Operations

- `services(address)`: Discovers services and characteristics. Returns a list of services with their UUIDs and associated characteristic UUIDs.
- `read(address, service_uuid, char_uuid)`: Reads a single value. Returns `data_hex` and `data_utf8`. If the data is not valid UTF-8, invalid bytes are skipped in `data_utf8`.
- `write_request(address, service_uuid, char_uuid, data)`: Writes data (hex string) to a characteristic with response.
- `write_command(address, service_uuid, char_uuid, data)`: Writes data (hex string) to a characteristic without response.
- `notify(address, service_uuid, char_uuid)`: Subscribes to notifications. Data is buffered in the background.
- `indicate(address, service_uuid, char_uuid)`: Subscribes to indications. Data is buffered in the background.
- `get_notifications(address)`: Retrieves and clears all buffered notifications/indications.
- `unsubscribe(address, service_uuid, char_uuid)`: Unsubscribes from notifications or indications.

## Platform-Specific Behavior

### macOS / iOS

- **Addresses**: Uses randomized UUIDs (e.g., `5E2A...`) instead of hardware MAC addresses. These UUIDs are temporary and may change between sessions or device restarts.
- **Permissions**: Requires Bluetooth permissions. If the MCP server fails to start or scan, check System Settings.

### Linux (BlueZ)

- **Addresses**: Uses standard MAC addresses (e.g., `AA:BB:CC:DD:EE:FF`).
- **Dependencies**: Works on any Linux OS that uses BlueZ.

### Windows

- **Addresses**: Uses standard MAC addresses (e.g., `AA:BB:CC:DD:EE:FF`).

## Data Encoding

- **`data_hex`**: Always reliable. Lowercase hex string without `0x` prefix.
- **`data_utf8`**: Convenience field. If the data is not valid UTF-8, invalid bytes are silently skipped, so it may be incomplete or empty for binary data.

