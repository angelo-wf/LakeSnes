
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "zip.h"

#include "snes.h"
#include "tracing.h"

/* depends on behaviour:
casting uintX_t to/from intX_t does 'expceted' unsigned<->signed conversion
  ((int8_t) 255 == -1)
same with assignment
  int8_t a; a = 0xff; a == -1
overflow is handled as expected
  (uint8_t a = 255; a++; a == 0; uint8_t b = 0; b--; b == 255)
clipping is handled as expected
  (uint16_t a = 0x123; uint8_t b = a; b == 0x23)
giving non 0/1 value to boolean makes it 0/1
  (bool a = 2; a == 1)
giving out-of-range vaue to function parameter clips it in range
  (void test(uint8_t a) {...}; test(255 + 1); a == 0 within test)
int is at least 32 bits
shifting into sign bit makes value negative
  int a = ((int16_t) (0x1fff << 3)) >> 3; a == -1
*/

static uint8_t* readFile(char* name, size_t* length);
static bool loadRom(char* name, Snes* snes);
static bool checkExtention(const char* name, bool forZip);
static void playAudio(Snes* snes, SDL_AudioDeviceID device, int16_t* audioBuffer);
static void renderScreen(Snes* snes, SDL_Renderer* renderer, SDL_Texture* texture);
static void handleInput(Snes* snes, int keyCode, bool pressed);

int main(int argc, char** argv) {
  // set up SDL
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    printf("Failed to init SDL: %s\n", SDL_GetError());
    return 1;
  }
  SDL_Window* window = SDL_CreateWindow("ElzSnes", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 512, 480, 0);
  if(window == NULL) {
    printf("Failed to create window: %s\n", SDL_GetError());
    return 1;
  }
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if(renderer == NULL) {
    printf("Failed to create renderer: %s\n", SDL_GetError());
    return 1;
  }
  SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBX8888, SDL_TEXTUREACCESS_STREAMING, 512, 480);
  if(texture == NULL) {
    printf("Failed to create texture: %s\n", SDL_GetError());
    return 1;
  }
  SDL_AudioSpec want, have;
  SDL_AudioDeviceID device;
  SDL_memset(&want, 0, sizeof(want));
  want.freq = 44100;
  want.format = AUDIO_S16;
  want.channels = 2;
  want.samples = 2048;
  want.callback = NULL; // use queue
  device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
  if(device == 0) {
    printf("Failed to open audio device: %s\n", SDL_GetError());
    return 1;
  }
  int16_t* audioBuffer = malloc(735 * 4); // *2 for stereo, *2 for sizeof(int16)
  SDL_PauseAudioDevice(device, 0);
  // init snes, load rom
  Snes* snes = snes_init();
  bool loaded = false;
  if(argc >= 2) {
    loaded = loadRom(argv[1], snes);
  } else {
    puts("No rom loaded");
  }
  // sdl loop
  bool running = true;
  bool paused = false;
  bool runOne = false;
  bool turbo = false;
  SDL_Event event;
  uint32_t lastTick = SDL_GetTicks();
  uint32_t curTick = 0;
  uint32_t delta = 0;
  // debugging
  bool debugging = false;
  bool cpuNext = false;
  bool spcNext = false;
  while(running) {
    while(SDL_PollEvent(&event)) {
      switch(event.type) {
        case SDL_KEYDOWN: {
          switch(event.key.keysym.sym) {
            case SDLK_l: debugging = !debugging; break;
            case SDLK_r: snes_reset(snes, false); break;
            case SDLK_e: snes_reset(snes, true); break;
            case SDLK_o: runOne = true; break;
            case SDLK_p: paused = !paused; break;
            case SDLK_t: turbo = true; break;
            case SDLK_j: {
              FILE* f = fopen("dump.bin", "wb");
              fwrite(snes->ram, 0x20000, 1, f);
              fwrite(snes->ppu->vram, 0x10000, 1, f);
              fwrite(snes->ppu->cgram, 0x200, 1, f);
              fwrite(snes->ppu->oam, 0x200, 1, f);
              fwrite(snes->ppu->highOam, 0x20, 1, f);
              fwrite(snes->apu->ram, 0x10000, 1, f);
              fclose(f);
              break;
            }
          }
          handleInput(snes, event.key.keysym.sym, true);
          break;
        }
        case SDL_KEYUP: {
          switch(event.key.keysym.sym) {
            case SDLK_t: turbo = false; break;
          }
          handleInput(snes, event.key.keysym.sym, false);
          break;
        }
        case SDL_QUIT: {
          running = false;
          break;
        }
        case SDL_DROPFILE: {
          char* droppedFile = event.drop.file;
          if(loadRom(droppedFile, snes)) loaded = true;
          SDL_free(droppedFile);
          break;
        }
      }
    }
    if(loaded && (!paused || runOne)) {
      runOne = false;
      if(!debugging) {
        if(turbo) {
          snes_runFrame(snes);
        }
        snes_runFrame(snes);
      } else {
        for(int i = 0; i < 50; i++) {
          snes_debugCycle(snes, &cpuNext, &spcNext);
          if(cpuNext) {
            char line[80];
            getProcessorStateCpu(snes, line);
            puts(line);
          }
          if(spcNext) {
            char line[57];
            getProcessorStateSpc(snes, line);
            puts(line);
          }
        }
      }
      playAudio(snes, device, audioBuffer);
      renderScreen(snes, renderer, texture);
    }
    SDL_RenderPresent(renderer); // vsyncs to 60 FPS
    // if vsync isn't working, delay manually
    curTick = SDL_GetTicks();
    delta = curTick - lastTick;
    lastTick = curTick;
    if(delta < 16) {
      SDL_Delay(16 - delta);
    }
  }
  // clean snes
  snes_free(snes);
  // clean sdl
  SDL_PauseAudioDevice(device, 1);
  SDL_CloseAudioDevice(device);
  free(audioBuffer);
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

static void playAudio(Snes* snes, SDL_AudioDeviceID device, int16_t* audioBuffer) {
  snes_setSamples(snes, audioBuffer, 735);
  if(SDL_GetQueuedAudioSize(device) <= 735 * 4 * 6) {
    // don't queue audio if buffer is still filled
    SDL_QueueAudio(device, audioBuffer, 735 * 4);
  }
}

static void renderScreen(Snes* snes, SDL_Renderer* renderer, SDL_Texture* texture) {
  void* pixels = NULL;
  int pitch = 0;
  if(SDL_LockTexture(texture, NULL, &pixels, &pitch) != 0) {
    printf("Failed to lock texture: %s\n", SDL_GetError());
    return;
  }
  snes_setPixels(snes, (uint8_t*) pixels);
  SDL_UnlockTexture(texture);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
}

static void handleInput(Snes* snes, int keyCode, bool pressed) {
  switch(keyCode) {
    case SDLK_z: snes_setButtonState(snes, 1, 0, pressed); break;
    case SDLK_a: snes_setButtonState(snes, 1, 1, pressed); break;
    case SDLK_RSHIFT: snes_setButtonState(snes, 1, 2, pressed); break;
    case SDLK_RETURN: snes_setButtonState(snes, 1, 3, pressed); break;
    case SDLK_UP: snes_setButtonState(snes, 1, 4, pressed); break;
    case SDLK_DOWN: snes_setButtonState(snes, 1, 5, pressed); break;
    case SDLK_LEFT: snes_setButtonState(snes, 1, 6, pressed); break;
    case SDLK_RIGHT: snes_setButtonState(snes, 1, 7, pressed); break;
    case SDLK_x: snes_setButtonState(snes, 1, 8, pressed); break;
    case SDLK_s: snes_setButtonState(snes, 1, 9, pressed); break;
    case SDLK_d: snes_setButtonState(snes, 1, 10, pressed); break;
    case SDLK_c: snes_setButtonState(snes, 1, 11, pressed); break;
  }
}

static bool checkExtention(const char* name, bool forZip) {
  if(name == NULL) return false;
  int length = strlen(name);
  if(length < 4) return false;
  if(forZip) {
    if(strcmp(name + length - 4, ".zip") == 0) return true;
    if(strcmp(name + length - 4, ".ZIP") == 0) return true;
  } else {
    if(strcmp(name + length - 4, ".smc") == 0) return true;
    if(strcmp(name + length - 4, ".SMC") == 0) return true;
    if(strcmp(name + length - 4, ".sfc") == 0) return true;
    if(strcmp(name + length - 4, ".SFC") == 0) return true;
  }
  return false;
}

static bool loadRom(char* name, Snes* snes) {
  // zip library from https://github.com/kuba--/zip
  size_t length = 0;
  uint8_t* file = NULL;
  if(checkExtention(name, true)) {
    struct zip_t* zip = zip_open(name, 0, 'r');
    if(zip != NULL) {
      int entries = zip_total_entries(zip);
      for(int i = 0; i < entries; i++) {
        zip_entry_openbyindex(zip, i);
        const char* zipFilename = zip_entry_name(zip);
        if(checkExtention(zipFilename, false)) {
          printf("Read \"%s\" from zip\n", zipFilename);
          zip_entry_read(zip, (void**) &file, &length);
          break;
        }
        zip_entry_close(zip);
      }
      zip_close(zip);
    }
  } else {
    file = readFile(name, &length);
  }
  if(file == NULL) {
    puts("Failed to read file");
    return false;
  }
  bool result = snes_loadRom(snes, file, length);
  free(file);
  return result;
}

static uint8_t* readFile(char* name, size_t* length) {
  FILE* f = fopen(name, "rb");
  if(f == NULL) {
    return NULL;
  }
  fseek(f, 0, SEEK_END);
  int size = ftell(f);
  rewind(f);
  uint8_t* buffer = malloc(size);
  fread(buffer, size, 1, f);
  fclose(f);
  *length = size;
  return buffer;
}
