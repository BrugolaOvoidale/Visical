#ifdef __APPLE__
#include "AVFoundation.hpp"
#import <AVFoundation/AVFoundation.h>


std::vector<std::string> AVFoundation::listDeviceIds()
{
    std::vector<std::string> ids;

    AVCaptureDeviceDiscoverySession* session =
        [AVCaptureDeviceDiscoverySession
        discoverySessionWithDeviceTypes : @[AVCaptureDeviceTypeBuiltInWideAngleCamera,
        AVCaptureDeviceTypeExternal]
        mediaType:AVMediaTypeVideo
        position : AVCaptureDevicePositionUnspecified];

    for (AVCaptureDevice* device in session.devices)
    {
        ids.push_back(std::string([device.uniqueID UTF8String]));
    }

    return ids;
}
#endif