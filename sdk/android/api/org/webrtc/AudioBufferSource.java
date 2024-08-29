package org.webrtc;

import java.nio.ByteBuffer;

/**
 * Java wrapper for a C++ AudioSourceInterface. Used as the source for one or
 * more {@code AudioTrack} objects.
 */
public class AudioBufferSource extends MediaSource {

  public AudioBufferSource(long nativeSource) {
    super(nativeSource);
  }

  public void OnAudioBuffer(ByteBuffer buffer, int sampleRate, int channels, long timestamp) {
    nativeOnCapturedBuffer(getNativeAudioSource(), buffer, sampleRate, channels, timestamp);
  }

  /** Returns a pointer to webrtc::AudioSourceInterface. */
  long getNativeAudioSource() {
    return getNativeMediaSource();
  }

  private static native void nativeOnCapturedBuffer(long nativeAudioBufferSource, ByteBuffer buffer,
      int sampleRate,
      int channels,
      long timestamp);
}
