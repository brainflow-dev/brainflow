import asyncio
import atexit
import weakref
from typing import List, Callable, Optional, Union, Any, Dict, Set
import simplepyble

_active_peripherals = weakref.WeakSet()

def _global_cleanup():
    # Clear all adapters known to SimpleBLE
    try:
        for a in simplepyble.Adapter.get_adapters():
            a.set_callback_on_scan_start(None)
            a.set_callback_on_scan_stop(None)
            a.set_callback_on_scan_found(None)
            a.set_callback_on_scan_updated(None)
            try:
                if a.scan_is_active():
                    a.scan_stop()
            except:
                pass
    except:
        pass
        
    # Clear peripherals that we tracked via aio.Peripheral
    for p in list(_active_peripherals):
        try:
            p._clear_all_callbacks_sync()
        except:
            pass

atexit.register(_global_cleanup)

class Descriptor:
    def __init__(self, internal_descriptor: simplepyble.Descriptor):
        self._internal = internal_descriptor

    def initialized(self) -> bool:
        return self._internal.initialized()

    def uuid(self) -> str:
        return self._internal.uuid()

class Characteristic:
    def __init__(self, internal_characteristic: simplepyble.Characteristic):
        self._internal = internal_characteristic

    def initialized(self) -> bool:
        return self._internal.initialized()

    def uuid(self) -> str:
        return self._internal.uuid()

    def descriptors(self) -> List[Descriptor]:
        return [Descriptor(d) for d in self._internal.descriptors()]

    def capabilities(self) -> List[str]:
        return self._internal.capabilities()

    def can_read(self) -> bool:
        return self._internal.can_read()

    def can_write_request(self) -> bool:
        return self._internal.can_write_request()

    def can_write_command(self) -> bool:
        return self._internal.can_write_command()

    def can_notify(self) -> bool:
        return self._internal.can_notify()

    def can_indicate(self) -> bool:
        return self._internal.can_indicate()

class Service:
    def __init__(self, internal_service: simplepyble.Service):
        self._internal = internal_service

    def initialized(self) -> bool:
        return self._internal.initialized()

    def uuid(self) -> str:
        return self._internal.uuid()

    def data(self) -> bytes:
        return self._internal.data()

    def characteristics(self) -> List[Characteristic]:
        return [Characteristic(c) for c in self._internal.characteristics()]

class Peripheral:
    def __init__(self, internal_peripheral: simplepyble.Peripheral):
        self._internal = internal_peripheral
        self._subscriptions: Set[tuple] = set()
        _active_peripherals.add(self)

    def initialized(self) -> bool:
        return self._internal.initialized()

    def identifier(self) -> str:
        return self._internal.identifier()

    def address(self) -> str:
        return self._internal.address()

    def address_type(self) -> str:
        return self._internal.address_type()

    def rssi(self) -> int:
        return self._internal.rssi()

    def tx_power(self) -> int:
        return self._internal.tx_power()

    def mtu(self) -> int:
        return self._internal.mtu()

    async def connect(self):
        loop = asyncio.get_running_loop()
        return await loop.run_in_executor(None, self._internal.connect)

    async def disconnect(self):
        loop = asyncio.get_running_loop()
        return await loop.run_in_executor(None, self._internal.disconnect)

    def is_connected(self) -> bool:
        return self._internal.is_connected()

    def is_connectable(self) -> bool:
        return self._internal.is_connectable()

    def is_paired(self) -> bool:
        return self._internal.is_paired()

    async def unpair(self):
        loop = asyncio.get_running_loop()
        return await loop.run_in_executor(None, self._internal.unpair)

    def services(self) -> List[Service]:
        return [Service(s) for s in self._internal.services()]

    def manufacturer_data(self) -> Dict[int, bytes]:
        return self._internal.manufacturer_data()

    async def read(self, service_uuid: str, characteristic_uuid: str) -> bytes:
        loop = asyncio.get_running_loop()
        return await loop.run_in_executor(None, self._internal.read, service_uuid, characteristic_uuid)

    async def write_request(self, service_uuid: str, characteristic_uuid: str, payload: bytes):
        loop = asyncio.get_running_loop()
        return await loop.run_in_executor(None, self._internal.write_request, service_uuid, characteristic_uuid, payload)

    async def write_command(self, service_uuid: str, characteristic_uuid: str, payload: bytes):
        loop = asyncio.get_running_loop()
        return await loop.run_in_executor(None, self._internal.write_command, service_uuid, characteristic_uuid, payload)

    async def notify(self, service_uuid: str, characteristic_uuid: str, callback: Callable[[bytes], None]):
        loop = asyncio.get_running_loop()
        self._subscriptions.add((service_uuid, characteristic_uuid))
        def wrapper(payload):
            if asyncio.iscoroutinefunction(callback):
                asyncio.run_coroutine_threadsafe(callback(payload), loop)
            else:
                loop.call_soon_threadsafe(callback, payload)
        return await loop.run_in_executor(None, self._internal.notify, service_uuid, characteristic_uuid, wrapper)

    async def indicate(self, service_uuid: str, characteristic_uuid: str, callback: Callable[[bytes], None]):
        loop = asyncio.get_running_loop()
        self._subscriptions.add((service_uuid, characteristic_uuid))
        def wrapper(payload):
            if asyncio.iscoroutinefunction(callback):
                asyncio.run_coroutine_threadsafe(callback(payload), loop)
            else:
                loop.call_soon_threadsafe(callback, payload)
        return await loop.run_in_executor(None, self._internal.indicate, service_uuid, characteristic_uuid, wrapper)

    async def unsubscribe(self, service_uuid: str, characteristic_uuid: str):
        loop = asyncio.get_running_loop()
        self._subscriptions.discard((service_uuid, characteristic_uuid))
        return await loop.run_in_executor(None, self._internal.unsubscribe, service_uuid, characteristic_uuid)

    async def descriptor_read(self, service_uuid: str, characteristic_uuid: str, descriptor_uuid: str) -> bytes:
        loop = asyncio.get_running_loop()
        return await loop.run_in_executor(None, self._internal.descriptor_read, service_uuid, characteristic_uuid, descriptor_uuid)

    async def descriptor_write(self, service_uuid: str, characteristic_uuid: str, descriptor_uuid: str, payload: bytes):
        loop = asyncio.get_running_loop()
        return await loop.run_in_executor(None, self._internal.descriptor_write, service_uuid, characteristic_uuid, descriptor_uuid, payload)

    def set_callback_on_connected(self, callback: Optional[Callable[[], None]]):
        if callback is None:
             self._internal.set_callback_on_connected(None)
             return
        loop = asyncio.get_running_loop()
        def wrapper():
            if asyncio.iscoroutinefunction(callback):
                asyncio.run_coroutine_threadsafe(callback(), loop)
            else:
                loop.call_soon_threadsafe(callback)
        self._internal.set_callback_on_connected(wrapper)

    def set_callback_on_disconnected(self, callback: Optional[Callable[[], None]]):
        if callback is None:
             self._internal.set_callback_on_disconnected(None)
             return
        loop = asyncio.get_running_loop()
        def wrapper():
            if asyncio.iscoroutinefunction(callback):
                asyncio.run_coroutine_threadsafe(callback(), loop)
            else:
                loop.call_soon_threadsafe(callback)
        self._internal.set_callback_on_disconnected(wrapper)

    def _clear_all_callbacks_sync(self):
        """Internal sync cleanup for atexit or __aexit__"""
        try:
            self._internal.set_callback_on_connected(None)
            self._internal.set_callback_on_disconnected(None)
            for s, c in list(self._subscriptions):
                try:
                    self._internal.unsubscribe(s, c)
                except:
                    pass
            self._subscriptions.clear()
        except:
            pass

    async def __aenter__(self):
        return self

    async def __aexit__(self, exc_type, exc_val, exc_tb):
        self._clear_all_callbacks_sync()

class Adapter:
    def __init__(self, internal_adapter: simplepyble.Adapter):
        self._internal = internal_adapter

    @classmethod
    def get_adapters(cls) -> List['Adapter']:
        return [cls(adapter) for adapter in simplepyble.Adapter.get_adapters()]

    def identifier(self) -> str:
        return self._internal.identifier()

    def address(self) -> str:
        return self._internal.address()

    def is_powered(self) -> bool:
        return self._internal.is_powered()
        
    def initialized(self) -> bool:
        return self._internal.initialized()
        
    def bluetooth_enabled(self) -> bool:
        return self._internal.bluetooth_enabled()

    async def power_on(self):
        loop = asyncio.get_running_loop()
        return await loop.run_in_executor(None, self._internal.power_on)

    async def power_off(self):
        loop = asyncio.get_running_loop()
        return await loop.run_in_executor(None, self._internal.power_off)

    async def scan_start(self):
        loop = asyncio.get_running_loop()
        return await loop.run_in_executor(None, self._internal.scan_start)

    async def scan_stop(self):
        loop = asyncio.get_running_loop()
        return await loop.run_in_executor(None, self._internal.scan_stop)
        
    async def scan_for(self, duration_ms: int):
        loop = asyncio.get_running_loop()
        return await loop.run_in_executor(None, self._internal.scan_for, duration_ms)
        
    def scan_is_active(self) -> bool:
        return self._internal.scan_is_active()

    def scan_get_results(self) -> List[Peripheral]:
        return [Peripheral(p) for p in self._internal.scan_get_results()]
        
    def get_paired_peripherals(self) -> List[Peripheral]:
        return [Peripheral(p) for p in self._internal.get_paired_peripherals()]

    def get_connected_peripherals(self) -> List[Peripheral]:
        return [Peripheral(p) for p in self._internal.get_connected_peripherals()]

    def set_callback_on_scan_start(self, callback: Optional[Callable[[], None]]):
        if callback is None:
             self._internal.set_callback_on_scan_start(None)
             return
        loop = asyncio.get_running_loop()
        def wrapper():
            if asyncio.iscoroutinefunction(callback):
                asyncio.run_coroutine_threadsafe(callback(), loop)
            else:
                loop.call_soon_threadsafe(callback)
        self._internal.set_callback_on_scan_start(wrapper)

    def set_callback_on_scan_stop(self, callback: Optional[Callable[[], None]]):
        if callback is None:
             self._internal.set_callback_on_scan_stop(None)
             return
        loop = asyncio.get_running_loop()
        def wrapper():
            if asyncio.iscoroutinefunction(callback):
                asyncio.run_coroutine_threadsafe(callback(), loop)
            else:
                loop.call_soon_threadsafe(callback)
        self._internal.set_callback_on_scan_stop(wrapper)

    def set_callback_on_scan_found(self, callback: Optional[Callable[[Peripheral], None]]):
        if callback is None:
             self._internal.set_callback_on_scan_found(None)
             return
        loop = asyncio.get_running_loop()
        def wrapper(peripheral):
            # Wrap the peripheral before passing it to the callback
            wrapped_peripheral = Peripheral(peripheral)
            if asyncio.iscoroutinefunction(callback):
                asyncio.run_coroutine_threadsafe(callback(wrapped_peripheral), loop)
            else:
                loop.call_soon_threadsafe(callback, wrapped_peripheral)
        self._internal.set_callback_on_scan_found(wrapper)

    def set_callback_on_scan_updated(self, callback: Optional[Callable[[Peripheral], None]]):
        if callback is None:
             self._internal.set_callback_on_scan_updated(None)
             return
        loop = asyncio.get_running_loop()
        def wrapper(peripheral):
            # Wrap the peripheral before passing it to the callback
            wrapped_peripheral = Peripheral(peripheral)
            if asyncio.iscoroutinefunction(callback):
                asyncio.run_coroutine_threadsafe(callback(wrapped_peripheral), loop)
            else:
                loop.call_soon_threadsafe(callback, wrapped_peripheral)
        self._internal.set_callback_on_scan_updated(wrapper)

    def set_callback_on_power_on(self, callback: Optional[Callable[[], None]]):
        if callback is None:
             self._internal.set_callback_on_power_on(None)
             return
        loop = asyncio.get_running_loop()
        def wrapper():
            if asyncio.iscoroutinefunction(callback):
                asyncio.run_coroutine_threadsafe(callback(), loop)
            else:
                loop.call_soon_threadsafe(callback)
        self._internal.set_callback_on_power_on(wrapper)

    def set_callback_on_power_off(self, callback: Optional[Callable[[], None]]):
        if callback is None:
             self._internal.set_callback_on_power_off(None)
             return
        loop = asyncio.get_running_loop()
        def wrapper():
            if asyncio.iscoroutinefunction(callback):
                asyncio.run_coroutine_threadsafe(callback(), loop)
            else:
                loop.call_soon_threadsafe(callback)
        self._internal.set_callback_on_power_off(wrapper)

    def _clear_all_callbacks_sync(self):
        """Internal sync cleanup for atexit or __aexit__"""
        try:
            self._internal.set_callback_on_scan_start(None)
            self._internal.set_callback_on_scan_stop(None)
            self._internal.set_callback_on_scan_found(None)
            self._internal.set_callback_on_scan_updated(None)
            if self._internal.scan_is_active():
                self._internal.scan_stop()
        except:
            pass

    async def __aenter__(self):
        return self

    async def __aexit__(self, exc_type, exc_val, exc_tb):
        self._clear_all_callbacks_sync()