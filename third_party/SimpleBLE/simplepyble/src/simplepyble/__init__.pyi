"""
Type stubs for SimplePyBLE - Python bindings for SimpleBLE

This module provides Bluetooth Low Energy (BLE) functionality for Python.
"""

from typing import Callable, Dict, List, Optional
from enum import Enum

__version__: str

class OperatingSystem(Enum):
    """Operating system enumeration."""
    WINDOWS: int
    MACOS: int
    LINUX: int

class BluetoothAddressType(Enum):
    """Bluetooth address type enumeration."""
    PUBLIC: int
    RANDOM: int
    UNSPECIFIED: int

class Descriptor:
    """Represents a GATT descriptor."""
    
    def uuid(self) -> str:
        """
        Get the UUID of the descriptor.
        
        Returns:
            str: The descriptor UUID
        """
        ...

class Characteristic:
    """Represents a GATT characteristic."""
    
    def uuid(self) -> str:
        """
        Get the UUID of the characteristic.
        
        Returns:
            str: The characteristic UUID
        """
        ...
    
    def descriptors(self) -> List[Descriptor]:
        """
        Get the descriptors of the characteristic.
        
        Returns:
            List[Descriptor]: List of descriptors
        """
        ...
    
    def capabilities(self) -> List[str]:
        """
        Get the capabilities of the characteristic.
        
        Returns:
            List[str]: List of capability strings
        """
        ...
    
    def can_read(self) -> bool:
        """
        Check if the characteristic can be read.
        
        Returns:
            bool: True if readable, False otherwise
        """
        ...
    
    def can_write_request(self) -> bool:
        """
        Check if the characteristic can be written with a request.
        
        Returns:
            bool: True if writable with request, False otherwise
        """
        ...
    
    def can_write_command(self) -> bool:
        """
        Check if the characteristic can be written with a command.
        
        Returns:
            bool: True if writable with command, False otherwise
        """
        ...
    
    def can_notify(self) -> bool:
        """
        Check if the characteristic can send notifications.
        
        Returns:
            bool: True if notifications supported, False otherwise
        """
        ...
    
    def can_indicate(self) -> bool:
        """
        Check if the characteristic can send indications.
        
        Returns:
            bool: True if indications supported, False otherwise
        """
        ...

class Service:
    """Represents a GATT service."""
    
    def uuid(self) -> str:
        """
        Get the UUID of the service.
        
        Returns:
            str: The service UUID
        """
        ...
    
    def data(self) -> bytes:
        """
        Get the advertised service data.
        
        Returns:
            bytes: The service data
        """
        ...
    
    def characteristics(self) -> List[Characteristic]:
        """
        Get the characteristics of the service.
        
        Returns:
            List[Characteristic]: List of characteristics
        """
        ...

class Peripheral:
    """Represents a BLE peripheral device."""
    
    def initialized(self) -> bool:
        """
        Check if the peripheral is initialized.
        
        Returns:
            bool: True if initialized, False otherwise
        """
        ...
    
    def identifier(self) -> str:
        """
        Get the identifier of the peripheral.
        
        Returns:
            str: The peripheral identifier
        """
        ...
    
    def address(self) -> str:
        """
        Get the address of the peripheral.
        
        Returns:
            str: The peripheral address
        """
        ...
    
    def address_type(self) -> BluetoothAddressType:
        """
        Get the address type of the peripheral.
        
        Returns:
            BluetoothAddressType: The address type
        """
        ...
    
    def rssi(self) -> int:
        """
        Get the RSSI (Received Signal Strength Indicator) of the peripheral.
        
        Returns:
            int: The RSSI value in dBm
        """
        ...
    
    def tx_power(self) -> int:
        """
        Get the transmit power of the peripheral.
        
        Returns:
            int: The transmit power in dBm
        """
        ...
    
    def mtu(self) -> int:
        """
        Get the negotiated MTU (Maximum Transmission Unit) value.
        
        Returns:
            int: The MTU value
        """
        ...
    
    def connect(self) -> None:
        """Connect to the peripheral."""
        ...
    
    def disconnect(self) -> None:
        """Disconnect from the peripheral."""
        ...
    
    def is_connected(self) -> bool:
        """
        Check if the peripheral is connected.
        
        Returns:
            bool: True if connected, False otherwise
        """
        ...
    
    def is_connectable(self) -> bool:
        """
        Check if the peripheral is connectable.
        
        Returns:
            bool: True if connectable, False otherwise
        """
        ...
    
    def is_paired(self) -> bool:
        """
        Check if the peripheral is paired.
        
        Returns:
            bool: True if paired, False otherwise
        """
        ...
    
    def unpair(self) -> None:
        """Unpair the peripheral."""
        ...
    
    def services(self) -> List[Service]:
        """
        Get the services of the peripheral.
        
        Returns:
            List[Service]: List of services
        """
        ...
    
    def manufacturer_data(self) -> Dict[int, bytes]:
        """
        Get the manufacturer data of the peripheral.
        
        Returns:
            Dict[int, bytes]: Dictionary mapping manufacturer ID to data
        """
        ...
    
    def read(self, service: str, characteristic: str) -> bytes:
        """
        Read a characteristic from the peripheral.
        
        Args:
            service: The service UUID
            characteristic: The characteristic UUID
            
        Returns:
            bytes: The characteristic value
        """
        ...
    
    def write_request(self, service: str, characteristic: str, payload: bytes) -> None:
        """
        Write a request to the peripheral.
        
        Args:
            service: The service UUID
            characteristic: The characteristic UUID
            payload: The data to write
        """
        ...
    
    def write_command(self, service: str, characteristic: str, payload: bytes) -> None:
        """
        Write a command to the peripheral.
        
        Args:
            service: The service UUID
            characteristic: The characteristic UUID
            payload: The data to write
        """
        ...
    
    def notify(self, service: str, characteristic: str, callback: Callable[[bytes], None]) -> None:
        """
        Subscribe to notifications from a characteristic.
        
        Args:
            service: The service UUID
            characteristic: The characteristic UUID
            callback: Callback function to receive notification data
        """
        ...
    
    def indicate(self, service: str, characteristic: str, callback: Callable[[bytes], None]) -> None:
        """
        Subscribe to indications from a characteristic.
        
        Args:
            service: The service UUID
            characteristic: The characteristic UUID
            callback: Callback function to receive indication data
        """
        ...
    
    def unsubscribe(self, service: str, characteristic: str) -> None:
        """
        Unsubscribe from a characteristic.
        
        Args:
            service: The service UUID
            characteristic: The characteristic UUID
        """
        ...
    
    def descriptor_read(self, service: str, characteristic: str, descriptor: str) -> bytes:
        """
        Read a descriptor from the peripheral.
        
        Args:
            service: The service UUID
            characteristic: The characteristic UUID
            descriptor: The descriptor UUID
            
        Returns:
            bytes: The descriptor value
        """
        ...
    
    def descriptor_write(self, service: str, characteristic: str, descriptor: str, payload: bytes) -> None:
        """
        Write a descriptor to the peripheral.
        
        Args:
            service: The service UUID
            characteristic: The characteristic UUID
            descriptor: The descriptor UUID
            payload: The data to write
        """
        ...
    
    def set_callback_on_connected(self, callback: Callable[[], None]) -> None:
        """
        Set the callback to be called when the peripheral connects.
        
        Args:
            callback: Callback function to call on connection
        """
        ...
    
    def set_callback_on_disconnected(self, callback: Callable[[], None]) -> None:
        """
        Set the callback to be called when the peripheral disconnects.
        
        Args:
            callback: Callback function to call on disconnection
        """
        ...

class Adapter:
    """Represents a BLE adapter."""
    
    @staticmethod
    def bluetooth_enabled() -> bool:
        """
        Check if Bluetooth is enabled.
        
        Returns:
            bool: True if Bluetooth is enabled, False otherwise
        """
        ...
    
    @staticmethod
    def get_adapters() -> List['Adapter']:
        """
        Get all available adapters.
        
        Returns:
            List[Adapter]: List of available adapters
        """
        ...
    
    def initialized(self) -> bool:
        """
        Check if the adapter is initialized.
        
        Returns:
            bool: True if initialized, False otherwise
        """
        ...
    
    def identifier(self) -> str:
        """
        Get the identifier of the adapter.
        
        Returns:
            str: The adapter identifier
        """
        ...
    
    def address(self) -> str:
        """
        Get the address of the adapter.
        
        Returns:
            str: The adapter address
        """
        ...
    
    def power_on(self) -> None:
        """Power on the adapter."""
        ...
    
    def power_off(self) -> None:
        """Power off the adapter."""
        ...
    
    def is_powered(self) -> bool:
        """
        Check if the adapter is powered.
        
        Returns:
            bool: True if powered, False otherwise
        """
        ...
    
    def scan_start(self) -> None:
        """Start scanning for peripherals."""
        ...
    
    def scan_stop(self) -> None:
        """Stop scanning for peripherals."""
        ...
    
    def scan_is_active(self) -> bool:
        """
        Check if the adapter is scanning.
        
        Returns:
            bool: True if scanning, False otherwise
        """
        ...
    
    def scan_for(self, timeout_ms: int) -> None:
        """
        Scan for peripherals for a given duration.
        
        Args:
            timeout_ms: Scan duration in milliseconds
        """
        ...
    
    def scan_get_results(self) -> List[Peripheral]:
        """
        Get the results of the last scan.
        
        Returns:
            List[Peripheral]: List of discovered peripherals
        """
        ...
    
    def set_callback_on_scan_start(self, callback: Callable[[], None]) -> None:
        """
        Set the callback to be called when scanning starts.
        
        Args:
            callback: Callback function to call when scan starts
        """
        ...
    
    def set_callback_on_scan_stop(self, callback: Callable[[], None]) -> None:
        """
        Set the callback to be called when scanning stops.
        
        Args:
            callback: Callback function to call when scan stops
        """
        ...
    
    def set_callback_on_scan_found(self, callback: Callable[[Peripheral], None]) -> None:
        """
        Set the callback to be called when a peripheral is found.
        
        Args:
            callback: Callback function to call when a peripheral is found
        """
        ...
    
    def set_callback_on_scan_updated(self, callback: Callable[[Peripheral], None]) -> None:
        """
        Set the callback to be called when a peripheral is updated.
        
        Args:
            callback: Callback function to call when a peripheral is updated
        """
        ...
    
    def get_paired_peripherals(self) -> List[Peripheral]:
        """
        Get all paired peripherals.
        
        Returns:
            List[Peripheral]: List of paired peripherals
        """
        ...

class _WinRTConfig:
    """WinRT-specific configuration options."""
    
    experimental_use_own_mta_apartment: bool
    """Use own MTA apartment (experimental)"""
    
    experimental_reinitialize_winrt_apartment_on_main_thread: bool
    """Reinitialize the WinRT apartment on the main thread (experimental)"""
    
    @staticmethod
    def reset() -> None:
        """Reset WinRT configuration options to their default values."""
        ...

class _SimpleBluezConfig:
    """SimpleBluez-specific configuration options."""
    
    @staticmethod
    def reset() -> None:
        """Reset SimpleBluez configuration options to their default values."""
        ...

class _CoreBluetoothConfig:
    """CoreBluetooth-specific configuration options."""
    
    @staticmethod
    def reset() -> None:
        """Reset CoreBluetooth configuration options to their default values."""
        ...

class _AndroidConfig:
    """Android-specific configuration options."""
    
    @staticmethod
    def reset() -> None:
        """Reset Android configuration options to their default values."""
        ...

class _BaseConfig:
    """Base configuration options."""
    
    @staticmethod
    def reset_all() -> None:
        """Reset all configuration options to their default values."""
        ...

class _Config:
    """Configuration options for SimpleBLE."""
    
    winrt: _WinRTConfig
    """WinRT-specific configuration"""
    
    simplebluez: _SimpleBluezConfig
    """SimpleBluez-specific configuration"""
    
    corebluetooth: _CoreBluetoothConfig
    """CoreBluetooth-specific configuration"""
    
    android: _AndroidConfig
    """Android-specific configuration"""
    
    base: _BaseConfig
    """Base configuration"""

config: _Config
"""Configuration module for SimpleBLE"""

def get_operating_system() -> OperatingSystem:
    """
    Get the currently-running operating system.
    
    Returns:
        OperatingSystem: The current operating system
    """
    ...

def get_simpleble_version() -> str:
    """
    Get the version of SimpleBLE.
    
    Returns:
        str: The SimpleBLE version string
    """
    ...
