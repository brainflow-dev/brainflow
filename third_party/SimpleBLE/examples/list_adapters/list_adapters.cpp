#include <iostream>

#include "simpleble/Adapter.h"

int main() {
    auto adapter_list = SimpleBLE::Adapter::get_adapters();

    if (adapter_list.size() == 0) {
        std::cout << "No adapters found" << std::endl;
        return 1;
    }

    for (auto& adapter : adapter_list) {
        std::cout << "Adapter: " << adapter.identifier() << " [" << adapter.address() << "]" << std::endl;
    }

    return 0;
}
