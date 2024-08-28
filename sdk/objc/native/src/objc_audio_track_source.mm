#include "sdk/objc/native/src/objc_audio_track_source.h"

#include "api/array_view.h"
#include "rtc_base/byte_order.h"
#include "rtc_base/logging.h"

#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>

namespace webrtc {

ObjcAudioTrackSource::ObjcAudioTrackSource()
    : worker_thread_(rtc::Thread::Create()),
      sample_rate_(0),
      number_of_channels_(0),
      task_queue_factory_(CreateDefaultTaskQueueFactory()),
      fine_audio_buffer_(nullptr),
      audio_device_buffer_(nullptr) {
  RTC_LOG(LS_INFO) << "ObjcAudioTrackSource::ObjcAudioTrackSource";
  worker_thread_->SetName("ObjcAudioTrackSource", worker_thread_.get());
  worker_thread_->Start();

  worker_thread_->BlockingCall([this] {
    audio_device_buffer_.reset(new webrtc::AudioDeviceBuffer(task_queue_factory_.get()));
  });
}

ObjcAudioTrackSource::~ObjcAudioTrackSource() {
  RTC_LOG(LS_INFO) << "ObjcAudioTrackSource::~ObjcAudioTrackSource";
  if (worker_thread_) {
    worker_thread_->BlockingCall([this] {
      audio_device_buffer_.reset();
    });
    worker_thread_->Stop();
  }
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

void ObjcAudioTrackSource::OnCapturedSampleBuffer(CMSampleBufferRef sampleBuffer) {
  CMFormatDescriptionRef description = CMSampleBufferGetFormatDescription(sampleBuffer);
  if (!description) {
    RTC_LOG(LS_ERROR) << "ObjcAudioTrackSource::OnCapturedSampleBuffer: No format description";
    return;
  }

  const AudioStreamBasicDescription *asbd =
      CMAudioFormatDescriptionGetStreamBasicDescription(description);
  if (!asbd) {
    RTC_LOG(LS_ERROR)
        << "ObjcAudioTrackSource::OnCapturedSampleBuffer: No stream basic description";
    return;
  }

  CMBlockBufferRef blockBuffer = CMSampleBufferGetDataBuffer(sampleBuffer);
  if (!blockBuffer) {
    RTC_LOG(LS_ERROR) << "ObjcAudioTrackSource::OnCapturedSampleBuffer: No block buffer";
    return;
  }

  AudioBufferList audioBufferList;
  size_t requiredSize = 0;
  auto ret = CMSampleBufferGetAudioBufferListWithRetainedBlockBuffer(
      sampleBuffer,
      &requiredSize,
      &audioBufferList,
      sizeof(audioBufferList),
      nullptr,
      nullptr,
      kCMSampleBufferFlag_AudioBufferList_Assure16ByteAlignment,
      &blockBuffer);
  if (ret != noErr) {
    RTC_LOG(LS_ERROR) << "ObjcAudioTrackSource::OnCapturedSampleBuffer: "
                         "CMSampleBufferGetAudioBufferListWithRetainedBlockBuffer failed: "
                      << ret;
    return;
  }

  // Update the audio device buffer
  UpdateAudioDeviceBuffer(asbd->mSampleRate, asbd->mChannelsPerFrame);

  rtc::ArrayView<int16_t> audioData(reinterpret_cast<int16_t *>(audioBufferList.mBuffers[0].mData),
                                    audioBufferList.mBuffers[0].mDataByteSize / sizeof(int16_t));

  if (asbd->mFormatFlags & kAudioFormatFlagIsBigEndian) {
    for (auto &sample : audioData) {
      sample = be16toh(sample);
    }
  }

  fine_audio_buffer_->DeliverRecordedData(audioData, 30);

  // Release the block buffer
  CFRelease(blockBuffer);
}

void ObjcAudioTrackSource::UpdateAudioDeviceBuffer(uint32_t sample_rate,
                                                   uint32_t number_of_channels) {
  if (sample_rate_ == sample_rate && number_of_channels_ == number_of_channels) {
    return;
  }
  sample_rate_ = sample_rate;
  number_of_channels_ = number_of_channels;

  if (worker_thread_->IsCurrent()) {
    UpdateAudioDeviceBufferOnWorkerThread();
  } else {
    worker_thread_->BlockingCall([this] {
      UpdateAudioDeviceBufferOnWorkerThread();
    });
  }
}

void ObjcAudioTrackSource::UpdateAudioDeviceBufferOnWorkerThread() {
  audio_device_buffer_->SetRecordingSampleRate(sample_rate_);
  audio_device_buffer_->SetRecordingChannels(number_of_channels_);
  fine_audio_buffer_.reset(new FineAudioBuffer(audio_device_buffer_.get()));
  audio_device_buffer_->RegisterAudioCallback(this);
}

int32_t ObjcAudioTrackSource::RecordedDataIsAvailable(const void *audioSamples,
                                                      size_t nSamples,
                                                      size_t nBytesPerSample,
                                                      size_t nChannels,
                                                      uint32_t samplesPerSec,
                                                      uint32_t totalDelayMS,
                                                      int32_t clockDrift,
                                                      uint32_t currentMicLevel,
                                                      bool keyPressed,
                                                      uint32_t &newMicLevel) {
  std::lock_guard<std::mutex> lock(sinks_mutex_);

  for (auto &sink : sinks_) {
    sink->OnData(audioSamples, 16, samplesPerSec, nChannels, nSamples);
  }

  return 0;
}

}  // namespace webrtc