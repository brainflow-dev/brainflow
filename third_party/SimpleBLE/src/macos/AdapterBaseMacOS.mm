#import "AdapterBaseMacOS.h"

@interface AdapterBaseMacOS () {
}

// Private properties
@property() SimpleBLE::AdapterBase* adapter;
@property(strong) dispatch_queue_t centralManagerQueue;
@property(strong) CBCentralManager* centralManager;

// Private methods
- (void)validateCentralManagerState;

@end

@implementation AdapterBaseMacOS

- (instancetype)init:(SimpleBLE::AdapterBase*)adapter {
    self = [super init];
    if (self) {
        _adapter = adapter;

        // TODO: Review dispatch_queue attributes to see if there's a better way to handle this.
        _centralManagerQueue = dispatch_queue_create("AdapterBaseMacOS.centralManagerQueue", DISPATCH_QUEUE_SERIAL);
        _centralManager = [[CBCentralManager alloc] initWithDelegate:self queue:_centralManagerQueue options:nil];

        // Validate authorization state of the central manager.
        if (CBCentralManager.authorization != CBManagerAuthorizationAllowedAlways) {
            // TODO: Convert to error.
            NSLog(@"Application does not have Bluetooth authorization.\n");
        }
    }
    return self;
}

- (void)validateCentralManagerState {
    // Validate the central manager state by checking if it is powered on for up to 5 seconds.
    NSDate* endDate = [NSDate dateWithTimeInterval:5.0 sinceDate:NSDate.now];
    while (self.centralManager.state != CBManagerStatePoweredOn && [NSDate.now compare:endDate] == NSOrderedAscending) {
        [NSThread sleepForTimeInterval:0.01];
    }

    if (self.centralManager.state != CBManagerStatePoweredOn) {
        NSException* myException = [NSException exceptionWithName:@"CBManagerNotPoweredException"
                                                           reason:@"CBManager is not powered on."
                                                         userInfo:nil];
        // TODO: Append current state to exception.
        @throw myException;
    }
}

- (void)scanStart {
    [self validateCentralManagerState];
    [self.centralManager scanForPeripheralsWithServices:nil options:@{CBCentralManagerScanOptionAllowDuplicatesKey : @YES}];
    if (!self.centralManager.isScanning) {
        NSLog(@"Did not start scanning!\n");
        // TODO: Should an exception be thrown here?
    }
}

- (void)scanStop {
    [self.centralManager stopScan];
}

- (bool)scanIsActive {
    return [self.centralManager isScanning];
}

#pragma mark - CBCentralManagerDelegate

- (void)centralManagerDidUpdateState:(CBCentralManager*)central {
    switch (central.state) {
        case CBManagerStateUnknown:
            // NSLog(@"CBManagerStateUnknown!\n");
            break;
        case CBManagerStateResetting:
            // NSLog(@"CBManagerStateResetting!\n");
            break;
        case CBManagerStateUnsupported:
            // NSLog(@"CBManagerStateUnsupported!\n");
            break;
        case CBManagerStateUnauthorized:
            // NSLog(@"CBManagerStateUnauthorized!\n");
            break;
        case CBManagerStatePoweredOff:
            // NSLog(@"CBManagerStatePoweredOff!\n");
            // NOTE: Notify the user that the Bluetooth adapter is turned off.
            break;
        case CBManagerStatePoweredOn:
            // NOTE: This state is required to be able to operate CoreBluetooth.
            // NSLog(@"CBManagerStatePoweredOn!\n");
            break;
    }
}

- (void)centralManager:(CBCentralManager*)central
    didDiscoverPeripheral:(CBPeripheral*)peripheral
        advertisementData:(NSDictionary<NSString*, id>*)advertisementData
                     RSSI:(NSNumber*)RSSI {
    SimpleBLE::advertising_data_t advertisingData;

    // Parse advertisementData to extract all relevant information

    // TODO: Do we need to extract the name as well?
    // NSString* advertisingName = advertisementData[CBAdvertisementDataLocalNameKey];

    // Check if the peripheral is connectable
    NSNumber* isConnectable = advertisementData[CBAdvertisementDataIsConnectable];
    advertisingData.connectable = isConnectable != nil && isConnectable.boolValue == YES;

    // Extract RSSI
    advertisingData.rssi = [RSSI intValue];

    // Extract Manufacturer Data
    NSData* rawManufacturerData = advertisementData[CBAdvertisementDataManufacturerDataKey];
    if (rawManufacturerData != nil) {
        const char* manufacturerDataBytes = (const char*)rawManufacturerData.bytes;

        uint16_t manufacturerID = *((uint16_t*)manufacturerDataBytes);
        SimpleBLE::ByteArray manufacturerData = SimpleBLE::ByteArray(&manufacturerDataBytes[2], (size_t)(rawManufacturerData.length - 2));
        advertisingData.manufacturer_data[manufacturerID] = manufacturerData;
    }

    // TODO: Should we use @synchronized (self)??
    _adapter->delegate_did_discover_peripheral(peripheral, self.centralManager, advertisingData);
}

- (void)centralManager:(CBCentralManager*)central didConnectPeripheral:(CBPeripheral*)peripheral {
    _adapter->delegate_did_connect_peripheral(peripheral);
}

- (void)centralManager:(CBCentralManager*)central didDisconnectPeripheral:(CBPeripheral*)peripheral error:(NSError*)error {
    if (error != nil) {
        NSLog(@"Peripheral %@ disconnected: %@\n", peripheral.name, error);
    }
    _adapter->delegate_did_disconnect_peripheral(peripheral);
}

- (void)centralManager:(CBCentralManager*)central didFailToConnectPeripheral:(CBPeripheral*)peripheral error:(NSError*)error {
    if (error != nil) {
        NSLog(@"Failed to connect to peripheral %@: %@\n", peripheral.name, error);
    }
}

@end
