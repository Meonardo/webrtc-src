#import "RTCAudioBufferSource+Private.h"

#include "rtc_base/checks.h"
#include "rtc_base/ref_counted_object.h"

#include "sdk/objc/native/src/objc_audio_track_source.h"

static webrtc::ObjcAudioTrackSource *getObjCAudioSource(
    const rtc::scoped_refptr<webrtc::AudioSourceInterface> nativeSource) {
  auto objc_track_source = static_cast<webrtc::ObjcAudioTrackSource *>(nativeSource.get());
  return objc_track_source;
}

@implementation RTC_OBJC_TYPE (RTCAudioBufferSource) {
}

@synthesize volume = _volume;
@synthesize nativeAudioSource = _nativeAudioSource;

- (instancetype)initWithFactory:(RTC_OBJC_TYPE(RTCPeerConnectionFactory) *)factory {
  RTC_DCHECK(factory);
  rtc::scoped_refptr<webrtc::ObjcAudioTrackSource> objCAudioTrackSource(
      new rtc::FinalRefCountedObject<webrtc::ObjcAudioTrackSource>());

  if (self = [super initWithFactory:factory
                  nativeMediaSource:objCAudioTrackSource
                               type:RTCMediaSourceTypeAudio]) {
    _nativeAudioSource = objCAudioTrackSource;
  }
  return self;
}

- (NSString *)description {
  NSString *stateString = [[self class] stringForState:self.state];
  return [NSString
      stringWithFormat:@"RTC_OBJC_TYPE(RTCAudioBufferSource)( %p ): %@", self, stateString];
}

- (void)setVolume:(double)volume {
  _volume = volume;
  _nativeAudioSource->SetVolume(volume);
}

- (void)capturer:(RTC_OBJC_TYPE(RTCAudioCapturer) *)capturer
    didCaptureAudioFrame:(void *)frame
          withSampleRate:(NSUInteger)sampleRate
           bitsPerSample:(NSUInteger)bitsPerSample
        numberOfChannels:(NSUInteger)numberOfChannels
          numberOfFrames:(NSUInteger)numberOfFrames {
  getObjCAudioSource(_nativeAudioSource)
      ->OnCapturedFrame(frame, sampleRate, bitsPerSample, numberOfChannels, numberOfFrames);
}

#pragma mark - Private

- (rtc::scoped_refptr<webrtc::AudioSourceInterface>)nativeAudioSource {
  return _nativeAudioSource;
}

@end