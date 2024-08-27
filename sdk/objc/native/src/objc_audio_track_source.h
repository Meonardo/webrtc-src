#ifndef SDK_OBJC_CLASSES_OBJC_AUDIO_TRACK_SOURCE_H_
#define SDK_OBJC_CLASSES_OBJC_AUDIO_TRACK_SOURCE_H_

#import "base/RTCAudioCapturer.h"

#include "base/RTCMacros.h"
#include "pc/local_audio_source.h"

#include <list>
#include <mutex>

namespace webrtc {

class ObjcAudioTrackSource : public webrtc::Notifier<webrtc::AudioSourceInterface> {
 public:
  explicit ObjcAudioTrackSource();
  ~ObjcAudioTrackSource() override;

  virtual void AddSink(webrtc::AudioTrackSinkInterface *sink) override;
  virtual void RemoveSink(webrtc::AudioTrackSinkInterface *sink) override;
  virtual SourceState state() const override { return kLive; }
  virtual  bool remote() const override { return true; }

  void OnCapturedFrame(const void *audio_data,
              int bits_per_sample,
              int sample_rate,
              size_t number_of_channels,
              size_t number_of_frames);

 private:
  std::list<webrtc::AudioTrackSinkInterface *> sinks_;
  std::mutex sinks_mutex_;
};

}  // namespace webrtc

#endif  // SDK_OBJC_CLASSES_OBJC_AUDIO_TRACK_SOURCE_H_