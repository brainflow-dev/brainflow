#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "socket_bluetooth.h"
#include "bluetooth_types.h"
#include "BluetoothWorker.h"

extern "C"{
    #include <errno.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <stdlib.h>
    #include <signal.h>
    #include <termios.h>
    #include <sys/poll.h>
    #include <sys/ioctl.h>
    #include <sys/socket.h>
    #include <sys/types.h>
}

#import <Foundation/NSObject.h>
#import <IOBluetooth/objc/IOBluetoothDevice.h>
#import <IOBluetooth/objc/IOBluetoothDeviceInquiry.h>


SocketBluetooth::SocketBluetooth (std::string mac_addr, int port)
{
    this->mac_addr = mac_addr;
    this->port = port;
    consumer = NULL;
}

int SocketBluetooth::connect ()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSString *address_string = [NSString stringWithCString:mac_addr.c_str () encoding:NSASCIIStringEncoding];
    BluetoothWorker *worker = [BluetoothWorker getInstance];

    // create pipe to communicate with delegat
    pipe_t *pipe = pipe_new (sizeof (unsigned char), 0);
	int res = (int)SocketBluetoothReturnCodes::STATUS_OK;

    IOReturn result = [worker connectDevice: address_string onChannel:port withPipe:pipe];

    if (result == kIOReturnSuccess)
    {
        pipe_consumer_t *c = pipe_consumer_new (pipe);
        // save consumer side of the pipe
        consumer = c;
    }
    else
    {
        res = (int)SocketBluetoothReturnCodes::CONNECT_ERROR;
    }

    pipe_free (pipe);
    [pool release];

    return res;
}

int SocketBluetooth::close()
{
    NSString *address_string = [NSString stringWithCString:mac_addr.c_str () encoding:NSASCIIStringEncoding];
    BluetoothWorker *worker = [BluetoothWorker getInstance];
    [worker disconnectFromDevice: address_string];

    return (int)SocketBluetoothReturnCodes::STATUS_OK;
}

int SocketBluetooth::recv (char *buffer, int length)
{
    int res = (int)pipe_pop_eager (consumer, buffer, length);
    for (int i = 0; i < res; i++)
    {
        temp_buffer.push (buffer[i]);
    }
    if ((int)temp_buffer.size () < length)
    {
        return 0;
    }
    for (int i = 0; i < length; i++)
    {
        buffer[i] = temp_buffer.front ();
        temp_buffer.pop ();
    }
    return length;
}

int SocketBluetooth::send (const char *buffer, int length)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    BluetoothWorker *worker = [BluetoothWorker getInstance];
    NSString *address_string = [NSString stringWithCString:mac_addr.c_str () encoding:NSASCIIStringEncoding];

    int res = length;
    if ([worker writeAsync: const_cast<char*>(buffer) length: length toDevice: address_string] != kIOReturnSuccess)
    {
        res = -1;
    }
    [pool release];

    return res;
}

int SocketBluetooth::bytes_available ()
{
	return 0;
}

std::pair<std::string, int> SocketBluetooth::discover (char *selector)
{
    // somehow it leads to connection reset and audio stops, todo - fix, dont use autodiscovery on macos
    NSArray *device_array = [IOBluetoothDevice pairedDevices];
    if ((device_array == nil) || ([device_array count] == 0))
    {
        return std::make_pair<std::string, int> (
            "", (int)SocketBluetoothReturnCodes::DEVICE_IS_NOT_CREATED_ERROR);
    }
    for (int i = 0; i < (int)device_array.count; i++)
    {
        IOBluetoothDevice *device = [device_array objectAtIndex:i];
        std::string device_name = std::string ([device.name UTF8String], [device.name lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
        if (device_name.find (std::string (selector)) != device_name.npos)
        {
            std::string address_string = std::string ([device.addressString UTF8String], [device.addressString lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
            std::pair<std::string, int> res = std::pair<std::string, int> (
                address_string, (int)SocketBluetoothReturnCodes::STATUS_OK);
            return res;
        }
    }

    return std::make_pair<std::string, int> (
        "", (int)SocketBluetoothReturnCodes::DEVICE_IS_NOT_CREATED_ERROR);
}
