# Troubleshooting SimpleAIBLE MCP Operations

## Bluetooth Not Enabled

If any MCP operation fails (scanning, connecting, reading, etc.), the first diagnostic step is to check if Bluetooth is enabled on the host system.

### Diagnostic Steps

1. **When an operation fails, call `bluetooth_enabled` tool** to check the Bluetooth status:
   ```python
   result = bluetooth_enabled()
   # Returns: {"enabled": true} or {"enabled": false}
   ```

2. **If Bluetooth is disabled (`enabled: false`)**:
   - Prompt the user: "Bluetooth appears to be disabled on your system. Please enable Bluetooth in your system settings and try again."
   - Wait for the user to confirm they've enabled Bluetooth
   - Call `bluetooth_enabled` again to verify it's now enabled
   - Retry the original operation

3. **If Bluetooth is enabled (`enabled: true`) but operations still fail**:
   - The issue is likely something else (permissions, device not in range, hardware issue, etc.)
   - Proceed with other troubleshooting steps:
     - Check if adapters are available with `get_adapters`
     - Verify device proximity and advertising status
     - Check system permissions (especially on macOS/iOS)

### Important Notes

- **Assume Bluetooth is enabled by default** - do not check `bluetooth_enabled` proactively
- **Only call `bluetooth_enabled` when an operation fails** to diagnose the issue
- Bluetooth must be enabled for **all** BLE operations to work
- The `bluetooth_enabled` check is fast and idempotent - safe to call multiple times
