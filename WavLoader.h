#ifndef wavloader_h_
#define wavloader_h_

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7   // Teensy 4 ignores this, uses pin 11
#define SDCARD_SCK_PIN   14  // Teensy 4 ignores this, uses pin 13

#include <Arduino.h>
#include <Audio.h>

typedef struct __attribute__((packed, aligned(4))) {
    unsigned char riff[4];                      // RIFF string
    int overall_size   ;               // overall size of file in bytes
    unsigned char wave[4];                      // WAVE string
    unsigned char fmt_chunk_marker[4];          // fmt string with trailing null char
    int length_of_fmt;                 // length of the format data
    short format_type;                   // format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
    short channels;                      // no.of channels
    int sample_rate;                   // sampling rate (blocks per second)
    int byterate;                      // SampleRate * NumChannels * BitsPerSample/8
    short block_size;                   // NumChannels * BitsPerSample/8
    short bits_per_sample;               // bits per sample, 8- 8bits, 16- 16 bits etc
    unsigned char data_chunk_header [4];        // DATA string or FLLR string
    int data_size;                     // NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
} WavHeader;


// typedef struct __attribute__((packed, aligned(4))) {
//   short sample;
// } Mono16;

// typedef struct __attribute__((packed, aligned(4))) {
//   int sample;
// } Mono24;

// typedef struct __attribute__((packed, aligned(4))) {
//   Mono24 left;
//   Mono24 right;
// } Stereo24;

// typedef struct __attribute__((packed, aligned(4))) {
//   Mono16 left;
//   Mono16 right;
// } Stereo16;

class WavLoader {
  public:
    WavLoader() {};
    void raw(const char * filename, float32_t * samples, int numSamples);
    void as_samples(const char * filename, short channel, float32_t * samples, int numSamples);
    void print_header(WavHeader & header);
};

#endif
