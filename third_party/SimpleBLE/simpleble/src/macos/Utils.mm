#import "Utils.h"

SimpleBLE::BluetoothUUID uuidToSimpleBLE(CBUUID* uuid) {
    NSString* uuidString = [uuid UUIDString];
    return uuidString.lowercaseString.UTF8String;
}
