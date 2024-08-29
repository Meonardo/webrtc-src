#include "sdk/android/src/jni/pc/audio_buffer_source.h"

#include "api/array_view.h"
#include "rtc_base/byte_order.h"
#include "rtc_base/logging.h"
#include "sdk/android/generated_peerconnection_jni/AudioBufferSource_jni.h"

namespace webrtc {
namespace jni {

AudioBufferSource::AudioBufferSource()
    : worker_thread_(rtc::Thread::Create()),
      sample_rate_(0),
      number_of_channels_(0),
      task_queue_factory_(CreateDefaultTaskQueueFactory()),
      fine_audio_buffer_(nullptr),
      audio_device_buffer_(nullptr) {
  RTC_LOG(LS_INFO) << "AudioBufferSource::AudioBufferSource";
  worker_thread_->SetName("AudioBufferSource", worker_thread_.get());
  worker_thread_->Start();

  worker_thread_->BlockingCall([this] {
    audio_device_buffer_.reset(
        new webrtc::AudioDeviceBuffer(task_queue_factory_.get()));
  });
}

AudioBufferSource::~AudioBufferSource() {
  RTC_LOG(LS_INFO) << "AudioBufferSource::~AudioBufferSource";
  if (worker_thread_) {
    worker_thread_->BlockingCall([this] { audio_device_buffer_.reset(); });
    worker_thread_->Stop();
  }
  RTC_LOG(LS_INFO) << "AudioBufferSource::~AudioBufferSource done";
}

void AudioBufferSource::AddSink(AudioTrackSinkInterface* sink) {
  RTC_LOG(LS_INFO) << "AudioBufferSource::AddSink";

  std::lock_guard<std::mutex> lock(sinks_mutex_);
  sinks_.push_back(sink);
}

void AudioBufferSource::RemoveSink(AudioTrackSinkInterface* sink) {
  RTC_LOG(LS_INFO) << "AudioBufferSource::RemoveSink";

  std::lock_guard<std::mutex> lock(sinks_mutex_);
  sinks_.remove(sink);
}

int32_t AudioBufferSource::RecordedDataIsAvailable(const void* audioSamples,
                                                   size_t nSamples,
                                                   size_t nBytesPerSample,
                                                   size_t nChannels,
                                                   uint32_t samplesPerSec,
                                                   uint32_t totalDelayMS,
                                                   int32_t clockDrift,
                                                   uint32_t currentMicLevel,
                                                   bool keyPressed,
                                                   uint32_t& newMicLevel) {
  std::lock_guard<std::mutex> lock(sinks_mutex_);

  for (auto& sink : sinks_) {
    sink->OnData(audioSamples, 16, samplesPerSec, nChannels, nSamples);
  }

  return 0;
}

void AudioBufferSource::UpdateAudioDeviceBuffer(uint32_t sample_rate,
                                                uint32_t number_of_channels) {
  if (sample_rate_ == sample_rate &&
      number_of_channels_ == number_of_channels) {
    return;
  }
  sample_rate_ = sample_rate;
  number_of_channels_ = number_of_channels;

  if (worker_thread_->IsCurrent()) {
    UpdateAudioDeviceBufferOnWorkerThread();
  } else {
    worker_thread_->BlockingCall(
        [this] { UpdateAudioDeviceBufferOnWorkerThread(); });
  }
}

void AudioBufferSource::UpdateAudioDeviceBufferOnWorkerThread() {
  audio_device_buffer_->SetRecordingSampleRate(sample_rate_);
  audio_device_buffer_->SetRecordingChannels(number_of_channels_);
  fine_audio_buffer_.reset(new FineAudioBuffer(audio_device_buffer_.get()));
  audio_device_buffer_->RegisterAudioCallback(this);
}

void AudioBufferSource::OnCapturedBuffer(JNIEnv* env,
                                         const JavaRef<jobject>& j_buffer,
                                         jint j_sample_rate,
                                         jint j_channels,
                                         jlong j_timestamp) {
  RTC_LOG(LS_INFO) << "AudioBufferSource::OnCapturedBuffer";

  // Get the buffer from ByteBuffer
  void* buffer = env->GetDirectBufferAddress(j_buffer.obj());
  if (!buffer) {
    RTC_LOG(LS_ERROR) << "AudioBufferSource::OnCapturedBuffer: "
                      << "Failed to get direct buffer address";
    return;
  }
  auto size = env->GetDirectBufferCapacity(j_buffer.obj());
  if (size == 0) {
    RTC_LOG(LS_ERROR) << "AudioBufferSource::OnCapturedBuffer: "
                      << "Buffer size is 0";
    return;
  }

  // Update the audio device buffer
  UpdateAudioDeviceBuffer(j_sample_rate, j_channels);

  // deliver to fine audio buffer to make 10ms chunks
  rtc::ArrayView<int16_t> audio_data(static_cast<int16_t*>(buffer),
                                     size / sizeof(int16_t));
  fine_audio_buffer_->DeliverRecordedData(audio_data, 30);
}

}  // namespace jni
}  // namespace webrtc