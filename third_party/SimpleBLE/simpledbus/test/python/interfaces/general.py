from dbus_next.service import ServiceInterface, method

class Emulator(ServiceInterface):
    def __init__(self, bus):
        self.bus = bus
        super().__init__("simpledbus.tester")

    def export(self):
        self.bus.export("/", self)

    @method()
    def Exit(self):
        """
        Finishes the emulation session by disconnecting from dbus.
        """
        self.bus.disconnect()
