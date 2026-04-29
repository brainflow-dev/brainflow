#pragma once

#import <CoreBluetooth/CoreBluetooth.h>
#import <Foundation/Foundation.h>

#include <cstdint>
#include <functional>
#include <vector>
#import "PeripheralMac.h"

@interface PeripheralBaseMacOS : NSObject<CBPeripheralDelegate>

- (instancetype)init:(CBPeripheral*)peripheral centralManager:(CBCentralManager*)centralManager;

- (void*)underlying;

- (NSString*)identifier;
- (NSString*)address;
- (uint16_t)mtu;

- (void)connect;
- (void)disconnect;
- (bool)isConnected;
- (std::vector<std::shared_ptr<SimpleBLE::ServiceBase>>)getServices;

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

- (SimpleBLE::ByteArray)read:(NSString*)service_uuid
         characteristic_uuid:(NSString*)characteristic_uuid
             descriptor_uuid:(NSString*)descriptor_uuid;
- (void)write:(NSString*)service_uuid
    characteristic_uuid:(NSString*)characteristic_uuid
        descriptor_uuid:(NSString*)descriptor_uuid
                payload:(NSData*)payload;

- (void)delegateDidConnect;
- (void)delegateDidFailToConnect:(NSError*)error;
- (void)delegateDidDisconnect:(NSError*)error;

@end
