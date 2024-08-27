#include "sdk/objc/native/src/objc_audio_track_source.h"

#import "rtc_base/logging.h"

namespace webrtc {

ObjcAudioTrackSource::ObjcAudioTrackSource() {
  RTC_LOG(LS_INFO) << "ObjcAudioTrackSource::ObjcAudioTrackSource";
}

ObjcAudioTrackSource::~ObjcAudioTrackSource() {
  RTC_LOG(LS_INFO) << "ObjcAudioTrackSource::~ObjcAudioTrackSource";
}

void ObjcAudioTrackSource::AddSink(AudioTrackSinkInterface *sink) {
  RTC_LOG(LS_INFO) << "ObjcAudioTrackSource::AddSink";

  std::lock_guard<std::mutex> lock(sinks_mutex_);
  sinks_.push_back(sink);
}

void ObjcAudioTrackSource::RemoveSink(AudioTrackSinkInterface *sink) {
  RTC_LOG(LS_INFO) << "ObjcAudioTrackSource::RemoveSink";

  std::lock_guard<std::mutex> lock(sinks_mutex_);
  sinks_.remove(sink);
}

void ObjcAudioTrackSource::OnCapturedFrame(const void *audio_data,
                                           int bits_per_sample,
                                           int sample_rate,
                                           size_t number_of_channels,
                                           size_t number_of_frames) {
  std::lock_guard<std::mutex> lock(sinks_mutex_);
  for (auto &sink : sinks_) {
    sink->OnData(audio_data, bits_per_sample, sample_rate, number_of_channels, number_of_frames);
  }
}

}  // namespace webrtc