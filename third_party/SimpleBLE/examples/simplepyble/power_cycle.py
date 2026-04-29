import simplepyble
import time

if __name__ == "__main__":
    print(f"Running on {simplepyble.get_operating_system()}")

    adapters = simplepyble.Adapter.get_adapters()

    if len(adapters) == 0:
        print("No adapters found")

    for adapter in adapters:
        print(f"Adapter: {adapter.identifier()} [{adapter.address()}]")

        print("Powering off adapter")
        adapter.power_off()
        print("Adapter powered: ", adapter.is_powered())
        time.sleep(1)
        print("Powering on adapter")
        adapter.power_on()
        print("Adapter powered: ", adapter.is_powered())
