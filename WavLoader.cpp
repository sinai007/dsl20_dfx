#include "WavLoader.h"

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#define INT24_MAX 8388607

void init_SD() {
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);

  if (!(SD.begin(SDCARD_CS_PIN))) {
    Serial.println("initialization failed!");
  }

}

void read_from_file(File file, unsigned char * buffer, int length) {
  int32_t i = 0;
  while (file.available() && i < length) {
     buffer[i] = file.read();
     i++;
  }
}

void read_file(const char * filename, unsigned char *buffer, int max) {
  init_SD();
  File file = SD.open(filename, FILE_READ);
  Serial.printf("File %s size %ld\n", filename, file.size());

  read_from_file(file, buffer, max);
  file.close();
}


int int24(unsigned char *value)
{
    /* Sign extend negative quantities */
    if( value[2] & 0x80 ) {
        return (0xff << 24) | (value[2] << 16)
                            | (value[1] <<  8)
                            |  value[0];
    } else {
        return (value[2] << 16)
              | (value[1] <<  8)
              |  value[0];
    }
}

/** Copies raw float wav content into the samples */
void WavLoader::raw(const char * filename, float32_t * samples, int numSamples) {
  read_file(filename, (unsigned char *)samples, numSamples*4);
}

void read_pcm(File & file, WavHeader &header, short channel, float32_t * samples, int numSamples ) {
  short shortBuffer;
  unsigned char charBuffer[3];
  // assumg single channel 
  for(int i=0; i< numSamples; i++) {
    if(header.bits_per_sample == 16) {
      
      read_from_file(file, (unsigned char *) &shortBuffer, sizeof(shortBuffer));
      samples[i] = (shortBuffer / (float)INT16_MAX);
    } else if(header.bits_per_sample == 24) {
      read_from_file(file, charBuffer, sizeof(charBuffer));
      int q = int24(charBuffer);
      samples[i] = ( q / (float)INT24_MAX);
    } else {
      Serial.printf("Unsupported wav format\n");
    }
  }
}

void WavLoader::as_samples(const char * filename, short channel, float32_t * samples, int numSamples) {
  init_SD();
  File file = SD.open(filename, FILE_READ);
  Serial.printf("File %s size %ld\n", filename, file.size());

  WavHeader header;
  read_from_file(file, (unsigned char *) &header, sizeof(header));

  if(header.format_type == 1 ) { //PCM
    read_pcm(file, header, channel, samples, numSamples);
  } else if (header.format_type == 3)  { //float
    //read_pcm(file, header, channel, samples, numSamples)
  }
  file.close();
  print_header(header);
}
//   if(channel >= header.channels) {
//     Serial.printf("Invalid channel");
//     return;
//   }  539452

//   int offset = sizeof(header);

//   for(int i=0; i< numSamples; i++) {
//       if(header.bits_per_sample == 16) {
//         short s;
//         // if(channel == 2) {
//         //   offset += sizeof(s);
//         // }
//         memcpy(&s, &(buffer[offset]), sizeof(s));
//         offset += sizeof(s);
//         samples[i] = (s / 32767);
//       } 
//     //   else {
//     //     int s;
//     //     if(channel == 2) {
//     //       offset += sizeof(s)
//     //     }
//     //     memcpy(&s, &(buffer[offset]), sizeof(s));
//     //     offset += sizeof(s);
//     //   }
//     // } else if(header.channels == 2) {

//     // }
//   }
  
// }

void WavLoader::print_header(WavHeader & header) {
  Serial.printf("Size: %d\n", header.overall_size);
  Serial.printf("Riff: %s\n", header.riff);
  Serial.printf("Channels: %d\n", header.channels);
  Serial.printf("SampleRate: %d\n", header.sample_rate);
  Serial.printf("Bits/Sample: %d\n", header.bits_per_sample);
  Serial.printf("FormatType: %d\n",header.format_type);
  Serial.printf("DataSize: %d\n",header.data_size);
  int num_samples = header.data_size / (header.channels * (header.bits_per_sample / 8));
  Serial.printf("Num Samples: %d\n", num_samples);
}


