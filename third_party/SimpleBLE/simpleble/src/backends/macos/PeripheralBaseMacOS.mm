#import "PeripheralBaseMacOS.h"
#import "CharacteristicBuilder.h"
#import "DescriptorBuilder.h"
#import "LoggingInternal.h"
#import "ServiceBuilder.h"
#import "Utils.h"

#import <simpleble/Exceptions.h>
#import <optional>

#define WAIT_UNTIL_FALSE(obj, var)                \
    do {                                          \
        BOOL _tmpVar = YES;                       \
        while (_tmpVar) {                         \
            [NSThread sleepForTimeInterval:0.01]; \
            @synchronized(obj) {                  \
                _tmpVar = (var);                  \
            }                                     \
        }                                         \
    } while (0)

#define WAIT_UNTIL_FALSE_WITH_TIMEOUT(obj, var, timeout)                                      \
    do {                                                                                      \
        BOOL _tmpVar = YES;                                                                   \
        NSDate* endDate = [NSDate dateWithTimeInterval:timeout sinceDate:NSDate.now];         \
        while (_tmpVar && [NSDate.now compare:endDate] == NSOrderedAscending) {               \
            [NSThread sleepForTimeInterval:0.01];                                             \
            @synchronized(obj) {                                                              \
                _tmpVar = (var);                                                              \
            }                                                                                 \
        }                                                                                     \
    } while (0)

// --------------------------------------------------

@interface BleTask : NSObject
@property(strong, atomic) NSError* error;
@property(atomic) BOOL pending;
@end

@implementation BleTask
- (instancetype)init {
    self = [super init];
    if (self) {
        _pending = NO;
    }
    return self;
}
@end

@interface DescriptorExtras : NSObject
@property(strong) BleTask* task;
@end

@implementation DescriptorExtras
- (instancetype)init {
    self = [super init];
    if (self) {
        _task = [[BleTask alloc] init];
    }
    return self;
}
@end

@interface CharacteristicExtras : NSObject {
  @public
    std::function<void(SimpleBLE::ByteArray)> valueChangedCallback;
}
@property(strong) BleTask* task;
@property(strong) NSData* value;
@property(strong) NSMutableDictionary<NSString*, DescriptorExtras*>* descriptorExtras;
@end

@implementation CharacteristicExtras
- (instancetype)init {
    self = [super init];
    if (self) {
        _task = [[BleTask alloc] init];
        _descriptorExtras = [[NSMutableDictionary alloc] init];
    }
    return self;
}
@end

// --------------------------------------------------

@interface PeripheralBaseMacOS () {}

@property(strong) CBPeripheral* peripheral;
@property(strong) CBCentralManager* centralManager;
@property(strong, atomic) BleTask* task;
@property(strong, atomic) NSError* disconnectionError;
@property(strong) NSMutableDictionary<NSString*, CharacteristicExtras*>* characteristicExtras;

- (CBService*)findService:(NSString*)uuid;
- (CBCharacteristic*)findCharacteristic:(NSString*)uuid service:(CBService*)service;
- (std::pair<CBService*, CBCharacteristic*>)findServiceAndCharacteristic:(NSString*)service_uuid
                                                     characteristic_uuid:(NSString*)characteristic_uuid;
- (void)throwBasedOnError:(NSError*)error withFormat:(NSString*)format, ...;

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
        _characteristicExtras = [[NSMutableDictionary alloc] init];
        _task = [[BleTask alloc] init];
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

- (uint16_t)mtu {
    return [self.peripheral maximumWriteValueLengthForType:CBCharacteristicWriteWithoutResponse];
}

- (void)connect {
    @synchronized(_task) {
        // --- Connect to the peripheral ---
        @synchronized(self) {
            _task.error = nil;
            _task.pending = YES;
            [self.centralManager connectPeripheral:self.peripheral options:@{}];  // TODO: Do we need to pass any options?
        }

        WAIT_UNTIL_FALSE_WITH_TIMEOUT(self, _task.pending, 5.0);

        if (self.peripheral.state != CBPeripheralStateConnected || _task.error != nil) {
            [self throwBasedOnError:_task.error withFormat:@"Peripheral Connection"];
        }

        // --- Discover services and characteristics ---

        @synchronized(self) {
            _task.error = nil;
            _task.pending = YES;
            [self.peripheral discoverServices:nil];
        }

        WAIT_UNTIL_FALSE(self, _task.pending);

        if (self.peripheral.state != CBPeripheralStateConnected) {
            [self throwBasedOnError:_disconnectionError withFormat:@"Service Discovery"];
        }

        if (self.peripheral.services == nil || _task.error != nil) {
            [self throwBasedOnError:_task.error withFormat:@"Service Discovery"];
        }

        // For each service found, discover characteristics.
        for (CBService* service in self.peripheral.services) {
            @synchronized(self) {
                _task.error = nil;
                _task.pending = YES;
                [self.peripheral discoverCharacteristics:nil forService:service];
            }

            WAIT_UNTIL_FALSE(self, _task.pending);

            if (self.peripheral.state != CBPeripheralStateConnected) {
                [self throwBasedOnError:_disconnectionError withFormat:@"Characteristic Discovery for service %@", service.UUID];
            }

            if (service.characteristics == nil || _task.error != nil) {
                [self throwBasedOnError:_task.error withFormat:@"Characteristic Discovery for service %@", service.UUID];
            }

            // For each characteristic, create the associated extra properties and discover descriptors.
            for (CBCharacteristic* characteristic in service.characteristics) {
                CharacteristicExtras* characteristicExtras = [[CharacteristicExtras alloc] init];

                @synchronized(self) {
                    _task.error = nil;
                    _task.pending = YES;
                    [self.peripheral discoverDescriptorsForCharacteristic:characteristic];
                }

                WAIT_UNTIL_FALSE(self, _task.pending);

                if (self.peripheral.state != CBPeripheralStateConnected) {
                    [self throwBasedOnError:_disconnectionError
                                 withFormat:@"Descriptor Discovery for characteristic %@", characteristic.UUID];
                }

                if (characteristic.descriptors == nil || _task.error != nil) {
                    [self throwBasedOnError:_task.error withFormat:@"Descriptor Discovery for characteristic %@", characteristic.UUID];
                }

                // For each descriptor, create the associated extra properties.
                for (CBDescriptor* descriptor in characteristic.descriptors) {
                    @synchronized(self) {
                        [characteristicExtras.descriptorExtras setObject:[[DescriptorExtras alloc] init]
                                                                  forKey:[[descriptor.UUID UUIDString] lowercaseString]];
                    }
                }

                @synchronized(self) {
                    [self.characteristicExtras setObject:characteristicExtras forKey:[[characteristic.UUID UUIDString] lowercaseString]];
                }
            }
        }
    }
}

- (void)disconnect {
    @synchronized(_task) {
        @synchronized(self) {
            _task.error = nil;
            _task.pending = YES;
            [self.centralManager cancelPeripheralConnection:self.peripheral];
        }

        WAIT_UNTIL_FALSE(self, _task.pending);

        if (self.peripheral.state != CBPeripheralStateDisconnected) {
            [self throwBasedOnError:_task.error withFormat:@"Peripheral Disconnection"];
        }
    }
}

- (bool)isConnected {
    return self.peripheral.state == CBPeripheralStateConnected;
}

- (std::vector<SimpleBLE::Service>)getServices {
    std::vector<SimpleBLE::Service> service_list;
    for (CBService* service in self.peripheral.services) {
        // Build the list of characteristics for the service.
        std::vector<SimpleBLE::Characteristic> characteristic_list;
        for (CBCharacteristic* characteristic in service.characteristics) {
            // Build the list of descriptors for the characteristic.
            std::vector<SimpleBLE::Descriptor> descriptor_list;
            for (CBDescriptor* descriptor in characteristic.descriptors) {
                descriptor_list.push_back(SimpleBLE::DescriptorBuilder(uuidToSimpleBLE(descriptor.UUID)));
            }

            bool can_read = (characteristic.properties & CBCharacteristicPropertyRead) != 0;
            bool can_write_request = (characteristic.properties & CBCharacteristicPropertyWrite) != 0;
            bool can_write_command = (characteristic.properties & CBCharacteristicPropertyWriteWithoutResponse) != 0;
            bool can_notify = (characteristic.properties & CBCharacteristicPropertyNotify) != 0;
            bool can_indicate = (characteristic.properties & CBCharacteristicPropertyIndicate) != 0;

            characteristic_list.push_back(SimpleBLE::CharacteristicBuilder(uuidToSimpleBLE(characteristic.UUID), descriptor_list, can_read,
                                                                           can_write_request, can_write_command, can_notify, can_indicate));
        }
        service_list.push_back(SimpleBLE::ServiceBuilder(uuidToSimpleBLE(service.UUID), characteristic_list));
    }

    return service_list;
}

- (SimpleBLE::ByteArray)read:(NSString*)service_uuid characteristic_uuid:(NSString*)characteristic_uuid {
    std::pair<CBService*, CBCharacteristic*> serviceAndCharacteristic = [self findServiceAndCharacteristic:service_uuid
                                                                                       characteristic_uuid:characteristic_uuid];

    CBCharacteristic* characteristic = serviceAndCharacteristic.second;

    // Check that the characteristic supports this feature.
    if ((characteristic.properties & CBCharacteristicPropertyRead) == 0) {
        NSLog(@"Characteristic does not support read.");
        throw SimpleBLE::Exception::OperationNotSupported();
    }

    CharacteristicExtras* characteristicExtras = [self.characteristicExtras objectForKey:characteristic_uuid];
    BleTask* task = characteristicExtras.task;

    @synchronized(task) {
        if (characteristic.isNotifying) {
            // If the characteristic is already notifying, we'll just wait for the next notification.
            @synchronized(self) {
                characteristicExtras.value = nil;
            }

            WAIT_UNTIL_FALSE(self, (characteristicExtras.value == nil));

            return SimpleBLE::ByteArray((const char*)characteristicExtras.value.bytes, characteristicExtras.value.length);

        } else {
            // Otherwise, we'll trigger a value read and wait for the response.
            @synchronized(self) {
                task.error = nil;
                task.pending = YES;
                [self.peripheral readValueForCharacteristic:characteristic];
            }

            WAIT_UNTIL_FALSE(self, task.pending);

            if (task.error != nil) {
                [self throwBasedOnError:task.error withFormat:@"Characteristic %@ Read", characteristic.UUID];
            }

            return SimpleBLE::ByteArray((const char*)characteristic.value.bytes, characteristic.value.length);
        }
    }
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

    CharacteristicExtras* characteristicExtras = [self.characteristicExtras objectForKey:characteristic_uuid];
    BleTask* task = characteristicExtras.task;

    @synchronized(task) {
        @synchronized(self) {
            task.error = nil;
            task.pending = YES;
            [self.peripheral writeValue:payload forCharacteristic:characteristic type:CBCharacteristicWriteWithResponse];
        }

        WAIT_UNTIL_FALSE(self, task.pending);

        if (task.error != nil) {
            [self throwBasedOnError:task.error withFormat:@"Characteristic %@ Write Request", characteristic.UUID];
        }
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

    CharacteristicExtras* characteristicExtras = [self.characteristicExtras objectForKey:characteristic_uuid];
    BleTask* task = characteristicExtras.task;

    @synchronized(task) {
        // NOTE: This write is unacknowledged.
        @synchronized(self) {
            [self.peripheral writeValue:payload forCharacteristic:characteristic type:CBCharacteristicWriteWithoutResponse];
        }
    }
}

- (void)notify:(NSString*)service_uuid
    characteristic_uuid:(NSString*)characteristic_uuid
               callback:(std::function<void(SimpleBLE::ByteArray)>)callback {
    std::pair<CBService*, CBCharacteristic*> serviceAndCharacteristic = [self findServiceAndCharacteristic:service_uuid
                                                                                       characteristic_uuid:characteristic_uuid];

    CBCharacteristic* characteristic = serviceAndCharacteristic.second;

    CharacteristicExtras* characteristicExtras = [self.characteristicExtras objectForKey:characteristic_uuid];
    BleTask* task = characteristicExtras.task;

    @synchronized(task) {
        @synchronized(self) {
            task.error = nil;
            task.pending = YES;
            characteristicExtras->valueChangedCallback = callback;
            [self.peripheral setNotifyValue:YES forCharacteristic:characteristic];
        }

        WAIT_UNTIL_FALSE(self, task.pending);

        if (!characteristic.isNotifying || task.error != nil) {
            [self throwBasedOnError:task.error withFormat:@"Characteristic %@ Notify/Indicate", characteristic.UUID];
        }
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

    CBCharacteristic* characteristic = serviceAndCharacteristic.second;

    CharacteristicExtras* characteristicExtras = [self.characteristicExtras objectForKey:characteristic_uuid];
    BleTask* task = characteristicExtras.task;

    @synchronized(task) {
        @synchronized(self) {
            task.error = nil;
            task.pending = YES;
            characteristicExtras->valueChangedCallback = nil;
            [self.peripheral setNotifyValue:NO forCharacteristic:characteristic];
        }

        WAIT_UNTIL_FALSE(self, task.pending);

        if (characteristic.isNotifying || task.error != nil) {
            [self throwBasedOnError:task.error withFormat:@"Characteristic %@ Unsubscribe", characteristic.UUID];
        }
    }
}

- (SimpleBLE::ByteArray)read:(NSString*)service_uuid
         characteristic_uuid:(NSString*)characteristic_uuid
             descriptor_uuid:(NSString*)descriptor_uuid {
    std::pair<CBService*, CBCharacteristic*> serviceAndCharacteristic = [self findServiceAndCharacteristic:service_uuid
                                                                                       characteristic_uuid:characteristic_uuid];

    CBCharacteristic* characteristic = serviceAndCharacteristic.second;
    CBDescriptor* descriptor = [self findDescriptor:descriptor_uuid characteristic:characteristic];

    CharacteristicExtras* characteristicExtras = [self.characteristicExtras objectForKey:characteristic_uuid];
    DescriptorExtras* descriptorExtras = [characteristicExtras.descriptorExtras objectForKey:descriptor_uuid];
    BleTask* task = descriptorExtras.task;

    @synchronized(task) {
        @synchronized(self) {
            task.error = nil;
            task.pending = YES;
            [self.peripheral readValueForDescriptor:descriptor];
        }

        WAIT_UNTIL_FALSE(self, task.pending);

        if (task.error != nil) {
            [self throwBasedOnError:task.error withFormat:@"Descriptor %@ Read", descriptor.UUID];
        }

        const char* bytes = (const char*)[descriptor.value bytes];

        return SimpleBLE::ByteArray(bytes, [descriptor.value length]);
    }
}

- (void)write:(NSString*)service_uuid
    characteristic_uuid:(NSString*)characteristic_uuid
        descriptor_uuid:(NSString*)descriptor_uuid
                payload:(NSData*)payload {
    std::pair<CBService*, CBCharacteristic*> serviceAndCharacteristic = [self findServiceAndCharacteristic:service_uuid
                                                                                       characteristic_uuid:characteristic_uuid];

    CBCharacteristic* characteristic = serviceAndCharacteristic.second;
    CBDescriptor* descriptor = [self findDescriptor:descriptor_uuid characteristic:characteristic];

    CharacteristicExtras* characteristicExtras = [self.characteristicExtras objectForKey:characteristic_uuid];
    DescriptorExtras* descriptorExtras = [characteristicExtras.descriptorExtras objectForKey:descriptor_uuid];
    BleTask* task = descriptorExtras.task;

    @synchronized(task) {
        @synchronized(self) {
            task.error = nil;
            task.pending = YES;
            [self.peripheral writeValue:payload forDescriptor:descriptor];
        }

        WAIT_UNTIL_FALSE(self, task.pending);

        if (task.error) {
            [self throwBasedOnError:task.error withFormat:@"Descriptor %@ Write", descriptor.UUID];
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

- (CBDescriptor*)findDescriptor:(NSString*)uuid characteristic:(CBCharacteristic*)characteristic {
    CBUUID* descriptor_uuid = [CBUUID UUIDWithString:uuid];

    for (CBDescriptor* descriptor in characteristic.descriptors) {
        if ([descriptor.UUID isEqual:descriptor_uuid]) {
            return descriptor;
        }
    }

    throw SimpleBLE::Exception::DescriptorNotFound([uuid UTF8String]);
}

- (std::pair<CBService*, CBCharacteristic*>)findServiceAndCharacteristic:(NSString*)service_uuid
                                                     characteristic_uuid:(NSString*)characteristic_uuid {
    CBService* service = [self findService:service_uuid];
    CBCharacteristic* characteristic = [self findCharacteristic:characteristic_uuid service:service];
    return std::pair<CBService*, CBCharacteristic*>(service, characteristic);
}

- (void)throwBasedOnError:(NSError*)error withFormat:(NSString*)format, ... {
    va_list argList;
    va_start(argList, format);
    NSString* formattedString = [[NSString alloc] initWithFormat:format arguments:argList];
    va_end(argList);

    if (error == nil) {
        NSString* exceptionMessage = [NSString stringWithFormat:@"%@ failed", formattedString];
        NSLog(@"%@", exceptionMessage);
        throw SimpleBLE::Exception::OperationFailed([exceptionMessage UTF8String]);
    } else {
        NSString* errorMessage = [error localizedDescription];
        NSString* exceptionMessage = [NSString stringWithFormat:@"%@ failed: %@", formattedString, errorMessage];
        NSLog(@"%@", exceptionMessage);
        throw SimpleBLE::Exception::OperationFailed([exceptionMessage UTF8String]);
    }
}

#pragma mark - CBCentralManagerDelegate

- (void)delegateDidConnect {
    @synchronized(self) {
        _task.pending = NO;
    }
}

- (void)delegateDidFailToConnect:(NSError*)error {
    @synchronized(self) {
        _task.error = error;
        _task.pending = NO;
    }
}

- (void)delegateDidDisconnect:(NSError*)error {
    @synchronized(self) {
        self->_disconnectionError = error;
        _task.pending = NO;

        for (CharacteristicExtras* characteristicExtras in self.characteristicExtras.allValues) {
            characteristicExtras.task.pending = NO;
            for (DescriptorExtras* descriptorExtras in characteristicExtras.descriptorExtras.allValues) {
                descriptorExtras.task.pending = NO;
            }
        }
    }
}

#pragma mark - CBPeripheralDelegate

- (void)peripheral:(CBPeripheral*)peripheral didModifyServices:(NSArray<CBService*>*)invalidatedServices {
    // NOTE: Whenever this method is called, any pending operations are cancelled. In addition to that,
    //       the provided list of services does NOT include any characteristics or descriptors, so need to
    //       clear pending flags for those as well.
    @synchronized(self) {
        _task.pending = NO;

        for (CharacteristicExtras* characteristicExtras in self.characteristicExtras.allValues) {
            characteristicExtras.task.pending = NO;
            for (DescriptorExtras* descriptorExtras in characteristicExtras.descriptorExtras.allValues) {
                descriptorExtras.task.pending = NO;
            }
        }
    }
}

- (void)peripheral:(CBPeripheral*)peripheral didDiscoverServices:(NSError*)error {
    @synchronized(self) {
        _task.error = error;
        _task.pending = NO;
    }
}

- (void)peripheral:(CBPeripheral*)peripheral didDiscoverCharacteristicsForService:(CBService*)service error:(NSError*)error {
    @synchronized(self) {
        _task.error = error;
        _task.pending = NO;
    }
}

- (void)peripheral:(CBPeripheral*)peripheral
    didDiscoverDescriptorsForCharacteristic:(CBCharacteristic*)characteristic
                                      error:(NSError*)error {
    @synchronized(self) {
        _task.error = error;
        _task.pending = NO;
    }
}

- (void)peripheral:(CBPeripheral*)peripheral didUpdateValueForCharacteristic:(CBCharacteristic*)characteristic error:(NSError*)error {
    CharacteristicExtras* characteristicExtras = [self.characteristicExtras objectForKey:[[characteristic.UUID UUIDString] lowercaseString]];

    if (characteristic.isNotifying) {
        // If the characteristic is notifying, just save the value and trigger the callback.
        @synchronized(self) {
            characteristicExtras.value = characteristic.value;
        }

        if (characteristicExtras->valueChangedCallback != nil) {
            characteristicExtras->valueChangedCallback(
                SimpleBLE::ByteArray((const char*)characteristic.value.bytes, characteristic.value.length));
        }

    } else {
        // If the characteristic is not notifying, then this is a response to a read request.
        BleTask* task = characteristicExtras.task;
        @synchronized(self) {
            task.error = error;
            task.pending = NO;
        }
    }
}

- (void)peripheral:(CBPeripheral*)peripheral didWriteValueForCharacteristic:(CBCharacteristic*)characteristic error:(NSError*)error {
    CharacteristicExtras* characteristicExtras = [self.characteristicExtras objectForKey:[[characteristic.UUID UUIDString] lowercaseString]];
    BleTask* task = characteristicExtras.task;

    @synchronized(self) {
        task.error = error;
        task.pending = NO;
    }
}

- (void)peripheral:(CBPeripheral*)peripheral
    didUpdateNotificationStateForCharacteristic:(CBCharacteristic*)characteristic
                                          error:(NSError*)error {
    CharacteristicExtras* characteristicExtras = [self.characteristicExtras objectForKey:[[characteristic.UUID UUIDString] lowercaseString]];
    BleTask* task = characteristicExtras.task;

    @synchronized(self) {
        task.error = error;
        task.pending = NO;
    }
}

- (void)peripheralIsReadyToSendWriteWithoutResponse:(CBPeripheral*)peripheral {
    NSLog(@"Peripheral ready to send: %@", peripheral);
}

- (void)peripheral:(CBPeripheral*)peripheral didUpdateValueForDescriptor:(CBDescriptor*)descriptor error:(NSError*)error {
    CharacteristicExtras* characteristicExtras = [self.characteristicExtras objectForKey:[[descriptor.characteristic.UUID UUIDString] lowercaseString]];
    DescriptorExtras* descriptorExtras = [characteristicExtras.descriptorExtras objectForKey:[[descriptor.UUID UUIDString] lowercaseString]];
    BleTask* task = descriptorExtras.task;

    @synchronized(self) {
        task.error = error;
        task.pending = NO;
    }
}

- (void)peripheral:(CBPeripheral*)peripheral didWriteValueForDescriptor:(CBDescriptor*)descriptor error:(NSError*)error {
    CharacteristicExtras* characteristicExtras = [self.characteristicExtras objectForKey:[[descriptor.characteristic.UUID UUIDString] lowercaseString]];
    DescriptorExtras* descriptorExtras = [characteristicExtras.descriptorExtras objectForKey:[[descriptor.UUID UUIDString] lowercaseString]];
    BleTask* task = descriptorExtras.task;

    @synchronized(self) {
        task.error = error;
        task.pending = NO;
    }
}

@end
