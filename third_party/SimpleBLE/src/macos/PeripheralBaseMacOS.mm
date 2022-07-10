#import "PeripheralBaseMacOS.h"
#import "Utils.h"

#import <simpleble/Exceptions.h>

typedef struct {
    BOOL readPending;
    BOOL writePending;
    std::function<void(SimpleBLE::ByteArray)> valueChangedCallback;
} characteristic_extras_t;

@interface PeripheralBaseMacOS () {
    // NOTE: This dictionary assumes that all characteristic UUIDs are unique, which could not always be the case.
    std::map<std::string, characteristic_extras_t> characteristic_extras_;
}

// Private properties
@property(strong) CBPeripheral* peripheral;
@property(strong) CBCentralManager* centralManager;

- (CBService*)findService:(NSString*)uuid;
- (CBCharacteristic*)findCharacteristic:(NSString*)uuid service:(CBService*)service;
- (std::pair<CBService*, CBCharacteristic*>)findServiceAndCharacteristic:(NSString*)service_uuid
                                                     characteristic_uuid:(NSString*)characteristic_uuid;

@end

@implementation PeripheralBaseMacOS

- (instancetype)init:(CBPeripheral*)peripheral centralManager:(CBCentralManager*)centralManager {
    self = [super init];
    if (self) {
        // NOTE: It's important to make a copy of the peripheral and central objects into
        // a strong property to prevent them from being deallocated by ARC or the garbage collector.
        _peripheral = [peripheral copy];
        _centralManager = centralManager;

        _peripheral.delegate = self;
    }
    return self;
}

- (void*)underlying {
    return (__bridge void*)self.peripheral;
}

- (NSString*)identifier {
    if (self.peripheral.name != nil) {
        return [self.peripheral.name copy];
    } else {
        return @"";
    }
}

- (NSString*)address {
    return [self.peripheral.identifier UUIDString];
}

- (void)connect {
    @synchronized(self) {
        // NSLog(@"Connecting to peripheral: %@", self.peripheral.name);
        [self.centralManager connectPeripheral:self.peripheral options:@{}];  // TODO: Do we need to pass any options?

        NSDate* endDate = nil;

        // Wait for the connection to be established for up to 5 seconds.
        endDate = [NSDate dateWithTimeInterval:5.0 sinceDate:NSDate.now];
        while (self.peripheral.state == CBPeripheralStateConnecting && [NSDate.now compare:endDate] == NSOrderedAscending) {
            [NSThread sleepForTimeInterval:0.01];
        }

        if (self.peripheral.state != CBPeripheralStateConnected) {
            throw SimpleBLE::Exception::OperationFailed();
        }

        [self.peripheral discoverServices:nil];

        // Wait for services to be discovered for up to 1 second.
        // NOTE: This is a bit of a hack but avoids the need of having a dedicated flag.
        endDate = [NSDate dateWithTimeInterval:1.0 sinceDate:NSDate.now];
        while (self.peripheral.services == nil && [NSDate.now compare:endDate] == NSOrderedAscending) {
            [NSThread sleepForTimeInterval:0.01];
        }

        if (self.peripheral.services == nil) {
            // If services could not be discovered, raise an exception.
            NSLog(@"Services could not be discovered.");
            throw SimpleBLE::Exception::OperationFailed();
        }

        // For each service found, discover characteristics.
        for (CBService* service in self.peripheral.services) {
            [self.peripheral discoverCharacteristics:nil forService:service];

            // Wait for characteristics  to be discovered for up to 1 second.
            // NOTE: This is a bit of a hack but avoids the need of having a dedicated flag.
            endDate = [NSDate dateWithTimeInterval:1.0 sinceDate:NSDate.now];
            while (service.characteristics == nil && [NSDate.now compare:endDate] == NSOrderedAscending) {
                [NSThread sleepForTimeInterval:0.01];
            }

            if (service.characteristics == nil) {
                // If characteristics could not be discovered, raise an exception.
                NSLog(@"Characteristics could not be discovered for service %@", service.UUID);
                throw SimpleBLE::Exception::OperationFailed();
            }

            // For each characteristic, create the associated extra properties.
            for (CBCharacteristic* characteristic in service.characteristics) {
                characteristic_extras_t extras;
                extras.readPending = NO;
                characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)] = extras;
            }
        }
    }
}

- (void)disconnect {
    @synchronized(self) {
        // NSLog(@"Disconnecting peripheral: %@ - State was %ld", self.peripheral.name, self.peripheral.state);
        [self.centralManager cancelPeripheralConnection:self.peripheral];

        NSDate* endDate = nil;

        // Wait for the connection to be established for up to 5 seconds.
        endDate = [NSDate dateWithTimeInterval:5.0 sinceDate:NSDate.now];
        while (self.peripheral.state == CBPeripheralStateDisconnecting && [NSDate.now compare:endDate] == NSOrderedAscending) {
            [NSThread sleepForTimeInterval:0.01];
        }

        if (self.peripheral.state != CBPeripheralStateDisconnected) {
            // If the disconnection failed, raise an exception.
            NSLog(@"Disconnection failed.");
            throw SimpleBLE::Exception::OperationFailed();
        }
    }
}

- (bool)isConnected {
    return self.peripheral.state == CBPeripheralStateConnected;
}

- (std::vector<SimpleBLE::BluetoothService>)getServices {
    // NOTE: We might want to return NSUUIDs in this function and convert them to
    // strings in the PeripheralBase class.

    std::vector<SimpleBLE::BluetoothService> services;

    // For each service, load the UUID and the corresponding characteristics.
    for (CBService* service in self.peripheral.services) {
        SimpleBLE::BluetoothService bluetoothService;
        bluetoothService.uuid = uuidToSimpleBLE(service.UUID);

        // Load all the characteristics for this service.
        NSArray<CBCharacteristic*>* characteristics = service.characteristics;
        for (CBCharacteristic* characteristic in characteristics) {
            bluetoothService.characteristics.push_back(uuidToSimpleBLE(characteristic.UUID));
        }

        services.push_back(bluetoothService);
    }

    return services;
}

- (SimpleBLE::ByteArray)read:(NSString*)service_uuid characteristic_uuid:(NSString*)characteristic_uuid {
    std::pair<CBService*, CBCharacteristic*> serviceAndCharacteristic = [self findServiceAndCharacteristic:service_uuid
                                                                                       characteristic_uuid:characteristic_uuid];

    CBCharacteristic* characteristic = serviceAndCharacteristic.second;

    @synchronized(self) {
        characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)].readPending = YES;
        [self.peripheral readValueForCharacteristic:characteristic];
    }

    // Wait for the read to complete for up to 1 second.
    NSDate* endDate = [NSDate dateWithTimeInterval:1.0 sinceDate:NSDate.now];
    BOOL readPending = YES;
    while (readPending && [NSDate.now compare:endDate] == NSOrderedAscending) {
        [NSThread sleepForTimeInterval:0.01];
        @synchronized(self) {
            readPending = characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)].readPending;
        }
    }

    if (readPending) {
        NSLog(@"Characteristic %@ could not be read", characteristic.UUID);
        throw SimpleBLE::Exception::OperationFailed();
    }

    return SimpleBLE::ByteArray((const char*)characteristic.value.bytes, characteristic.value.length);
}

- (void)writeRequest:(NSString*)service_uuid characteristic_uuid:(NSString*)characteristic_uuid payload:(NSData*)payload {
    std::pair<CBService*, CBCharacteristic*> serviceAndCharacteristic = [self findServiceAndCharacteristic:service_uuid
                                                                                       characteristic_uuid:characteristic_uuid];

    CBCharacteristic* characteristic = serviceAndCharacteristic.second;

    // Check that the characteristic supports this feature.
    if ((characteristic.properties & CBCharacteristicPropertyWrite) == 0) {
        NSLog(@"Characteristic does not support write with response.");
        throw SimpleBLE::Exception::OperationNotSupported();
    }

    @synchronized(self) {
        characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)].writePending = YES;
        [self.peripheral writeValue:payload forCharacteristic:characteristic type:CBCharacteristicWriteWithResponse];
    }

    // Wait for the read to complete for up to 1 second.
    NSDate* endDate = [NSDate dateWithTimeInterval:1.0 sinceDate:NSDate.now];
    BOOL writePending = YES;
    while (writePending && [NSDate.now compare:endDate] == NSOrderedAscending) {
        [NSThread sleepForTimeInterval:0.01];
        @synchronized(self) {
            writePending = characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)].writePending;
        }
    }

    if (writePending) {
        NSLog(@"Characteristic %@ could not be written", characteristic.UUID);
        throw SimpleBLE::Exception::OperationFailed();
    }
}

- (void)writeCommand:(NSString*)service_uuid characteristic_uuid:(NSString*)characteristic_uuid payload:(NSData*)payload {
    std::pair<CBService*, CBCharacteristic*> serviceAndCharacteristic = [self findServiceAndCharacteristic:service_uuid
                                                                                       characteristic_uuid:characteristic_uuid];

    CBCharacteristic* characteristic = serviceAndCharacteristic.second;

    // Check that the characteristic supports this feature.
    if ((characteristic.properties & CBCharacteristicPropertyWriteWithoutResponse) == 0) {
        NSLog(@"Characteristic does not support write without response.");
        throw SimpleBLE::Exception::OperationNotSupported();
    }

    // NOTE: This write is unacknowledged.
    @synchronized(self) {
        [self.peripheral writeValue:payload forCharacteristic:characteristic type:CBCharacteristicWriteWithoutResponse];
    }
}

- (void)notify:(NSString*)service_uuid
    characteristic_uuid:(NSString*)characteristic_uuid
               callback:(std::function<void(SimpleBLE::ByteArray)>)callback {
    std::pair<CBService*, CBCharacteristic*> serviceAndCharacteristic = [self findServiceAndCharacteristic:service_uuid
                                                                                       characteristic_uuid:characteristic_uuid];

    CBCharacteristic* characteristic = serviceAndCharacteristic.second;

    @synchronized(self) {
        characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)].valueChangedCallback = callback;
        [self.peripheral setNotifyValue:YES forCharacteristic:characteristic];
    }

    // Wait for the update to complete for up to 1 second.
    NSDate* endDate = [NSDate dateWithTimeInterval:1.0 sinceDate:NSDate.now];
    while (!characteristic.isNotifying && [NSDate.now compare:endDate] == NSOrderedAscending) {
        [NSThread sleepForTimeInterval:0.01];
    }

    if (!characteristic.isNotifying) {
        NSLog(@"Could not enable notifications for characteristic %@", characteristic.UUID);
        throw SimpleBLE::Exception::OperationFailed();
    }
}

- (void)indicate:(NSString*)service_uuid
    characteristic_uuid:(NSString*)characteristic_uuid
               callback:(std::function<void(SimpleBLE::ByteArray)>)callback {
    [self notify:service_uuid characteristic_uuid:characteristic_uuid callback:callback];
}

- (void)unsubscribe:(NSString*)service_uuid characteristic_uuid:(NSString*)characteristic_uuid {
    std::pair<CBService*, CBCharacteristic*> serviceAndCharacteristic = [self findServiceAndCharacteristic:service_uuid
                                                                                       characteristic_uuid:characteristic_uuid];

    @synchronized(self) {
        CBCharacteristic* characteristic = serviceAndCharacteristic.second;
        [self.peripheral setNotifyValue:NO forCharacteristic:characteristic];

        // Wait for the update to complete for up to 2 second.
        NSDate* endDate = [NSDate dateWithTimeInterval:2.0 sinceDate:NSDate.now];
        while (characteristic.isNotifying && [NSDate.now compare:endDate] == NSOrderedAscending) {
            [NSThread sleepForTimeInterval:0.01];
        }

        if (characteristic.isNotifying) {
            NSLog(@"Could not disable notifications for characteristic %@", characteristic.UUID);
            throw SimpleBLE::Exception::OperationFailed();
        } else {
            // Only delete the callback if the characteristic is no longer notifying, to
            // prevent triggering a segfault.
            characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)].valueChangedCallback = nil;
        }
    }
}

#pragma mark - Auxiliary methods

- (CBService*)findService:(NSString*)uuid {
    CBUUID* service_uuid = [CBUUID UUIDWithString:uuid];

    for (CBService* service in self.peripheral.services) {
        if ([service.UUID isEqual:service_uuid]) {
            return service;
        }
    }

    throw SimpleBLE::Exception::ServiceNotFound([uuid UTF8String]);
}

- (CBCharacteristic*)findCharacteristic:(NSString*)uuid service:(CBService*)service {
    CBUUID* characteristic_uuid = [CBUUID UUIDWithString:uuid];

    if (service == nil) {
        throw SimpleBLE::Exception::BaseException("Invalid service parameter.");
    }

    for (CBCharacteristic* characteristic in service.characteristics) {
        if ([characteristic.UUID isEqual:characteristic_uuid]) {
            return characteristic;
        }
    }

    throw SimpleBLE::Exception::CharacteristicNotFound([uuid UTF8String]);
}

- (std::pair<CBService*, CBCharacteristic*>)findServiceAndCharacteristic:(NSString*)service_uuid
                                                     characteristic_uuid:(NSString*)characteristic_uuid {
    CBService* service = [self findService:service_uuid];
    CBCharacteristic* characteristic = [self findCharacteristic:characteristic_uuid service:service];
    return std::pair<CBService*, CBCharacteristic*>(service, characteristic);
}

#pragma mark - CBCentralManagerDelegate

- (void)delegateDidConnect {
    // NOTE: As the connection process is polling-based, this callback is not needed,
    // but might be useful in the future.
}

- (void)delegateDidDisconnect {
    // NOTE: We're keeping this callback for potential future use.
}

#pragma mark - CBPeripheralDelegate

- (void)peripheral:(CBPeripheral*)peripheral didDiscoverServices:(NSError*)error {
    // NOTE: As we are currently polling the result of the discovery, this callback is not needed,
    // but might be useful in the future.
    if (error != nil) {
        NSLog(@"Error while discovering services for peripheral %@: %@\n", peripheral.name, error);
    }
}

- (void)peripheral:(CBPeripheral*)peripheral didDiscoverCharacteristicsForService:(CBService*)service error:(NSError*)error {
    // NOTE: As we are currently polling the result of the discovery, this callback is not needed,
    // but might be useful in the future.
    if (error != nil) {
        NSLog(@"Error while discovering characteristics for service %@: %@\n", service.UUID, error);
    }
}

- (void)peripheral:(CBPeripheral*)peripheral didUpdateValueForCharacteristic:(CBCharacteristic*)characteristic error:(NSError*)error {
    // NSLog(@"Updated value for characteristic: %@", characteristic.UUID);
    if (error != nil) {
        NSLog(@"Characteristic value update error: %@\n", error);
        return;
    }

    @synchronized(self) {
        // If the characteristic still had a pending read, clear the flag and return
        if (characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)].readPending) {
            characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)].readPending = NO;
            return;
        }

        // Check if the characteristic has a callback and call it
        if (characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)].valueChangedCallback != nil) {
            SimpleBLE::ByteArray received_data((const char*)characteristic.value.bytes, characteristic.value.length);
            characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)].valueChangedCallback(received_data);
        }
    }
}

- (void)peripheral:(CBPeripheral*)peripheral didWriteValueForCharacteristic:(CBCharacteristic*)characteristic error:(NSError*)error {
    // NSLog(@"Wrote value for characteristic: %@", characteristic.UUID);
    if (error != nil) {
        NSLog(@"Error: %@\n", error);
        return;
    }
    @synchronized(self) {
        characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)].writePending = NO;
    }
}

- (void)peripheral:(CBPeripheral*)peripheral
    didUpdateNotificationStateForCharacteristic:(CBCharacteristic*)characteristic
                                          error:(NSError*)error {
    if (error != nil) {
        NSLog(@"Notification state update error: %@\n", error);
    }
}

- (void)peripheralIsReadyToSendWriteWithoutResponse:(CBPeripheral*)peripheral {
    NSLog(@"Peripheral ready to send: %@", peripheral);
}

@end
