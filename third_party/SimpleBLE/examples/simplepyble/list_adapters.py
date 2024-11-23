import simplepyble

if __name__ == "__main__":
    print(f"Running on {simplepyble.get_operating_system()}")

    adapters = simplepyble.Adapter.get_adapters()

    if len(adapters) == 0:
        print("No adapters found")

    for adapter in adapters:
        print(f"Adapter: {adapter.identifier()} [{adapter.address()}]")
