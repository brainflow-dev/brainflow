/*
 * Copyright (c) 2014, Radosław Eichler
 * Copyright (c) 2012-2013, Eelco Cramer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "BluetoothWorker.h"
#import "BluetoothDeviceResources.h"
#import <Foundation/NSObject.h>
#import <IOBluetooth/objc/IOBluetoothDevice.h>
#import <IOBluetooth/objc/IOBluetoothRFCOMMChannel.h>
#import <IOBluetooth/objc/IOBluetoothSDPUUID.h>
#import <IOBluetooth/objc/IOBluetoothSDPServiceRecord.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "pipe.h"

#ifndef RFCOMM_UUID
#define RFCOMM_UUID 0x0003
#endif

/** Private class for wrapping a pipe */
@interface Pipe : NSObject {
    pipe_t *pipe;
}
@property (nonatomic, assign) pipe_t *pipe;
@end

/** Implementation of the pipe class */
@implementation Pipe
@synthesize pipe;
@end

/** Private class for wrapping data */
@interface BTData : NSObject {
    NSData *data;
    NSString *address;
}
@property (nonatomic, assign) NSData *data;
@property (nonatomic, assign) NSString *address;
@end

/** Implementation of bt data class */
@implementation BTData
@synthesize data;
@synthesize address;
@end

/** Class that is handling all the Bluetooth work */
@implementation BluetoothWorker

/** The BluetoothWorker class is a singleton. An instance can be obtained using this method */
+ (id)getInstance
{
    static BluetoothWorker *instance = nil;
    static dispatch_once_t onceToken;

    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
    });

    return instance;
}

/** Initializes a BluetoothWorker object */
- (id) init
{
    self = [super init];
    sdpLock = [[NSLock alloc] init];
    devices = [[NSMutableDictionary alloc] init];
    devicesLock = [[NSLock alloc] init];
    connectLock = [[NSLock alloc] init];
    writeLock = [[NSLock alloc] init];

    // creates a worker thread that handles all the asynchronous stuff
    worker = [[NSThread alloc]initWithTarget: self selector: @selector(startBluetoothThread:) object: nil];
    [worker start];
    return self;
}

/** Creates a run loop and sets a timer to keep the run loop alive */
- (void) startBluetoothThread: (id) arg
{
    @autoreleasepool {
        NSRunLoop *runLoop = [NSRunLoop currentRunLoop];
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnonnull"
          //schedule a timer so runMode won't stop immediately
        keepAliveTimer = [[NSTimer alloc] initWithFireDate:[NSDate distantFuture]
            interval:1 target:nil selector:nil userInfo:nil repeats:YES];
        [runLoop addTimer:keepAliveTimer forMode:NSDefaultRunLoopMode];
         [[NSRunLoop currentRunLoop] run];
#pragma clang diagnostic pop
    }
}

/** Disconnect from a Bluetooth device */
- (void) disconnectFromDevice:(NSString *)address
{
    // this function is called synchronous from javascript so it waits on the worker task to complete.
    [self performSelector:@selector(disconnectFromDeviceTask:) onThread:worker withObject: address waitUntilDone:true];
}

/** Task on the worker to disconnect from a Bluetooth device */
- (void) disconnectFromDeviceTask: (NSString *) address
{
    // make it safe
    [devicesLock lock];

    BluetoothDeviceResources *res = [devices objectForKey: address];

    if (res != nil) {
        if (res.producer != NULL) {
            pipe_producer_free(res.producer);
            res.producer = NULL;
        }

        if (res.channel != NULL) {
            [res.channel closeChannel];
            res.channel = NULL;
        }

        if (res.device != NULL) {
            [res.device closeConnection];
            res.device = NULL;
        }

        [devices removeObjectForKey: address];
    }

    [devicesLock unlock];
}

/** Connect to a Bluetooth device on a specific channel using a pipe to communicate with the main thread */
- (IOReturn)connectDevice: (NSString *) address onChannel: (int) channel withPipe: (pipe_t *)pipe
{
    [connectLock lock];

    Pipe *pipeObj = [[Pipe alloc] init];
    pipeObj.pipe = pipe;

    NSDictionary *parameters = [[NSDictionary alloc] initWithObjectsAndKeys:
        address, @"address", [NSNumber numberWithInt: channel], @"channel", pipeObj, @"pipe", nil];

    // connect to a device and wait for the result
    [self performSelector:@selector(connectDeviceTask:) onThread:worker withObject:parameters waitUntilDone:true];
    IOReturn result = connectResult;
    [connectLock unlock];

    return result;
}

/** Task to connect to a specific device */
- (void)connectDeviceTask: (NSDictionary *)parameters
{
    NSString *address = [parameters objectForKey:@"address"];
    NSNumber *channelID = [parameters objectForKey:@"channel"];
    pipe_t *pipe = ((Pipe *)[parameters objectForKey:@"pipe"]).pipe;

    connectResult = kIOReturnError;

    [devicesLock lock];

    IOBluetoothDevice *device = [IOBluetoothDevice deviceWithAddressString:address];
    if (device != nil && [devices objectForKey: [device addressString]] == nil) {

        IOBluetoothRFCOMMChannel *channel = [[IOBluetoothRFCOMMChannel alloc] init];
        if ([device openRFCOMMChannelSync: &channel withChannelID:[channelID intValue] delegate: self] == kIOReturnSuccess) {
            connectResult = kIOReturnSuccess;
               pipe_producer_t *producer = pipe_producer_new(pipe);
               BluetoothDeviceResources *res = [[BluetoothDeviceResources alloc] init];
               res.device = device;
               res.producer = producer;
            res.channel = channel;

            [devices setObject:res forKey:[device addressString]];
        }
    }

    [devicesLock unlock];
}

/** Write synchronized to a connected Bluetooth device */
- (IOReturn)writeAsync:(void *)data length:(UInt16)length toDevice: (NSString *)address
{
    [writeLock lock];

    BTData *writeData = [[BTData alloc] init];
    writeData.data = [NSData dataWithBytes: data length: length];
    writeData.address = address;

    // wait for the write to be performed on the worker thread
    [self performSelector:@selector(writeAsyncTask:) onThread:worker withObject:writeData waitUntilDone:true];

    IOReturn result = writeResult;
    [writeLock unlock];

    return result;
}

/** Task to do the writing */
- (void)writeAsyncTask:(BTData *)writeData
{
    while (![devicesLock tryLock]) {
        CFRunLoopRun();
    }

    BluetoothDeviceResources *res = [devices objectForKey:writeData.address];

    if (res != nil) {
        char *idx = (char *)[writeData.data bytes];
        ssize_t numBytesRemaining;
        BluetoothRFCOMMMTU rfcommChannelMTU;

        numBytesRemaining = [writeData.data length];
        writeResult = kIOReturnSuccess;

        // Get the RFCOMM Channel's MTU.  Each write can only
        // contain up to the MTU size number of bytes.
        rfcommChannelMTU = [res.channel getMTU];

        // Loop through the data until we have no more to send.
        while ((writeResult == kIOReturnSuccess) && (numBytesRemaining > 0)) {
            // finds how many bytes I can send:
            ssize_t numBytesToWrite = ((numBytesRemaining > rfcommChannelMTU) ? rfcommChannelMTU :  numBytesRemaining);

            // Send the bytes
            writeResult = [res.channel writeAsync:idx length:numBytesToWrite refcon:devicesLock];

            while (![devicesLock tryLock]) {
                CFRunLoopRun();
            }

            // Updates the position in the buffer:
            numBytesRemaining -= numBytesToWrite;
            idx += numBytesToWrite;
        }
    }

    [devicesLock unlock];
    CFRunLoopStop(CFRunLoopGetCurrent());
}

- (void)rfcommChannelWriteComplete:(IOBluetoothRFCOMMChannel*)rfcommChannel refcon:(void*)refcon status:(IOReturn)error
{
    [devicesLock unlock];
    CFRunLoopStop(CFRunLoopGetCurrent());
}

/** Get the RFCOMM channel for a given device */
- (int) getRFCOMMChannelID: (NSString *) address
{
    [sdpLock lock];
    // call the task on the worker thread and wait for the result
    [self performSelector:@selector(getRFCOMMChannelIDTask:) onThread:worker withObject:address waitUntilDone:true];
    int returnValue = lastChannelID;
    [sdpLock unlock];
    return returnValue;
}

/** Task to get the RFCOMM channel */
- (void) getRFCOMMChannelIDTask: (NSString *) address
{
    IOBluetoothDevice *device = [IOBluetoothDevice deviceWithAddressString:address];
    IOBluetoothSDPUUID *uuid = [[IOBluetoothSDPUUID alloc] initWithUUID16:RFCOMM_UUID];
    NSArray *uuids = [NSArray arrayWithObject:uuid];

    // always perform a new SDP query
    NSDate *lastServicesUpdate = [device getLastServicesUpdate];
    NSDate *currentServiceUpdate = NULL;

    // only search for the UUIDs we are going to need...
    [device performSDPQuery: NULL uuids: uuids];

    bool stop = false;

    NSTimeInterval endTime = [[NSDate date] timeIntervalSince1970] + 60;
    // if needed wait for a while for the sdp update
    while (!stop && [[NSDate date] timeIntervalSince1970] < endTime) { // wait no more than 60 seconds for SDP update
        currentServiceUpdate = [device getLastServicesUpdate];

        if (currentServiceUpdate != NULL && [currentServiceUpdate laterDate: lastServicesUpdate]) {
            stop = true;
        } else {
            sleep(1);
        }
    }

    NSArray *services = [device services];

    // if there are services check if it is the one we are looking for.
    if (services != NULL) {
        for (NSUInteger i=0; i<[services count]; i++) {
            IOBluetoothSDPServiceRecord *sr = [services objectAtIndex: i];

            if ([sr hasServiceFromArray: uuids]) {
                BluetoothRFCOMMChannelID cid = -1;
                if ([sr getRFCOMMChannelID: &cid] == kIOReturnSuccess) {
                    lastChannelID = cid;
                    return;
                }
            }
        }
    }

    // This can happen is some conditions where the network is unreliable. Just ignore for now...
    lastChannelID = -1;
}

/** Called when data is received from a remote device */
- (void)rfcommChannelData:(IOBluetoothRFCOMMChannel*)rfcommChannel data:(void *)dataPointer length:(size_t)dataLength
{
    NSString *address = [[rfcommChannel getDevice] addressString];

    while (![devicesLock tryLock]) {
        CFRunLoopRun();
    }

    BluetoothDeviceResources *res = [devices objectForKey: address];


    if (res != NULL && res.producer != NULL) {
        // push the data into the pipe so it can be read from the main thread
        pipe_push(res.producer, dataPointer, dataLength);
    }

    [devicesLock unlock];
    CFRunLoopStop(CFRunLoopGetCurrent());
}

/** Called when a channel has been closed */
- (void)rfcommChannelClosed:(IOBluetoothRFCOMMChannel*)rfcommChannel
{
    [self disconnectFromDevice: [[rfcommChannel getDevice] addressString]];
}
@end
