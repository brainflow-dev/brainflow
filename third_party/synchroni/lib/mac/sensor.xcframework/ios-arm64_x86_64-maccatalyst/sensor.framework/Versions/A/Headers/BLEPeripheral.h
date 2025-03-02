//
//  BLEPeripheral.h
//  TheOne
//
//  Created by 王涛 on 2017/6/13.
//  Copyright © 2017年 tcl. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>


@interface BLEPeripheral : NSObject

@property (nonatomic, strong)   CBPeripheral *cbPeripheral;
//deviceName
@property (nonatomic, copy)     NSString *name;

@property (nonatomic, copy)     NSString *macAddress;

@property (nonatomic, strong)   NSNumber *rssi;

+(NSString*)parseMacAddress:(NSDictionary*)advData;

- (instancetype)initWithPeripheral:(CBPeripheral *)peripheral;


/**
 * 订阅外设服务，注：未订阅的服务不提供读写操作
 *
 * @param serviceUUID Serice UUID
 * @param charactUUID Characteristic UUID
 */
- (void)addService:(NSString *)serviceUUID characteristic:(NSString *)charactUUID;


/**
 * 注册生成已订阅的特征实例数据集
 * 用于特征读写
 * @param service 服务
 * @param characteristic 特征
 */
- (void)registerForService:(CBService *)service characteristic:(CBCharacteristic *)characteristic;

/**
 * 已订阅的所有服务UUID
 *
 * @return UUID 数组
 */
- (NSArray *)services;

/**
 * 已订阅服务的特征信息
 *
 * @param uuidString service uuid string
 * @return characteristic uuid string array
 */
- (NSArray *)characteristicForService:(NSString *)uuidString;

/**
 * 根据规则生成唯一UUID(不同服务存在相同特征)
 *
 @param serviceUUID 服务UUID
 @param charactUUID 特征UUID
 @return 唯一标识符
 */
+ (NSString *)generateIdentifierWithService:(NSString *)serviceUUID characteristic:(NSString *)charactUUID;

/**
 * 判断订阅的服务特征是否启用通知功能
 *
 * @param service  服务
 * @param characteristic 特征
 * @return 通知启用状态
 */
- (BOOL)isNotifyForService:(CBService *)service characteristic:(CBCharacteristic *)characteristic;

/**
 Description

 @param data 传输数据
 @param identifier Serive & Charactersitic 合成的UUID
*/
//- (void)writeValue:(NSData *)data forIdentifier:(NSString *)identifier;
- (void)writeValue:(NSData *)data forIdentifier:(NSString *)identifier;

- (void)writeDataWithResponseValue:(NSData *)data forIdentifier:(NSString *)identifier;
/**
 Description

 @param indetifier Serive & Charactersitic 合成的UUID
 */
- (void)readValueForIndetifier:(NSString *)indetifier;
/**
 Description

 @param dict 转换macString rssi 设备距离
 */
- (BOOL)transformMacStringWithAdvertisementData:(NSDictionary *)dict withRssi:(NSNumber *)rssi;

- (void)setNotifyForIdetifier:(NSString*)indetifier enable:(BOOL)enable;
@end
