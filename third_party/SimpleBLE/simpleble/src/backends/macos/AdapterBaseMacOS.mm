#import "AdapterBaseMacOS.h"
#import "TargetConditionals.h"

#import <fmt/core.h>
#import <simpleble/Exceptions.h>
#import "LoggingInternal.h"
#import "Utils.h"

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

#define WAIT_UNTIL_FALSE_WITH_TIMEOUT(obj, var, timeout)                              \
    do {                                                                              \
        BOOL _tmpVar = YES;                                                           \
        NSDate* endDate = [NSDate dateWithTimeInterval:timeout sinceDate:NSDate.now]; \
        while (_tmpVar && [NSDate.now compare:endDate] == NSOrderedAscending) {       \
            [NSThread sleepForTimeInterval:0.01];                                     \
            @synchronized(obj) {                                                      \
                _tmpVar = (var);                                                      \
            }                                                                         \
        }                                                                             \
    } while (0)

#if TARGET_OS_OSX
extern "C" {
// Credit to the Chromium project for finding and documenting this undocumented API.
//
// Undocumented IOBluetooth Preference API [1]. Used by `blueutil` [2] and
// `Karabiner` [3] to programmatically control the Bluetooth state. Calling the
// method with `1` powers the adapter on, calling it with `0` powers it off.
// Using this API has the same effect as turning Bluetooth on or off using the
// macOS System Preferences [4], and will effect all adapters.
//
// [1] https://goo.gl/Gbjm1x
// [2] http://www.frederikseiffert.de/blueutil/
// [3] https://pqrs.org/osx/karabiner/
// [4] https://support.apple.com/kb/PH25091
void IOBluetoothPreferenceSetControllerPowerState(int state);
}
#endif
@interface AdapterBaseMacOS () {
}

// Private properties
@property() SimpleBLE::AdapterMac* adapter;
@property(strong) dispatch_queue_t centralManagerQueue;
@property(strong) CBCentralManager* centralManager;

@end

@implementation AdapterBaseMacOS

- (bool)isBluetoothEnabled {
    return CBCentralManager.authorization == CBManagerAuthorizationAllowedAlways && _centralManager.state == CBManagerStatePoweredOn;
}

- (instancetype)init:(SimpleBLE::AdapterMac*)adapter {
    self = [super init];
    if (self) {
        _adapter = adapter;
        _uuid = @"39a76676-2788-46c9-afa0-f0c0c31e6fd9";

        // Use a high-priority queue to ensure that events are processed immediately.
        dispatch_queue_attr_t qos = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, QOS_CLASS_USER_INITIATED, -1);
        _centralManagerQueue = dispatch_queue_create("AdapterBaseMacOS.centralManagerQueue", qos);
        _centralManager = [[CBCentralManager alloc] initWithDelegate:self queue:_centralManagerQueue options:nil];

        // Wait for the central manager state to be updated for up to 5 seconds.
        NSDate* endDate = [NSDate dateWithTimeInterval:5.0 sinceDate:NSDate.now];
        while (_centralManager.state == CBManagerStateUnknown && [NSDate.now compare:endDate] == NSOrderedAscending) {
            [NSThread sleepForTimeInterval:0.01];
        }
    }
    return self;
}

- (void*)underlying {
    return (__bridge void*)self.centralManager;
}

- (void)powerOn {
#if TARGET_OS_OSX
    IOBluetoothPreferenceSetControllerPowerState(1);

    // Wait for the central manager state to be updated for up to 5 seconds.
    NSDate* endDate = [NSDate dateWithTimeInterval:5.0 sinceDate:NSDate.now];
    while (_centralManager.state != CBManagerStatePoweredOn && [NSDate.now compare:endDate] == NSOrderedAscending) {
        [NSThread sleepForTimeInterval:0.01];
    }
#endif
}

- (void)powerOff {
#if TARGET_OS_OSX
    IOBluetoothPreferenceSetControllerPowerState(0);

    // Wait for the central manager state to be updated for up to 5 seconds.
    NSDate* endDate = [NSDate dateWithTimeInterval:5.0 sinceDate:NSDate.now];
    while (_centralManager.state != CBManagerStatePoweredOff && [NSDate.now compare:endDate] == NSOrderedAscending) {
        [NSThread sleepForTimeInterval:0.01];
    }
#endif
}

- (bool)isPowered {
    return self.centralManager.state == CBManagerStatePoweredOn;
}

- (void)scanStart {
    [self.centralManager scanForPeripheralsWithServices:nil options:@{CBCentralManagerScanOptionAllowDuplicatesKey : @YES}];
}

- (void)scanStop {
    [self.centralManager stopScan];
}

- (bool)scanIsActive {
    return [self.centralManager isScanning];
}

- (NSString*)address {
    return self.uuid;
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
            // NSLog(@"CBManagerStatePoweredOn!\n");
            // NOTE: This state is required to be able to operate CoreBluetooth.
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
    advertisingData.rssi = [RSSI shortValue];

    // Extract Tx Power
    NSNumber* txPower = [advertisementData objectForKey:CBAdvertisementDataTxPowerLevelKey];
    if (txPower == nil) {
        advertisingData.tx_power = INT16_MIN;
    } else {
        advertisingData.tx_power = [txPower shortValue];
    }

    // Extract Manufacturer Data
    NSData* rawManufacturerData = advertisementData[CBAdvertisementDataManufacturerDataKey];
    if (rawManufacturerData != nil) {
        const char* manufacturerDataBytes = (const char*)rawManufacturerData.bytes;

        uint16_t manufacturerID = *((uint16_t*)manufacturerDataBytes);
        SimpleBLE::ByteArray manufacturerData = SimpleBLE::ByteArray(&manufacturerDataBytes[2], (size_t)(rawManufacturerData.length - 2));
        advertisingData.manufacturer_data[manufacturerID] = manufacturerData;
    }

    // Extract Service Data
    NSDictionary* rawServiceDataDict = advertisementData[CBAdvertisementDataServiceDataKey];
    for (CBUUID* serviceUuid in rawServiceDataDict) {
        NSData* rawServiceData = rawServiceDataDict[serviceUuid];
        const char* rawServiceDataBytes = (const char*)rawServiceData.bytes;
        size_t rawServiceDataLength = (size_t)rawServiceData.length;
        SimpleBLE::ByteArray serviceData = SimpleBLE::ByteArray(rawServiceDataBytes, rawServiceDataLength);
        advertisingData.service_data[uuidToSimpleBLE(serviceUuid)] = serviceData;
    }

    // Extract Service UUIDs
    NSArray* services = advertisementData[CBAdvertisementDataServiceUUIDsKey];
    if (services != nil) {
        for (CBUUID* serviceUuid in services) {
            advertisingData.service_data[uuidToSimpleBLE(serviceUuid)] = SimpleBLE::ByteArray();
        }
    }

    _adapter->delegate_did_discover_peripheral((__bridge void*)peripheral, (__bridge void*)self.centralManager, advertisingData);
}

- (void)centralManager:(CBCentralManager*)central didConnectPeripheral:(CBPeripheral*)peripheral {
    _adapter->delegate_did_connect_peripheral((__bridge void*)peripheral);
}

- (void)centralManager:(CBCentralManager*)central didDisconnectPeripheral:(CBPeripheral*)peripheral error:(NSError*)error {
    // NSLog(@"didDisconnectPeripheral (A): %@, error: %@", peripheral, error);
    _adapter->delegate_did_disconnect_peripheral((__bridge void*)peripheral, (__bridge void*)error);
}

- (void)centralManager:(CBCentralManager*)central didFailToConnectPeripheral:(CBPeripheral*)peripheral error:(NSError*)error {
    _adapter->delegate_did_fail_to_connect_peripheral((__bridge void*)peripheral, (__bridge void*)error);
}

- (void)centralManager:(CBCentralManager*)central didDisconnectPeripheral:(CBPeripheral*)peripheral timestamp:(CFAbsoluteTime)timestamp isReconnecting:(BOOL)isReconnecting error:(NSError*)error {
    // NSLog(@"didDisconnectPeripheral (B): %@, error: %@", peripheral, error);
    _adapter->delegate_did_disconnect_peripheral((__bridge void*)peripheral, (__bridge void*)error);
}

@end
