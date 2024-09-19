# SimpleRsBLE

The ultimate fully-fledged cross-platform library and bindings for Bluetooth Low Energy (BLE).

## Overview

The [SimpleBLE](https://github.com/OpenBluetoothToolbox/SimpleBLE/) project aims to provide
fully cross-platform BLE libraries and bindings, designed for simplicity and ease of use
with a licencing scheme chosen to be friendly towards commercial use. All specific operating
system quirks are handled internally to provide a consistent behavior across all platforms.
The libraries also provide first-class support for vendorization of all third-party
dependencies, allowing for easy integration into existing projects.

If you want to use SimpleRsBLE and need help. **Please do not hesitate to reach out!**

-   Join our [Discord](https://discord.gg/N9HqNEcvP3) server.
-   Contact me: `kevin at dewald dot me`

## Supported platforms

-   Windows: Windows 10+
-   Linux: Ubuntu 20.04+
-   MacOS: 10.15+ (except 12.0, 12.1, and 12.2)
-   iOS: 15.0+

## Usage

You can add SimpleRsBLE to your project by adding the following lines to your `Cargo.toml`:

```toml
[dependencies]
simplersble = "0.6.0"
```

Please review our [code examples](https://github.com/OpenBluetoothToolbox/SimpleBLE/tree/main/examples/simplersble/src/bin)
on GitHub for more information on how to use SimpleRsBLE.

## Contributing

Pull requests are welcome. For major changes, please open an issue first to discuss
what you would like to change.

# License

Since February 15th 2024, SimpleBLE is now available under the GNU General Public License
version 3 (GPLv3), with the option for a commercial license without the GPLv3 restrictions
available for a fee. (More information on pricing and commercial terms of service will be
available soon.)

To enquire about a commercial license, please contact us at `contact at simpleble dot org`.

Likewise, if you are using SimpleBLE in an open-source project and would like to request
a free commercial license or if you have any other questions, please reach out at `contact at simpleble dot org`.
