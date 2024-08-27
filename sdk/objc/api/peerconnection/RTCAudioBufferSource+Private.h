#import "RTCAudioBufferSource.h"

#import "RTCMediaSource+Private.h"

@interface RTC_OBJC_TYPE (RTCAudioBufferSource)()

/**
 * The AudioSourceInterface object passed to this RTCAudioSource during
 * construction.
 */
@property(nonatomic, readonly) rtc::scoped_refptr<webrtc::AudioSourceInterface> nativeAudioSource;

/** Initialize an RTCAudioBufferSource from a native AudioSourceInterface. */
- (instancetype)initWithFactory:(RTC_OBJC_TYPE(RTCPeerConnectionFactory) *)factory;

@end