
#ifndef DSP_H
#define DSP_H

#include <stdint.h>
#include <stdbool.h>

typedef struct Dsp Dsp;

#include "apu.h"

typedef struct DspChannel {
  // pitch
  uint16_t pitch;
  uint16_t pitchCounter;
  bool pitchModulation;
  // brr decoding
  int16_t decodeBuffer[19]; // 16 samples per brr-block, +3 for interpolation
  uint8_t srcn;
  uint16_t decodeOffset;
  uint8_t previousFlags; // from last sample
  int16_t old;
  int16_t older;
  bool useNoise;
  // adsr, envelope, gain
  uint16_t adsrRates[4]; // attack, decay, sustain, gain
  uint16_t rateCounter;
  uint8_t adsrState; // 0: attack, 1: decay, 2: sustain, 3: gain, 4: release
  uint16_t sustainLevel;
  bool useGain;
  uint8_t gainMode;
  bool directGain;
  uint16_t gainValue; // for direct gain
  uint16_t gain;
  // keyon/off
  bool keyOn;
  bool keyOff;
  // output
  int16_t sampleOut; // final sample, to be multiplied by channel volume
  int8_t volumeL;
  int8_t volumeR;
  bool echoEnable;
} DspChannel;

struct Dsp {
  Apu* apu;
  // mirror ram
  uint8_t ram[0x80];
  // 8 channels
  DspChannel channel[8];
  // overarching
  uint16_t dirPage;
  bool evenCycle;
  bool mute;
  bool reset;
  int8_t masterVolumeL;
  int8_t masterVolumeR;
  // noise
  int16_t noiseSample;
  uint16_t noiseRate;
  uint16_t noiseCounter;
  // echo
  bool echoWrites;
  int8_t echoVolumeL;
  int8_t echoVolumeR;
  int8_t feedbackVolume;
  uint16_t echoBufferAdr;
  uint16_t echoDelay;
  uint16_t echoRemain;
  uint16_t echoBufferIndex;
  uint8_t firBufferIndex;
  int8_t firValues[8];
  int16_t firBufferL[8];
  int16_t firBufferR[8];
  // sample ring buffer (1024 samples, *2 for stereo)
  int16_t sampleBuffer[0x400 * 2];
  uint16_t sampleOffset; // current offset in samplebuffer
};

Dsp* dsp_init(Apu* apu);
void dsp_free(Dsp* dsp);
void dsp_reset(Dsp* dsp);
void dsp_cycle(Dsp* dsp);
uint8_t dsp_read(Dsp* dsp, uint8_t adr);
void dsp_write(Dsp* dsp, uint8_t adr, uint8_t val);
void dsp_getSamples(Dsp* dsp, int16_t* sampleData, int samplesPerFrame);

#endif
