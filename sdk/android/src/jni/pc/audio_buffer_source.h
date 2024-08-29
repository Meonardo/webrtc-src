#ifndef SDK_ANDROID_SRC_JNI_AUDIO_BUFFER_SOURCE_H_
#define SDK_ANDROID_SRC_JNI_AUDIO_BUFFER_SOURCE_H_

#include <jni.h>

#include <list>
#include <memory>
#include <mutex>

#include "api/task_queue/default_task_queue_factory.h"
#include "modules/audio_device/audio_device_buffer.h"
#include "modules/audio_device/fine_audio_buffer.h"
#include "pc/local_audio_source.h"
#include "rtc_base/thread.h"
#include "sdk/android/src/jni/jni_helpers.h"

namespace webrtc {
namespace jni {

class AudioBufferSource : public webrtc::Notifier<webrtc::AudioSourceInterface>,
                          public webrtc::AudioTransport {
 public:
  explicit AudioBufferSource();
  ~AudioBufferSource() override;

  virtual void AddSink(webrtc::AudioTrackSinkInterface* sink) override;
  virtual void RemoveSink(webrtc::AudioTrackSinkInterface* sink) override;
  virtual SourceState state() const override { return kLive; }
  virtual bool remote() const override { return false; }

  void OnCapturedBuffer(JNIEnv* env,
                        const JavaRef<jobject>& j_buffer,
                        jint j_sample_rate,
                        jint j_channels,
                        jlong j_timestamp);

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
                           int64_t* ntp_time_ms) override {
    return 0;
  }

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

}  // namespace jni
}  // namespace webrtc

#endif  // SDK_ANDROID_SRC_JNI_AUDIO_BUFFER_SOURCE_H_