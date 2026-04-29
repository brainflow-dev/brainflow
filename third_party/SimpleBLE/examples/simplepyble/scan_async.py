import asyncio
from simplepyble.aio import Adapter

async def main():
    adapters = Adapter.get_adapters()

    if len(adapters) == 0:
        print("No adapters found")
        return

    # Query the user to pick an adapter
    print("Please select an adapter:")
    for i, adapter in enumerate(adapters):
        print(f"{i}: {adapter.identifier()} [{adapter.address()}]")

    # input() is blocking, but it's okay for initialization in an example
    choice = int(input("Enter choice: "))
    adapter = adapters[choice]

    print(f"Selected adapter: {adapter.identifier()} [{adapter.address()}]")

    adapter.set_callback_on_scan_start(lambda: print("Scan started."))
    adapter.set_callback_on_scan_stop(lambda: print("Scan complete."))
    adapter.set_callback_on_scan_found(lambda peripheral: print(f"Found {peripheral.identifier()} [{peripheral.address()}]"))

    # Use the context manager to ensure clean exit
    async with adapter:
        # Scan for 5 seconds
        print("Scanning for 5 seconds...")
        await adapter.scan_for(5000)

        peripherals = adapter.scan_get_results()
        print("
The following peripherals were found:")
        for peripheral in peripherals:
            connectable_str = "Connectable" if peripheral.is_connectable() else "Non-Connectable"
            print(f"{peripheral.identifier()} [{peripheral.address()}] - {connectable_str}")
            print(f'    Address Type: {peripheral.address_type()}')
            print(f'    Tx Power: {peripheral.tx_power()} dBm')

            manufacturer_data = peripheral.manufacturer_data()
            for manufacturer_id, value in manufacturer_data.items():
                print(f"    Manufacturer ID: {manufacturer_id}")
                print(f"    Manufacturer data: {value}")

            services = peripheral.services()
            for service in services:
                print(f"    Service UUID: {service.uuid()}")
                print(f"    Service data: {service.data()}")

if __name__ == "__main__":
    asyncio.run(main())
