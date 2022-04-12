import signal
import asyncio
import dbus_next
from dbus_next.aio import MessageBus
from interfaces import Emulator

active = True
bus = None


def handler(signum, frame):
    global bus
    global active
    bus.disconnect()
    active = False


async def setup():
    global bus
    bus = await MessageBus(bus_type=dbus_next.BusType.SESSION).connect()

    # Create object instances
    emulator = Emulator(bus)

    # Register object instances
    emulator.export()
    
    # Request bus name
    await bus.request_name("simplebluez.tester.python")


async def main():
    global active
    while active:
        await asyncio.sleep(0.1)


if __name__ == "__main__":
    signal.signal(signal.SIGINT, handler)
    asyncio.get_event_loop().run_until_complete(setup())
    asyncio.get_event_loop().run_until_complete(main())
