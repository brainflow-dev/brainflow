//
//  sensorProfile.h
//  sensorProfile
//
//  Created by 叶常青 on 2023/12/21.
//
#ifndef sensor_profile_h
#define sensor_profile_h

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import <sensor/BLEPeripheral.h>
#import <sensor/defines.h>

@interface DeviceInfo : NSObject

@property (atomic, strong) NSString* deviceName;
@property (atomic, strong) NSString* modelName;
@property (atomic, strong) NSString* hardwareVersion;
@property (atomic, strong) NSString* firmwareVersion;
@property (atomic, assign) int MTUSize;
@property (atomic, assign) bool isMTUFine;
@property (atomic, assign) int EEGChannelCount;
@property (atomic, assign) int ECGChannelCount;
@property (atomic, assign) int BRTHChannelCount;
@property (atomic, assign) int AccChannelCount;
@property (atomic, assign) int GyroChannelCount;

@end

@interface Sample : NSObject
@property (atomic, assign) int timeStampInMs;
@property (atomic, assign) int sampleIndex;
@property (atomic, assign) int channelIndex;
@property (atomic, assign) BOOL isLost;
@property (atomic, assign) float rawData;
@property (atomic, assign) float convertData;
@property (atomic, assign) float impedance;
@property (atomic, assign) float saturation;
@end

@interface SensorData : NSObject

@property (atomic, assign) NotifyDataType dataType;
@property (atomic, assign) int lastPackageIndex;
@property (atomic, assign) int lastPackageCounter;
@property (atomic, assign) int resolutionBits;
@property (atomic, assign) int sampleRate;
@property (atomic, assign) int channelCount;
@property (atomic, assign) unsigned long long channelMask;
@property (atomic, assign) int packageSampleCount;
@property (atomic, assign) int minPackageSampleCount;
@property (atomic, assign) double K;
@property (atomic, strong) NSArray<NSArray<Sample*>*>* channelSamples;
-(id)init;
-(void)clear;
-(NSArray<SensorData*>*)flushSamples;
@end

@protocol SensorProfileDelegate;

@interface SensorProfile : NSObject
{
    
}
@property (atomic, weak) id<SensorProfileDelegate> delegate;
@property (atomic, assign, readonly) BLEState state;
@property (atomic, readonly) NSString* stateString;
@property (atomic, strong, readonly) BLEPeripheral* device;
@property (atomic, strong, readonly) DeviceInfo* deviceInfo;
@property (atomic, assign, readonly) bool hasInit;
@property (atomic, assign, readonly) bool hasStartDataNotification;


-(BOOL)connect;
-(void)disconnect;
-(BOOL)startDataNotification;
-(BOOL)stopDataNotification;

- (void)initAll:(int)packageCount timeout:(NSTimeInterval)timeout completion:(void (^)(BOOL success, NSError * err))completionHandler;
- (void)initEEG:(int)packageCount timeout:(NSTimeInterval)timeout completion:(void (^)(BOOL success, NSError * err))completionHandler;
- (void)initECG:(int)packageCount timeout:(NSTimeInterval)timeout completion:(void (^)(BOOL success, NSError * err))completionHandler;
- (void)initIMU:(int)packageCount timeout:(NSTimeInterval)timeout completion:(void (^)(BOOL success, NSError * err))completionHandler;
- (void)initBRTH:(int)packageCount timeout:(NSTimeInterval)timeout completion:(void (^)(BOOL success, NSError * err))completionHandler;
- (void)initDataTransfer:(BOOL)isGetFeature  timeout:(NSTimeInterval)timeout completion:(void (^)(int flag, NSError * err))completionHandler;
- (void)getBattery:(NSTimeInterval)timeout completion:(void (^)(int battery, NSError * err))completionHandler;
- (void)getDeviceInfo:(BOOL)onlyMTU timeout:(NSTimeInterval)timeout completion:(void (^)(DeviceInfo* deviceInfo, NSError * err))completionHandler;
- (void)startDataNotification:(NSTimeInterval)timeout completion:(void (^)(BOOL success, NSError * err))completionHandler;
- (void)stopDataNotification:(NSTimeInterval)timeout completion:(void (^)(BOOL success, NSError * err))completionHandler;

- (void)setParam:(NSTimeInterval)timeout key:(NSString*_Nonnull)key value:(NSString*_Nonnull)value completion:(void (^_Nonnull)(NSString*_Nonnull result, NSError * _Nullable err))completionHandler;

@end

@protocol SensorProfileDelegate
- (void)onSensorErrorCallback:(SensorProfile*_Nonnull)profile err:(NSError*_Nonnull)err;
- (void)onSensorStateChange:(SensorProfile*_Nonnull)profile newState:(BLEState)newState;
- (void)onSensorNotifyData:(SensorProfile*_Nonnull)profile rawData:(SensorData*_Nonnull) rawData;
@end

#endif
