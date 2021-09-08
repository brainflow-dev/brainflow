#import "PeripheralBaseMacOS.h"
#import "Utils.h"

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
        // a strong property to prevent them from being deallocated by the garbage collector.
        _peripheral = [peripheral copy];
        _centralManager = centralManager;

        _peripheral.delegate = self;
    }
    return self;
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
    // NSLog(@"Connecting to peripheral: %@", self.peripheral.name);
    [self.centralManager connectPeripheral:self.peripheral options:@{}];  // TODO: Do we need to pass any options?

    NSDate* endDate = nil;

    // Wait for the connection to be established for up to 5 seconds.
    endDate = [NSDate dateWithTimeInterval:5.0 sinceDate:NSDate.now];
    while (self.peripheral.state == CBPeripheralStateConnecting && [NSDate.now compare:endDate] == NSOrderedAscending) {
        [NSThread sleepForTimeInterval:0.01];
    }

    if (self.peripheral.state != CBPeripheralStateConnected) {
        // If the connection failed, raise an exception.
        // TODO: Raise an exception.
        NSLog(@"Connection failed.");
        return;
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
        // TODO: Raise an exception.
        NSLog(@"Services could not be discovered.");
        return;
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
            // TODO: Raise an exception.
            NSLog(@"Characteristics could not be discovered for service %@", service.UUID);
        }

        // For each characteristic, create the associated extra properties.
        for (CBCharacteristic* characteristic in service.characteristics) {
            characteristic_extras_t extras;
            extras.readPending = NO;
            characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)] = extras;
        }
    }
}

- (void)disconnect {
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
        // TODO: Raise an exception.
        NSLog(@"Disconnection failed.");
        return;
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

    if (serviceAndCharacteristic.first == nil || serviceAndCharacteristic.second == nil) {
        // TODO: Raise an exception.
        NSLog(@"Could not find service and characteristic.");
        return SimpleBLE::ByteArray();
    }

    CBCharacteristic* characteristic = serviceAndCharacteristic.second;
    characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)].readPending = YES;
    [self.peripheral readValueForCharacteristic:characteristic];

    // Wait for the read to complete for up to 1 second.
    NSDate* endDate = [NSDate dateWithTimeInterval:1.0 sinceDate:NSDate.now];
    while (characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)].readPending && [NSDate.now compare:endDate] == NSOrderedAscending) {
        [NSThread sleepForTimeInterval:0.01];
    }

    if (characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)].readPending) {
        // TODO: Raise an exception.
        NSLog(@"Characteristic %@ could not be read", characteristic.UUID);
        return SimpleBLE::ByteArray();
    }

    return SimpleBLE::ByteArray((const char*)characteristic.value.bytes, characteristic.value.length);
}

- (void)writeRequest:(NSString*)service_uuid characteristic_uuid:(NSString*)characteristic_uuid payload:(NSData*)payload {
    std::pair<CBService*, CBCharacteristic*> serviceAndCharacteristic = [self findServiceAndCharacteristic:service_uuid
                                                                                       characteristic_uuid:characteristic_uuid];

    if (serviceAndCharacteristic.first == nil || serviceAndCharacteristic.second == nil) {
        // TODO: Raise an exception.
        NSLog(@"Could not find service and characteristic.");
    }

    // NOTE: This write is unacknowledged.
    CBCharacteristic* characteristic = serviceAndCharacteristic.second;
    [self.peripheral writeValue:payload forCharacteristic:characteristic type:CBCharacteristicWriteWithoutResponse];
}

- (void)writeCommand:(NSString*)service_uuid characteristic_uuid:(NSString*)characteristic_uuid payload:(NSData*)payload {
    std::pair<CBService*, CBCharacteristic*> serviceAndCharacteristic = [self findServiceAndCharacteristic:service_uuid
                                                                                       characteristic_uuid:characteristic_uuid];

    if (serviceAndCharacteristic.first == nil || serviceAndCharacteristic.second == nil) {
        // TODO: Raise an exception.
        NSLog(@"Could not find service and characteristic.");
    }

    CBCharacteristic* characteristic = serviceAndCharacteristic.second;
    characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)].writePending = YES;
    [self.peripheral writeValue:payload forCharacteristic:characteristic type:CBCharacteristicWriteWithResponse];

    // Wait for the read to complete for up to 1 second.
    NSDate* endDate = [NSDate dateWithTimeInterval:1.0 sinceDate:NSDate.now];
    while (characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)].writePending &&
           [NSDate.now compare:endDate] == NSOrderedAscending) {
        [NSThread sleepForTimeInterval:0.01];
    }

    if (characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)].writePending) {
        // TODO: Raise an exception.
        NSLog(@"Characteristic %@ could not be written", characteristic.UUID);
    }
}

- (void)notify:(NSString*)service_uuid
    characteristic_uuid:(NSString*)characteristic_uuid
               callback:(std::function<void(SimpleBLE::ByteArray)>)callback {
    std::pair<CBService*, CBCharacteristic*> serviceAndCharacteristic = [self findServiceAndCharacteristic:service_uuid
                                                                                       characteristic_uuid:characteristic_uuid];

    if (serviceAndCharacteristic.first == nil || serviceAndCharacteristic.second == nil) {
        // TODO: Raise an exception.
        NSLog(@"Could not find service and characteristic.");
    }

    CBCharacteristic* characteristic = serviceAndCharacteristic.second;
    characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)].valueChangedCallback = callback;
    [self.peripheral setNotifyValue:YES forCharacteristic:characteristic];

    // Wait for the update to complete for up to 1 second.
    NSDate* endDate = [NSDate dateWithTimeInterval:1.0 sinceDate:NSDate.now];
    while (!characteristic.isNotifying && [NSDate.now compare:endDate] == NSOrderedAscending) {
        [NSThread sleepForTimeInterval:0.01];
    }

    if (!characteristic.isNotifying) {
        // TODO: Raise an exception.
        NSLog(@"Could not enable notifications for characteristic %@", characteristic.UUID);
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

    if (serviceAndCharacteristic.first == nil || serviceAndCharacteristic.second == nil) {
        // TODO: Raise an exception.
        NSLog(@"Could not find service and characteristic.");
    }

    CBCharacteristic* characteristic = serviceAndCharacteristic.second;
    [self.peripheral setNotifyValue:NO forCharacteristic:characteristic];

    // Wait for the update to complete for up to 1 second.
    NSDate* endDate = [NSDate dateWithTimeInterval:1.0 sinceDate:NSDate.now];
    while (characteristic.isNotifying && [NSDate.now compare:endDate] == NSOrderedAscending) {
        [NSThread sleepForTimeInterval:0.01];
    }

    if (characteristic.isNotifying) {
        // TODO: Raise an exception.
        NSLog(@"Could not disable notifications for characteristic %@", characteristic.UUID);
    } else {
        // Only delete the callback if the characteristic is no longer notifying, to
        // prevent triggering a segfault.
        characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)].valueChangedCallback = nil;
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

    // TODO Raise an exception.
    return nil;
}

- (CBCharacteristic*)findCharacteristic:(NSString*)uuid service:(CBService*)service {
    CBUUID* characteristic_uuid = [CBUUID UUIDWithString:uuid];

    if (service == nil) {
        // TODO Raise an exception.
        return nil;
    }

    for (CBCharacteristic* characteristic in service.characteristics) {
        if ([characteristic.UUID isEqual:characteristic_uuid]) {
            return characteristic;
        }
    }

    // TODO Raise an exception.
    return nil;
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
        NSLog(@"Error: %@\n", error);
        return;
    }

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

- (void)peripheral:(CBPeripheral*)peripheral didWriteValueForCharacteristic:(CBCharacteristic*)characteristic error:(NSError*)error {
    // NSLog(@"Wrote value for characteristic: %@", characteristic.UUID);
    if (error != nil) {
        NSLog(@"Error: %@\n", error);
        return;
    }

    characteristic_extras_[uuidToSimpleBLE(characteristic.UUID)].writePending = NO;
}

- (void)peripheral:(CBPeripheral*)peripheral
    didUpdateNotificationStateForCharacteristic:(CBCharacteristic*)characteristic
                                          error:(NSError*)error {
    NSLog(@"Updated notification state for characteristic: %@", characteristic.UUID);
    if (error != nil) {
        NSLog(@"Error: %@\n", error);
    }
}

@end
