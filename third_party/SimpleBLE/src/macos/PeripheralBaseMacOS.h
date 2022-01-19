#pragma once

#import <CoreBluetooth/CoreBluetooth.h>
#import <Foundation/Foundation.h>

#include <functional>
#include <vector>
#include "PeripheralBase.h"

@interface PeripheralBaseMacOS : NSObject<CBPeripheralDelegate>

- (instancetype)init:(CBPeripheral*)peripheral centralManager:(CBCentralManager*)centralManager;

- (NSString*)identifier;
- (NSString*)address;

- (void)connect;
- (void)disconnect;
- (bool)isConnected;
- (std::vector<SimpleBLE::BluetoothService>)getServices;

- (SimpleBLE::ByteArray)read:(NSString*)service_uuid characteristic_uuid:(NSString*)characteristic_uuid;
- (void)writeRequest:(NSString*)service_uuid characteristic_uuid:(NSString*)characteristic_uuid payload:(NSData*)payload;
- (void)writeCommand:(NSString*)service_uuid characteristic_uuid:(NSString*)characteristic_uuid payload:(NSData*)payload;
- (void)notify:(NSString*)service_uuid
    characteristic_uuid:(NSString*)characteristic_uuid
               callback:(std::function<void(SimpleBLE::ByteArray)>)callback;
- (void)indicate:(NSString*)service_uuid
    characteristic_uuid:(NSString*)characteristic_uuid
               callback:(std::function<void(SimpleBLE::ByteArray)>)callback;
- (void)unsubscribe:(NSString*)service_uuid characteristic_uuid:(NSString*)characteristic_uuid;

- (void)delegateDidConnect;
- (void)delegateDidDisconnect;

@end
