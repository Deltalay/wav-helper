#ifndef WAV_HELPER_H
#define WAV_HELPER_H
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#define WAV_FAIL_TO_OPEN_FILE 1
#define WAV_WRITE_SUCCESS 0
#define WAV_STEREO 2
#define WAV_MONO 1
// We will handle data seperately
// We will try to handle as much as we can
// instead of letting user decide.
struct wav_header {
  uint32_t chunkSize;
  uint32_t subChunk1Size;
  uint16_t audioFormat;
  uint16_t numChannels;
  uint32_t sampleRate;
  uint16_t bitsPerSample;
  uint32_t byteRate;
  uint16_t blockAlign;
  uint32_t subChunk2Size;
};
enum wave_function { SINE, COSINE };
// U32 little endian if we have 0x12345678 we have to write 78 56 34 12
// first we & with 0xff which give us 0x78
// second we shift it by 1 byte which is 0x123456 then do the same which give us
// 0x56
static void write_u32_le(FILE *f, uint32_t val) {
  fputc(val & 0xFF, f);
  fputc((val >> 8) & 0xFF, f);
  fputc((val >> 16) & 0xFF, f);
  fputc((val >> 24) & 0xFF, f);
}

static void write_u16_le(FILE *f, uint16_t val) {
  fputc(val & 0xFF, f);
  fputc((val >> 8) & 0xFF, f);
}

static inline struct wav_header wav_int(uint16_t numChannels,
                                        uint32_t sampleRate,
                                        uint16_t bitsPerSample,
                                        uint32_t numSamples) {
  struct wav_header header;
  if (bitsPerSample != 8 && bitsPerSample != 16 && bitsPerSample != 24 &&
      bitsPerSample != 32) {
    fprintf(stderr, "Bits Per sample must be either 8, 16, 24, or 32\n");
    struct wav_header invalid = {0};
    return invalid;
  }
  uint16_t bytesPerSample = bitsPerSample / 8;
  header.subChunk1Size = 16;
  header.audioFormat = 1;
  header.numChannels = numChannels;
  header.sampleRate = sampleRate;
  header.bitsPerSample = bitsPerSample;
  header.byteRate = sampleRate * numChannels * bytesPerSample;
  header.blockAlign = numChannels * bytesPerSample;
  header.subChunk2Size = numSamples * numChannels * bytesPerSample;
  header.chunkSize = header.subChunk2Size + 36;
  return header;
}
static inline void wav_freq2raw(double *buffer, size_t numSamples, double freq,
                                float amplitude, uint32_t sampleRate,
                                enum wave_function wave) {
  double phase = 0.0;
  double phase_inc = 2.0 * M_PI * freq / sampleRate;

  for (size_t i = 0; i < numSamples; i++) {
    switch (wave) {
    case SINE:
      buffer[i] = amplitude * sin(phase);
      break;
    case COSINE:
      buffer[i] = amplitude * cos(phase);
      break;
    }
    phase += phase_inc;
    if (phase >= 2.0 * M_PI)
      phase -= 2.0 * M_PI;
  }
}
static inline int wav_write(const struct wav_header *header,
                            const char *filename, const double *data,
                            size_t data_size, float amplitude) {
  FILE *f = fopen(filename, "wb");
  if (!f) {
    return WAV_FAIL_TO_OPEN_FILE;
  }
  fwrite("RIFF", 1, 4, f);
  write_u32_le(f, header->chunkSize);
  fwrite("WAVE", 1, 4, f);
  fwrite("fmt ", 1, 4, f);
  write_u32_le(f, header->subChunk1Size);
  write_u16_le(f, header->audioFormat);
  write_u16_le(f, header->numChannels);
  write_u32_le(f, header->sampleRate);
  write_u32_le(f, header->byteRate);
  write_u16_le(f, header->blockAlign);
  write_u16_le(f, header->bitsPerSample);
  fwrite("data", 1, 4, f);
  write_u32_le(f, header->subChunk2Size);
  if (amplitude < 0.0f)
    amplitude = 0.0f;
  if (amplitude > 1.0f)
    amplitude = 1.0f;

  for (size_t i = 0; i < data_size; i++) {
    double sample = amplitude * data[i];
    int16_t pcm = (int16_t)(sample * 32767.0);
    for (uint16_t ch = 0; ch < header->numChannels; ch++)
      write_u16_le(f, pcm);
  }
  fclose(f);
  return WAV_WRITE_SUCCESS;
}
#endif
