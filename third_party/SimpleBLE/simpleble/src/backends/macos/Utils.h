#pragma once

#import <CoreBluetooth/CoreBluetooth.h>
#import <Foundation/Foundation.h>

#include <simpleble/Types.h>

SimpleBLE::BluetoothUUID uuidToSimpleBLE(CBUUID* uuid);
NSString* uuidToString(CBUUID* uuid);
