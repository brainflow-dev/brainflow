pub mod types;
pub mod adapter;
pub mod peripheral;
pub mod service;
pub mod characteristic;
pub mod descriptor;

use adapter::InnerAdapter;
use peripheral::InnerPeripheral;

pub use types::{Error, BluetoothAddressType, CharacteristicCapability};
pub use adapter::Adapter;
pub use adapter::ScanEvent;
pub use adapter::PowerEvent;
pub use peripheral::Peripheral;
pub use peripheral::ConnectionEvent;
pub use peripheral::ValueChangedEvent;
pub use service::Service;
pub use characteristic::Characteristic;
pub use descriptor::Descriptor;

#[cxx::bridge]
mod ffi {

    // ----------------------------------------------------------------------------------

    // The following structs are used to wrap the underlying classes into the objects
    // that are used in the Rust bindings.

    #[namespace = "Bindings"]
    struct RustyAdapterWrapper {
        internal: UniquePtr<RustyAdapter>,
    }

    #[namespace = "Bindings"]
    struct RustyPeripheralWrapper {
        internal: UniquePtr<RustyPeripheral>,
    }

    #[namespace = "Bindings"]
    struct RustyServiceWrapper {
        internal: UniquePtr<RustyService>,
    }

    #[namespace = "Bindings"]
    struct RustyCharacteristicWrapper {
        internal: UniquePtr<RustyCharacteristic>,
    }

    #[namespace = "Bindings"]
    struct RustyDescriptorWrapper {
        internal: UniquePtr<RustyDescriptor>,
    }

    // ----------------------------------------------------------------------------------

    #[namespace = "Bindings"]
    struct RustyManufacturerDataWrapper {
        company_id: u16,
        data: Vec<u8>,
    }

    #[namespace = "SimpleBLE"]
    #[repr(i32)]
    enum BluetoothAddressType {
        PUBLIC,
        RANDOM,
        UNSPECIFIED,
    }

    // ----------------------------------------------------------------------------------

    #[namespace = "SimpleRsBLE"]
    extern "Rust" {
        type InnerAdapter;

        pub fn on_callback_scan_start(self: &mut InnerAdapter);
        pub fn on_callback_scan_stop(self: &mut InnerAdapter);
        pub fn on_callback_scan_updated(self: &mut InnerAdapter, peripheral: &mut RustyPeripheralWrapper);
        pub fn on_callback_scan_found(self: &mut InnerAdapter, peripheral: &mut RustyPeripheralWrapper);
        pub fn on_callback_power_on(self: &mut InnerAdapter);
        pub fn on_callback_power_off(self: &mut InnerAdapter);

        type InnerPeripheral;

        pub fn on_callback_connected(self: &mut InnerPeripheral);
        pub fn on_callback_disconnected(self: &mut InnerPeripheral);
        pub fn on_callback_characteristic_updated(
            self: &mut InnerPeripheral,
            service: &String,
            Characteristic: &String,
            data: &Vec<u8>,
        );
    }

    unsafe extern "C++" {
        include!("src/bindings/Bindings.hpp");

        #[namespace = "SimpleBLE"]
        type BluetoothAddressType;

        #[namespace = "Bindings"]
        type RustyAdapter;

        #[namespace = "Bindings"]
        type RustyPeripheral;

        #[namespace = "Bindings"]
        type RustyService;

        #[namespace = "Bindings"]
        type RustyCharacteristic;

        #[namespace = "Bindings"]
        type RustyDescriptor;

        // Common functions

        #[namespace = "Bindings"]
        fn RustyAdapter_bluetooth_enabled() -> Result<bool>;

        #[namespace = "Bindings"]
        fn RustyAdapter_get_adapters() -> Result<Vec<RustyAdapterWrapper>>;

        // RustyAdapter functions

        fn link(self: &RustyAdapter, target: Pin<&mut InnerAdapter>) -> Result<()>;
        fn unlink(self: &RustyAdapter) -> Result<()>;

        fn initialized(self: &RustyAdapter) -> Result<bool>;
        fn identifier(self: &RustyAdapter) -> Result<String>;
        fn address(self: &RustyAdapter) -> Result<String>;

        fn power_on(self: &RustyAdapter) -> Result<()>;
        fn power_off(self: &RustyAdapter) -> Result<()>;
        fn is_powered(self: &RustyAdapter) -> Result<bool>;

        fn scan_start(self: &RustyAdapter) -> Result<()>;
        fn scan_stop(self: &RustyAdapter) -> Result<()>;
        fn scan_for(self: &RustyAdapter, timeout_ms: i32) -> Result<()>;
        fn scan_is_active(self: &RustyAdapter) -> Result<bool>;
        fn scan_get_results(self: &RustyAdapter) -> Result<Vec<RustyPeripheralWrapper>>;

        fn get_paired_peripherals(self: &RustyAdapter) -> Result<Vec<RustyPeripheralWrapper>>;
        fn get_connected_peripherals(self: &RustyAdapter) -> Result<Vec<RustyPeripheralWrapper>>;

        // RustyPeripheral functions

        fn link(self: &RustyPeripheral, target: Pin<&mut InnerPeripheral>) -> Result<()>;
        fn unlink(self: &RustyPeripheral) -> Result<()>;

        fn initialized(self: &RustyPeripheral) -> Result<bool>;
        fn identifier(self: &RustyPeripheral) -> Result<String>;
        fn address(self: &RustyPeripheral) -> Result<String>;
        fn address_type(self: &RustyPeripheral) -> Result<BluetoothAddressType>;
        fn rssi(self: &RustyPeripheral) -> Result<i16>;

        fn tx_power(self: &RustyPeripheral) -> Result<i16>;
        fn mtu(self: &RustyPeripheral) -> Result<u16>;

        fn connect(self: &RustyPeripheral) -> Result<()>;
        fn disconnect(self: &RustyPeripheral) -> Result<()>;
        fn is_connected(self: &RustyPeripheral) -> Result<bool>;
        fn is_connectable(self: &RustyPeripheral) -> Result<bool>;
        fn is_paired(self: &RustyPeripheral) -> Result<bool>;
        fn unpair(self: &RustyPeripheral) -> Result<()>;

        fn services(self: &RustyPeripheral) -> Result<Vec<RustyServiceWrapper>>;
        fn manufacturer_data(self: &RustyPeripheral) -> Result<Vec<RustyManufacturerDataWrapper>>;

        fn read(
            self: &RustyPeripheral,
            service: &String,
            characteristic: &String,
        ) -> Result<Vec<u8>>;
        fn write_request(
            self: &RustyPeripheral,
            service: &String,
            characteristic: &String,
            data: &Vec<u8>,
        ) -> Result<()>;
        fn write_command(
            self: &RustyPeripheral,
            service: &String,
            characteristic: &String,
            data: &Vec<u8>,
        ) -> Result<()>;
        fn notify(self: &RustyPeripheral, service: &String, characteristic: &String) -> Result<()>;
        fn indicate(
            self: &RustyPeripheral,
            service: &String,
            characteristic: &String,
        ) -> Result<()>;
        fn unsubscribe(
            self: &RustyPeripheral,
            service: &String,
            characteristic: &String,
        ) -> Result<()>;

        fn read_descriptor(
            self: &RustyPeripheral,
            service: &String,
            characteristic: &String,
            descriptor: &String,
        ) -> Result<Vec<u8>>;
        fn write_descriptor(
            self: &RustyPeripheral,
            service: &String,
            characteristic: &String,
            descriptor: &String,
            data: &Vec<u8>,
        ) -> Result<()>;

        // RustyService functions

        fn initialized(self: &RustyService) -> bool;
        fn uuid(self: &RustyService) -> String;
        fn data(self: &RustyService) -> Vec<u8>;
        fn characteristics(self: &RustyService) -> Vec<RustyCharacteristicWrapper>;

        // RustyCharacteristic functions

        fn initialized(self: &RustyCharacteristic) -> bool;
        fn uuid(self: &RustyCharacteristic) -> String;
        fn descriptors(self: &RustyCharacteristic) -> Vec<RustyDescriptorWrapper>;
        fn capabilities(self: &RustyCharacteristic) -> Vec<String>;

        fn can_read(self: &RustyCharacteristic) -> bool;
        fn can_write_request(self: &RustyCharacteristic) -> bool;
        fn can_write_command(self: &RustyCharacteristic) -> bool;
        fn can_notify(self: &RustyCharacteristic) -> bool;
        fn can_indicate(self: &RustyCharacteristic) -> bool;

        // RustyDescriptor functions

        fn initialized(self: &RustyDescriptor) -> bool;
        fn uuid(self: &RustyDescriptor) -> String;
    }
}


