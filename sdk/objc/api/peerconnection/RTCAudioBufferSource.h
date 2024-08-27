#import <Foundation/Foundation.h>

#import "RTCMacros.h"
#import "RTCMediaSource.h"
#import "RTCAudioCapturer.h"

NS_ASSUME_NONNULL_BEGIN

RTC_OBJC_EXPORT
@interface RTC_OBJC_TYPE (RTCAudioBufferSource) : RTC_OBJC_TYPE(RTCMediaSource) <RTC_OBJC_TYPE(RTCAudioCapturerDelegate)>

- (instancetype)init NS_UNAVAILABLE;

// Sets the volume for the RTCMediaSource. `volume` is a gain value in the range
// [0, 10].
// Temporary fix to be able to modify volume of remote audio tracks.
// TODO(kthelgason): Property stays here temporarily until a proper volume-api
// is available on the surface exposed by webrtc.
@property(nonatomic, assign) double volume;

@end

NS_ASSUME_NONNULL_END