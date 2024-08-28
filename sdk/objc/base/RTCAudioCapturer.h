#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

#import "RTCMacros.h"

NS_ASSUME_NONNULL_BEGIN

@class RTC_OBJC_TYPE(RTCAudioCapturer);

RTC_OBJC_EXPORT
@protocol RTC_OBJC_TYPE(RTCAudioCapturerDelegate)<NSObject> 
- (void)capturer : (RTC_OBJC_TYPE(RTCAudioCapturer) *)capturer didCaptureAudioFrame
    : (void *)frame withSampleRate : (NSUInteger)sampleRate bitsPerSample
    : (NSUInteger)bitsPerSample numberOfChannels : (NSUInteger)numberOfChannels numberOfFrames
    : (NSUInteger)numberOfFrames;
    
@optional    
- (void)capturer: (RTC_OBJC_TYPE(RTCAudioCapturer) *)capturer didCaptureAudioSampleBuffer
    : (CMSampleBufferRef)sampleBuffer;
@end

RTC_OBJC_EXPORT
@interface RTC_OBJC_TYPE (RTCAudioCapturer) : NSObject

@property(nonatomic, weak) id<RTC_OBJC_TYPE(RTCAudioCapturerDelegate)> delegate;

- (instancetype)initWithDelegate:(id<RTC_OBJC_TYPE(RTCAudioCapturerDelegate)>)delegate;

@end

NS_ASSUME_NONNULL_END