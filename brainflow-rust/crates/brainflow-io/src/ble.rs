//! Bluetooth Low Energy (BLE) abstraction.
//!
//! Provides platform-specific BLE GATT client functionality for
//! biosensor devices like Muse and Ganglion.

use alloc::boxed::Box;
use alloc::string::String;
use alloc::vec::Vec;

use crate::IoError;

/// A 128-bit UUID for BLE services and characteristics.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct Uuid128([u8; 16]);

impl Uuid128 {
    /// Create a new UUID from bytes.
    pub const fn from_bytes(bytes: [u8; 16]) -> Self {
        Self(bytes)
    }

    /// Create from standard 16-bit BLE UUID.
    pub const fn from_u16(uuid: u16) -> Self {
        let mut bytes = [0u8; 16];
        // Standard Bluetooth Base UUID: 00000000-0000-1000-8000-00805F9B34FB
        bytes[0] = 0x00;
        bytes[1] = 0x00;
        bytes[2] = (uuid >> 8) as u8;
        bytes[3] = uuid as u8;
        bytes[4] = 0x00;
        bytes[5] = 0x00;
        bytes[6] = 0x10;
        bytes[7] = 0x00;
        bytes[8] = 0x80;
        bytes[9] = 0x00;
        bytes[10] = 0x00;
        bytes[11] = 0x80;
        bytes[12] = 0x5F;
        bytes[13] = 0x9B;
        bytes[14] = 0x34;
        bytes[15] = 0xFB;
        Self(bytes)
    }

    /// Create from string representation "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx".
    pub fn from_str(s: &str) -> Option<Self> {
        let s = s.replace('-', "");
        if s.len() != 32 {
            return None;
        }

        let mut bytes = [0u8; 16];
        for (i, chunk) in s.as_bytes().chunks(2).enumerate() {
            if i >= 16 {
                return None;
            }
            let high = hex_digit(chunk[0])?;
            let low = hex_digit(chunk[1])?;
            bytes[i] = (high << 4) | low;
        }
        Some(Self(bytes))
    }

    /// Get the raw bytes.
    pub fn as_bytes(&self) -> &[u8; 16] {
        &self.0
    }
}

/// Helper function to convert hex digit to value.
fn hex_digit(c: u8) -> Option<u8> {
    match c {
        b'0'..=b'9' => Some(c - b'0'),
        b'a'..=b'f' => Some(c - b'a' + 10),
        b'A'..=b'F' => Some(c - b'A' + 10),
        _ => None,
    }
}

/// BLE device information from scanning.
#[derive(Debug, Clone)]
pub struct BleDevice {
    /// Device address (MAC address or platform-specific identifier).
    pub address: String,
    /// Device name if available.
    pub name: Option<String>,
    /// RSSI signal strength.
    pub rssi: i8,
    /// Advertised service UUIDs.
    pub service_uuids: Vec<Uuid128>,
    /// Manufacturer-specific data.
    pub manufacturer_data: Vec<u8>,
}

/// BLE GATT service.
#[derive(Debug, Clone)]
pub struct GattService {
    /// Service UUID.
    pub uuid: Uuid128,
    /// Service handle (platform-specific).
    pub handle: u16,
    /// Whether this is a primary service.
    pub is_primary: bool,
}

/// BLE GATT characteristic.
#[derive(Debug, Clone)]
pub struct GattCharacteristic {
    /// Characteristic UUID.
    pub uuid: Uuid128,
    /// Characteristic handle (platform-specific).
    pub handle: u16,
    /// Value handle for read/write operations.
    pub value_handle: u16,
    /// Properties bitmask.
    pub properties: CharacteristicProperties,
}

/// Characteristic properties bitmask.
#[derive(Debug, Clone, Copy, Default)]
pub struct CharacteristicProperties(u8);

impl CharacteristicProperties {
    pub const BROADCAST: u8 = 0x01;
    pub const READ: u8 = 0x02;
    pub const WRITE_WITHOUT_RESPONSE: u8 = 0x04;
    pub const WRITE: u8 = 0x08;
    pub const NOTIFY: u8 = 0x10;
    pub const INDICATE: u8 = 0x20;
    pub const SIGNED_WRITE: u8 = 0x40;
    pub const EXTENDED: u8 = 0x80;

    /// Create from raw bitmask.
    pub const fn from_raw(value: u8) -> Self {
        Self(value)
    }

    /// Check if characteristic can be read.
    pub fn can_read(&self) -> bool {
        self.0 & Self::READ != 0
    }

    /// Check if characteristic can be written.
    pub fn can_write(&self) -> bool {
        self.0 & Self::WRITE != 0
    }

    /// Check if characteristic can be written without response.
    pub fn can_write_without_response(&self) -> bool {
        self.0 & Self::WRITE_WITHOUT_RESPONSE != 0
    }

    /// Check if characteristic supports notifications.
    pub fn can_notify(&self) -> bool {
        self.0 & Self::NOTIFY != 0
    }

    /// Check if characteristic supports indications.
    pub fn can_indicate(&self) -> bool {
        self.0 & Self::INDICATE != 0
    }
}

/// Callback for BLE scan results.
pub type ScanCallback = Box<dyn Fn(&BleDevice) + Send + Sync>;

/// Callback for characteristic notifications.
pub type NotificationCallback = Box<dyn Fn(&[u8]) + Send + Sync>;

/// BLE scanner trait for discovering devices.
pub trait BleScanner: Send + Sync {
    /// Start scanning for devices.
    ///
    /// # Arguments
    /// * `filter_services` - Only report devices advertising these service UUIDs.
    /// * `callback` - Called for each discovered device.
    fn start_scan(
        &mut self,
        filter_services: Option<&[Uuid128]>,
        callback: ScanCallback,
    ) -> Result<(), IoError>;

    /// Stop scanning.
    fn stop_scan(&mut self) -> Result<(), IoError>;

    /// Check if currently scanning.
    fn is_scanning(&self) -> bool;
}

/// BLE GATT client trait for connecting to devices.
pub trait BleClient: Send + Sync {
    /// Connect to a device by address.
    fn connect(&mut self, address: &str, timeout_ms: u32) -> Result<(), IoError>;

    /// Disconnect from the current device.
    fn disconnect(&mut self) -> Result<(), IoError>;

    /// Check if connected.
    fn is_connected(&self) -> bool;

    /// Get the connected device address.
    fn device_address(&self) -> Option<&str>;

    /// Discover all services.
    fn discover_services(&mut self) -> Result<Vec<GattService>, IoError>;

    /// Discover characteristics for a service.
    fn discover_characteristics(&mut self, service: &GattService) -> Result<Vec<GattCharacteristic>, IoError>;

    /// Read a characteristic value.
    fn read_characteristic(&mut self, characteristic: &GattCharacteristic) -> Result<Vec<u8>, IoError>;

    /// Write to a characteristic.
    fn write_characteristic(
        &mut self,
        characteristic: &GattCharacteristic,
        data: &[u8],
        with_response: bool,
    ) -> Result<(), IoError>;

    /// Subscribe to notifications on a characteristic.
    fn subscribe(
        &mut self,
        characteristic: &GattCharacteristic,
        callback: NotificationCallback,
    ) -> Result<(), IoError>;

    /// Unsubscribe from notifications.
    fn unsubscribe(&mut self, characteristic: &GattCharacteristic) -> Result<(), IoError>;

    /// Set connection timeout.
    fn set_timeout(&mut self, timeout_ms: u32);

    /// Get the current MTU size.
    fn mtu(&self) -> u16;

    /// Request a specific MTU size.
    fn request_mtu(&mut self, mtu: u16) -> Result<u16, IoError>;
}

/// Well-known BLE service UUIDs.
pub mod services {
    use super::Uuid128;

    /// Generic Access Profile.
    pub const GENERIC_ACCESS: Uuid128 = Uuid128::from_u16(0x1800);
    /// Generic Attribute Profile.
    pub const GENERIC_ATTRIBUTE: Uuid128 = Uuid128::from_u16(0x1801);
    /// Device Information.
    pub const DEVICE_INFORMATION: Uuid128 = Uuid128::from_u16(0x180A);
    /// Battery Service.
    pub const BATTERY: Uuid128 = Uuid128::from_u16(0x180F);

    // Muse headband service UUIDs
    /// Muse primary service UUID.
    pub const MUSE_SERVICE: Uuid128 = Uuid128::from_bytes([
        0x00, 0x00, 0xFE, 0x8D, 0x00, 0x00, 0x10, 0x00,
        0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB,
    ]);

    // OpenBCI Ganglion service UUID
    /// Ganglion primary service UUID.
    pub const GANGLION_SERVICE: Uuid128 = Uuid128::from_bytes([
        0xFE, 0x84, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
        0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB,
    ]);
}

/// Well-known BLE characteristic UUIDs.
pub mod characteristics {
    use super::Uuid128;

    /// Device Name characteristic.
    pub const DEVICE_NAME: Uuid128 = Uuid128::from_u16(0x2A00);
    /// Appearance characteristic.
    pub const APPEARANCE: Uuid128 = Uuid128::from_u16(0x2A01);
    /// Manufacturer Name characteristic.
    pub const MANUFACTURER_NAME: Uuid128 = Uuid128::from_u16(0x2A29);
    /// Model Number characteristic.
    pub const MODEL_NUMBER: Uuid128 = Uuid128::from_u16(0x2A24);
    /// Serial Number characteristic.
    pub const SERIAL_NUMBER: Uuid128 = Uuid128::from_u16(0x2A25);
    /// Firmware Revision characteristic.
    pub const FIRMWARE_REVISION: Uuid128 = Uuid128::from_u16(0x2A26);
    /// Battery Level characteristic.
    pub const BATTERY_LEVEL: Uuid128 = Uuid128::from_u16(0x2A19);

    // Muse-specific characteristics
    /// Muse control/command characteristic.
    pub const MUSE_CONTROL: Uuid128 = Uuid128::from_bytes([
        0x27, 0x3E, 0x00, 0x01, 0x40, 0x01, 0x4E, 0xE0,
        0xB7, 0xA0, 0x6A, 0xE8, 0xB1, 0x07, 0x5C, 0xE3,
    ]);
    /// Muse TP9 electrode data.
    pub const MUSE_TP9: Uuid128 = Uuid128::from_bytes([
        0x27, 0x3E, 0x00, 0x02, 0x40, 0x01, 0x4E, 0xE0,
        0xB7, 0xA0, 0x6A, 0xE8, 0xB1, 0x07, 0x5C, 0xE3,
    ]);
    /// Muse AF7 electrode data.
    pub const MUSE_AF7: Uuid128 = Uuid128::from_bytes([
        0x27, 0x3E, 0x00, 0x03, 0x40, 0x01, 0x4E, 0xE0,
        0xB7, 0xA0, 0x6A, 0xE8, 0xB1, 0x07, 0x5C, 0xE3,
    ]);
    /// Muse AF8 electrode data.
    pub const MUSE_AF8: Uuid128 = Uuid128::from_bytes([
        0x27, 0x3E, 0x00, 0x04, 0x40, 0x01, 0x4E, 0xE0,
        0xB7, 0xA0, 0x6A, 0xE8, 0xB1, 0x07, 0x5C, 0xE3,
    ]);
    /// Muse TP10 electrode data.
    pub const MUSE_TP10: Uuid128 = Uuid128::from_bytes([
        0x27, 0x3E, 0x00, 0x05, 0x40, 0x01, 0x4E, 0xE0,
        0xB7, 0xA0, 0x6A, 0xE8, 0xB1, 0x07, 0x5C, 0xE3,
    ]);
    /// Muse AUX/Right AUX electrode data.
    pub const MUSE_RIGHTAUX: Uuid128 = Uuid128::from_bytes([
        0x27, 0x3E, 0x00, 0x06, 0x40, 0x01, 0x4E, 0xE0,
        0xB7, 0xA0, 0x6A, 0xE8, 0xB1, 0x07, 0x5C, 0xE3,
    ]);
    /// Muse accelerometer data.
    pub const MUSE_ACCEL: Uuid128 = Uuid128::from_bytes([
        0x27, 0x3E, 0x00, 0x07, 0x40, 0x01, 0x4E, 0xE0,
        0xB7, 0xA0, 0x6A, 0xE8, 0xB1, 0x07, 0x5C, 0xE3,
    ]);
    /// Muse gyroscope data.
    pub const MUSE_GYRO: Uuid128 = Uuid128::from_bytes([
        0x27, 0x3E, 0x00, 0x08, 0x40, 0x01, 0x4E, 0xE0,
        0xB7, 0xA0, 0x6A, 0xE8, 0xB1, 0x07, 0x5C, 0xE3,
    ]);
    /// Muse PPG data.
    pub const MUSE_PPG: Uuid128 = Uuid128::from_bytes([
        0x27, 0x3E, 0x00, 0x09, 0x40, 0x01, 0x4E, 0xE0,
        0xB7, 0xA0, 0x6A, 0xE8, 0xB1, 0x07, 0x5C, 0xE3,
    ]);

    // Ganglion-specific characteristics
    /// Ganglion data receive characteristic.
    pub const GANGLION_RECEIVE: Uuid128 = Uuid128::from_bytes([
        0x2D, 0x30, 0xC0, 0x82, 0xF3, 0x9F, 0x11, 0x55,
        0xAF, 0xA6, 0x00, 0x02, 0xA5, 0xD5, 0xC5, 0x1B,
    ]);
    /// Ganglion data send characteristic.
    pub const GANGLION_SEND: Uuid128 = Uuid128::from_bytes([
        0x2D, 0x30, 0xC0, 0x83, 0xF3, 0x9F, 0x11, 0x55,
        0xAF, 0xA6, 0x00, 0x02, 0xA5, 0xD5, 0xC5, 0x1B,
    ]);
}

/// Standard library BLE implementation using platform APIs.
#[cfg(all(feature = "std", target_os = "linux"))]
pub mod linux_ble {
    use super::*;
    use alloc::string::ToString;
    use alloc::vec;
    use core::sync::atomic::{AtomicBool, Ordering};

    /// Linux BLE client using BlueZ DBus API.
    ///
    /// Note: This is a stub implementation. A full implementation would
    /// require interfacing with the BlueZ DBus API using zbus or similar.
    pub struct LinuxBleClient {
        connected: AtomicBool,
        address: Option<String>,
        timeout_ms: u32,
        mtu: u16,
    }

    impl LinuxBleClient {
        /// Create a new Linux BLE client.
        pub fn new() -> Self {
            Self {
                connected: AtomicBool::new(false),
                address: None,
                timeout_ms: 10000,
                mtu: 23, // Default BLE MTU
            }
        }
    }

    impl Default for LinuxBleClient {
        fn default() -> Self {
            Self::new()
        }
    }

    impl BleClient for LinuxBleClient {
        fn connect(&mut self, address: &str, _timeout_ms: u32) -> Result<(), IoError> {
            // Stub: would use BlueZ DBus API
            self.address = Some(address.to_string());
            self.connected.store(true, Ordering::SeqCst);
            Err(IoError::NotSupported)
        }

        fn disconnect(&mut self) -> Result<(), IoError> {
            self.connected.store(false, Ordering::SeqCst);
            self.address = None;
            Ok(())
        }

        fn is_connected(&self) -> bool {
            self.connected.load(Ordering::SeqCst)
        }

        fn device_address(&self) -> Option<&str> {
            self.address.as_deref()
        }

        fn discover_services(&mut self) -> Result<Vec<GattService>, IoError> {
            if !self.is_connected() {
                return Err(IoError::NotConnected);
            }
            Err(IoError::NotSupported)
        }

        fn discover_characteristics(&mut self, _service: &GattService) -> Result<Vec<GattCharacteristic>, IoError> {
            if !self.is_connected() {
                return Err(IoError::NotConnected);
            }
            Err(IoError::NotSupported)
        }

        fn read_characteristic(&mut self, _characteristic: &GattCharacteristic) -> Result<Vec<u8>, IoError> {
            if !self.is_connected() {
                return Err(IoError::NotConnected);
            }
            Err(IoError::NotSupported)
        }

        fn write_characteristic(
            &mut self,
            _characteristic: &GattCharacteristic,
            _data: &[u8],
            _with_response: bool,
        ) -> Result<(), IoError> {
            if !self.is_connected() {
                return Err(IoError::NotConnected);
            }
            Err(IoError::NotSupported)
        }

        fn subscribe(
            &mut self,
            _characteristic: &GattCharacteristic,
            _callback: NotificationCallback,
        ) -> Result<(), IoError> {
            if !self.is_connected() {
                return Err(IoError::NotConnected);
            }
            Err(IoError::NotSupported)
        }

        fn unsubscribe(&mut self, _characteristic: &GattCharacteristic) -> Result<(), IoError> {
            if !self.is_connected() {
                return Err(IoError::NotConnected);
            }
            Err(IoError::NotSupported)
        }

        fn set_timeout(&mut self, timeout_ms: u32) {
            self.timeout_ms = timeout_ms;
        }

        fn mtu(&self) -> u16 {
            self.mtu
        }

        fn request_mtu(&mut self, mtu: u16) -> Result<u16, IoError> {
            if !self.is_connected() {
                return Err(IoError::NotConnected);
            }
            // Can only negotiate up to 512 bytes
            self.mtu = mtu.min(512);
            Ok(self.mtu)
        }
    }

    /// Linux BLE scanner using BlueZ.
    pub struct LinuxBleScanner {
        scanning: AtomicBool,
    }

    impl LinuxBleScanner {
        pub fn new() -> Self {
            Self {
                scanning: AtomicBool::new(false),
            }
        }
    }

    impl Default for LinuxBleScanner {
        fn default() -> Self {
            Self::new()
        }
    }

    impl BleScanner for LinuxBleScanner {
        fn start_scan(
            &mut self,
            _filter_services: Option<&[Uuid128]>,
            _callback: ScanCallback,
        ) -> Result<(), IoError> {
            self.scanning.store(true, Ordering::SeqCst);
            Err(IoError::NotSupported)
        }

        fn stop_scan(&mut self) -> Result<(), IoError> {
            self.scanning.store(false, Ordering::SeqCst);
            Ok(())
        }

        fn is_scanning(&self) -> bool {
            self.scanning.load(Ordering::SeqCst)
        }
    }
}

/// No-std stub BLE implementation.
#[cfg(not(feature = "std"))]
pub mod nostd_ble {
    use super::*;

    /// Stub BLE client for no_std environments.
    pub struct StubBleClient;

    impl StubBleClient {
        pub fn new() -> Self {
            Self
        }
    }

    impl Default for StubBleClient {
        fn default() -> Self {
            Self::new()
        }
    }

    impl BleClient for StubBleClient {
        fn connect(&mut self, _address: &str, _timeout_ms: u32) -> Result<(), IoError> {
            Err(IoError::NotSupported)
        }

        fn disconnect(&mut self) -> Result<(), IoError> {
            Ok(())
        }

        fn is_connected(&self) -> bool {
            false
        }

        fn device_address(&self) -> Option<&str> {
            None
        }

        fn discover_services(&mut self) -> Result<Vec<GattService>, IoError> {
            Err(IoError::NotSupported)
        }

        fn discover_characteristics(&mut self, _service: &GattService) -> Result<Vec<GattCharacteristic>, IoError> {
            Err(IoError::NotSupported)
        }

        fn read_characteristic(&mut self, _characteristic: &GattCharacteristic) -> Result<Vec<u8>, IoError> {
            Err(IoError::NotSupported)
        }

        fn write_characteristic(
            &mut self,
            _characteristic: &GattCharacteristic,
            _data: &[u8],
            _with_response: bool,
        ) -> Result<(), IoError> {
            Err(IoError::NotSupported)
        }

        fn subscribe(
            &mut self,
            _characteristic: &GattCharacteristic,
            _callback: NotificationCallback,
        ) -> Result<(), IoError> {
            Err(IoError::NotSupported)
        }

        fn unsubscribe(&mut self, _characteristic: &GattCharacteristic) -> Result<(), IoError> {
            Err(IoError::NotSupported)
        }

        fn set_timeout(&mut self, _timeout_ms: u32) {}

        fn mtu(&self) -> u16 {
            23
        }

        fn request_mtu(&mut self, _mtu: u16) -> Result<u16, IoError> {
            Err(IoError::NotSupported)
        }
    }

    /// Stub BLE scanner for no_std environments.
    pub struct StubBleScanner;

    impl StubBleScanner {
        pub fn new() -> Self {
            Self
        }
    }

    impl Default for StubBleScanner {
        fn default() -> Self {
            Self::new()
        }
    }

    impl BleScanner for StubBleScanner {
        fn start_scan(
            &mut self,
            _filter_services: Option<&[Uuid128]>,
            _callback: ScanCallback,
        ) -> Result<(), IoError> {
            Err(IoError::NotSupported)
        }

        fn stop_scan(&mut self) -> Result<(), IoError> {
            Ok(())
        }

        fn is_scanning(&self) -> bool {
            false
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_uuid_from_u16() {
        let uuid = Uuid128::from_u16(0x180F);
        let bytes = uuid.as_bytes();
        assert_eq!(bytes[2], 0x18);
        assert_eq!(bytes[3], 0x0F);
    }

    #[test]
    fn test_uuid_from_str() {
        let uuid = Uuid128::from_str("0000180F-0000-1000-8000-00805F9B34FB");
        assert!(uuid.is_some());
        let uuid = uuid.unwrap();
        let bytes = uuid.as_bytes();
        assert_eq!(bytes[2], 0x18);
        assert_eq!(bytes[3], 0x0F);
    }

    #[test]
    fn test_characteristic_properties() {
        let props = CharacteristicProperties::from_raw(
            CharacteristicProperties::READ | CharacteristicProperties::NOTIFY,
        );
        assert!(props.can_read());
        assert!(props.can_notify());
        assert!(!props.can_write());
        assert!(!props.can_indicate());
    }

    #[test]
    fn test_hex_digit() {
        assert_eq!(hex_digit(b'0'), Some(0));
        assert_eq!(hex_digit(b'9'), Some(9));
        assert_eq!(hex_digit(b'A'), Some(10));
        assert_eq!(hex_digit(b'F'), Some(15));
        assert_eq!(hex_digit(b'a'), Some(10));
        assert_eq!(hex_digit(b'f'), Some(15));
        assert_eq!(hex_digit(b'G'), None);
    }
}
