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

    choice = int(input("Enter choice: "))
    adapter = adapters[choice]

    print(f"Selected adapter: {adapter.identifier()} [{adapter.address()}]")

    # Scan for 5 seconds
    print("Scanning for 5 seconds...")
    await adapter.scan_for(5000)
    peripherals = adapter.scan_get_results()

    if not peripherals:
        print("No peripherals found.")
        return

    # Query the user to pick a peripheral
    print("Please select a peripheral:")
    for i, peripheral in enumerate(peripherals):
        print(f"{i}: {peripheral.identifier()} [{peripheral.address()}]")

    choice = int(input("Enter choice: "))
    peripheral = peripherals[choice]

    print(f"Connecting to: {peripheral.identifier()} [{peripheral.address()}]")
    
    # Use context manager for automatic disconnection and cleanup
    async with peripheral:
        await peripheral.connect()
        print("Successfully connected, listing services...")
        
        services = peripheral.services()
        service_characteristic_pair = []
        for service in services:
            for characteristic in service.characteristics():
                service_characteristic_pair.append((service.uuid(), characteristic.uuid()))

        if not service_characteristic_pair:
            print("No services or characteristics found.")
            return

        # Query the user to pick a service/characteristic pair
        print("Please select a service/characteristic pair:")
        for i, (service_uuid, characteristic) in enumerate(service_characteristic_pair):
            print(f"{i}: {service_uuid} {characteristic}")

        choice = int(input("Enter choice: "))
        service_uuid, characteristic_uuid = service_characteristic_pair[choice]

        # Async callback example
        async def on_notification(data):
            print(f"Notification received: {data}")

        await peripheral.notify(service_uuid, characteristic_uuid, on_notification)
        print("Subscribed! Waiting for notifications for 10 seconds...")
        await asyncio.sleep(10)

        await peripheral.unsubscribe(service_uuid, characteristic_uuid)
        print("Unsubscribed.")

if __name__ == "__main__":
    asyncio.run(main())
