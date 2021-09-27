#pragma once

#import <CoreBluetooth/CoreBluetooth.h>
#import <Foundation/Foundation.h>

#include <functional>
#include "AdapterBase.h"

@interface AdapterBaseMacOS : NSObject<CBCentralManagerDelegate>

- (instancetype)init:(SimpleBLE::AdapterBase*)adapter;

- (void)scanStart;

- (void)scanStop;

- (bool)scanIsActive;

@end
