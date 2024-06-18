# Note: This test suite is only evaluating the Python bindings, not the C++ library.
#       The SimpleBLE implementation to test this on is the PLAIN version.
import simplepyble


def test_get_adapters():
    assert simplepyble.Adapter.bluetooth_enabled() == True

    adapters = simplepyble.Adapter.get_adapters()
    assert len(adapters) == 1

    adapter = adapters[0]
    assert adapter.identifier() == "Plain Adapter"
    assert adapter.address() == "AA:BB:CC:DD:EE:FF"


def test_scan_blocking():
    adapter = simplepyble.Adapter.get_adapters()[0]

    adapter.scan_for(1)
    peripherals = adapter.scan_get_results()
    assert len(peripherals) == 1

    peripheral = peripherals[0]
    assert peripheral.identifier() == "Plain Peripheral"
    assert peripheral.address() == "11:22:33:44:55:66"
    assert peripheral.rssi() == -60
    assert peripheral.is_connected() == False
    assert peripheral.is_paired() == False


def test_scan_async():
    # TODO: Implement once we have proper callback and advertising emulation.
    pass


def test_connect():
    adapter = simplepyble.Adapter.get_adapters()[0]

    adapter.scan_for(1)
    peripherals = adapter.scan_get_results()
    peripheral = peripherals[0]

    peripheral.connect()
    assert peripheral.is_connected() == True
    assert peripheral.is_paired() == True

    services = peripheral.services()
    assert len(services) == 1

    service = services[0]
    assert service.uuid() == "0000180f-0000-1000-8000-00805f9b34fb"

    characteristics = service.characteristics()
    assert len(characteristics) == 1
    
    characteristic = characteristics[0]
    assert characteristic.uuid() == "00002a19-0000-1000-8000-00805f9b34fb"

    peripheral.disconnect()
    assert peripheral.is_connected() == False
    assert peripheral.is_paired() == True