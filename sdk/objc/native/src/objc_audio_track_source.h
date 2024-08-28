#ifndef SDK_OBJC_CLASSES_OBJC_AUDIO_TRACK_SOURCE_H_
#define SDK_OBJC_CLASSES_OBJC_AUDIO_TRACK_SOURCE_H_

#include <list>
#include <memory>
#include <mutex>

#include "api/task_queue/default_task_queue_factory.h"
#import "base/RTCAudioCapturer.h"
#include "base/RTCMacros.h"
#include "modules/audio_device/audio_device_buffer.h"
#include "modules/audio_device/fine_audio_buffer.h"
#include "rtc_base/thread.h"
#include "pc/local_audio_source.h"

namespace webrtc {

class ObjcAudioTrackSource
    : public webrtc::Notifier<webrtc::AudioSourceInterface>,
      public webrtc::AudioTransport {
 public:
  explicit ObjcAudioTrackSource();
  ~ObjcAudioTrackSource() override;

  virtual void AddSink(webrtc::AudioTrackSinkInterface* sink) override;
  virtual void RemoveSink(webrtc::AudioTrackSinkInterface* sink) override;
  virtual SourceState state() const override { return kLive; }
  virtual bool remote() const override { return true; }

  void OnCapturedFrame(const void* audio_data,
                       int bits_per_sample,
                       int sample_rate,
                       size_t number_of_channels,
                       size_t number_of_frames);
  void OnCapturedSampleBuffer(CMSampleBufferRef sampleBuffer);

  // AudioTransport overrides
  int32_t RecordedDataIsAvailable(const void* audioSamples,
                                  size_t nSamples,
                                  size_t nBytesPerSample,
                                  size_t nChannels,
                                  uint32_t samplesPerSec,
                                  uint32_t totalDelayMS,
                                  int32_t clockDrift,
                                  uint32_t currentMicLevel,
                                  bool keyPressed,
                                  uint32_t& newMicLevel) override;
  int32_t NeedMorePlayData(size_t nSamples,
                           size_t nBytesPerSample,
                           size_t nChannels,
                           uint32_t samplesPerSec,
                           void* audioSamples,
                           size_t& nSamplesOut,  // NOLINT
                           int64_t* elapsed_time_ms,
                           int64_t* ntp_time_ms) override { return 0; }

  void PullRenderData(int bits_per_sample,
                      int sample_rate,
                      size_t number_of_channels,
                      size_t number_of_frames,
                      void* audio_data,
                      int64_t* elapsed_time_ms,
                      int64_t* ntp_time_ms) override {}

 private:
  std::list<webrtc::AudioTrackSinkInterface*> sinks_;
  std::mutex sinks_mutex_;

  std::unique_ptr<rtc::Thread> worker_thread_;
  uint32_t sample_rate_;
  uint32_t number_of_channels_;
  const std::unique_ptr<TaskQueueFactory> task_queue_factory_;
  std::unique_ptr<webrtc::FineAudioBuffer> fine_audio_buffer_;
  std::unique_ptr<webrtc::AudioDeviceBuffer> audio_device_buffer_;

  void UpdateAudioDeviceBuffer(uint32_t sample_rate,
                               uint32_t number_of_channels);
  void UpdateAudioDeviceBufferOnWorkerThread();
};

}  // namespace webrtc

#endif  // SDK_OBJC_CLASSES_OBJC_AUDIO_TRACK_SOURCE_H_