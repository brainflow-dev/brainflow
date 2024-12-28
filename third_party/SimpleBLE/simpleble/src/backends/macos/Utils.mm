#import "Utils.h"

#import <fmt/core.h>

SimpleBLE::BluetoothUUID uuidToSimpleBLE(CBUUID* uuid) {
    std::string uuid_raw = [[[uuid UUIDString] lowercaseString] UTF8String];

    if (uuid_raw.length() == 4) {
        return fmt::format("0000{}-0000-1000-8000-00805f9b34fb", uuid_raw);
    } else {
        return uuid_raw;
    }
}

NSString* uuidToString(CBUUID* uuid) {
    NSString* uuidString = [[uuid UUIDString] lowercaseString];

    if ([uuidString length] == 4) {
        return [NSString stringWithFormat:@"0000%@-0000-1000-8000-00805f9b34fb", uuidString];
    } else {
        return uuidString;
    }
}
