#include "wav.h"
#include <stdlib.h>

int main() {
  uint32_t sampleRate = 44100;
  float amplitude = 0.5f;

  // Map "HELLO" to frequencies
  double freqs[] = {440, 494, 523, 523, 587};
  size_t numLetters = sizeof(freqs) / sizeof(freqs[0]);
  double duration = 0.3; // seconds per letter

  size_t samplesPerLetter = (size_t)(sampleRate * duration);
  size_t totalSamples = samplesPerLetter * numLetters;
  double *buffer = malloc(sizeof(double) * totalSamples);

  for (size_t i = 0; i < numLetters; i++) {
    wav_freq2raw_sine(buffer + i * samplesPerLetter, samplesPerLetter, freqs[i],
                      amplitude, sampleRate);
  }

  struct wav_header header = wav_int(WAV_MONO, sampleRate, 16, totalSamples);
  wav_write(&header, "hello.wav", buffer, totalSamples, 1.0f);

  free(buffer);
  return 0;
}
