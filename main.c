
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
  ((int8_t) 255) == -1
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

static uint8_t* readFile(const char* name, size_t* length);
static bool loadRom(const char* name, Snes* snes);
static void setTitle(SDL_Window* window, const char* path);
static bool checkExtention(const char* name, bool forZip);
static void playAudio(Snes* snes, SDL_AudioDeviceID device, int16_t* audioBuffer, int wantedSamples);
static void renderScreen(Snes* snes, SDL_Renderer* renderer, SDL_Texture* texture);
static void handleInput(Snes* snes, int keyCode, bool pressed);

int main(int argc, char** argv) {
  // set up SDL
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    printf("Failed to init SDL: %s\n", SDL_GetError());
    return 1;
  }
  SDL_Window* window = SDL_CreateWindow("LakeSnes", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 512, 480, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  if(window == NULL) {
    printf("Failed to create window: %s\n", SDL_GetError());
    return 1;
  }
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if(renderer == NULL) {
    printf("Failed to create renderer: %s\n", SDL_GetError());
    return 1;
  }
  SDL_RenderSetLogicalSize(renderer, 512, 480); // Preserve aspect ratio
  SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBX8888, SDL_TEXTUREACCESS_STREAMING, 512, 480);
  if(texture == NULL) {
    printf("Failed to create texture: %s\n", SDL_GetError());
    return 1;
  }
  SDL_AudioSpec want, have;
  SDL_AudioDeviceID device;
  SDL_memset(&want, 0, sizeof(want));
  want.freq = 48000;
  want.format = AUDIO_S16;
  want.channels = 2;
  want.samples = 2048;
  want.callback = NULL; // use queue
  device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
  if(device == 0) {
    printf("Failed to open audio device: %s\n", SDL_GetError());
    return 1;
  }
  int16_t* audioBuffer = malloc(want.freq / 50 * 4); // *2 for stereo, *2 for sizeof(int16)
  SDL_PauseAudioDevice(device, 0);
  // print version
  SDL_version version;
  SDL_GetVersion(&version);
  printf("LakeSnes - Running with SDL %d.%d.%d\n", version.major, version.minor, version.patch);
  // init snes, load rom
  Snes* snes = snes_init();
  bool loaded = false;
  if(argc >= 2) {
    loaded = loadRom(argv[1], snes);
    if(loaded) setTitle(window, argv[1]);
  } else {
    puts("No rom loaded");
  }
  // sdl loop
  bool running = true;
  bool paused = false;
  bool runOne = false;
  bool turbo = false;
  SDL_Event event;
  int fullscreenFlags = 0;
  // timing
  uint64_t countFreq = SDL_GetPerformanceFrequency();
  uint64_t lastCount = SDL_GetPerformanceCounter();
  float timeAdder = 0.0;
  float wanted = 1.0 / (snes->palTiming ? 50.0 : 60.0);
  int wantedSamples = want.freq / (snes->palTiming ? 50 : 60);

  while(running) {
    while(SDL_PollEvent(&event)) {
      switch(event.type) {
        case SDL_KEYDOWN: {
          switch(event.key.keysym.sym) {
            case SDLK_r: snes_reset(snes, false); break;
            case SDLK_e: snes_reset(snes, true); break;
            case SDLK_o: runOne = true; break;
            case SDLK_p: paused = !paused; break;
            case SDLK_t: turbo = true; break;
            case SDLK_j: {
              puts("Dumping to dump.bin...");
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
            case SDLK_l: {
              // run one cpu cycle
              snes_runCpuCycle(snes);
              char line[80];
              getProcessorStateCpu(snes, line);
              puts(line);
              break;
            }
            case SDLK_k: {
              // run one spc cycle
              snes_runSpcCycle(snes);
              char line[57];
              getProcessorStateSpc(snes, line);
              puts(line);
              break;
            }
            case SDLK_RETURN: {
              if(event.key.keysym.mod & KMOD_ALT) {
                fullscreenFlags ^= SDL_WINDOW_FULLSCREEN_DESKTOP;
                SDL_SetWindowFullscreen(window, fullscreenFlags);
              }
              break;
            }
          }
          if((event.key.keysym.mod & (KMOD_ALT | KMOD_CTRL | KMOD_GUI)) == 0) {
            // only send keypress if not holding ctrl/alt/meta
            handleInput(snes, event.key.keysym.sym, true);
          }
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
          if(loaded) setTitle(window, droppedFile);
          SDL_free(droppedFile);
          wanted = 1.0 / (snes->palTiming ? 50.0 : 60.0);
          wantedSamples = want.freq / (snes->palTiming ? 50 : 60);
          break;
        }
      }
    }

    uint64_t curCount = SDL_GetPerformanceCounter();
    uint64_t delta = curCount - lastCount;
    lastCount = curCount;
    float seconds = delta / (float) countFreq;
    timeAdder += seconds;
    // allow 2 ms earlier, to prevent skipping due to being just below wanted
    while(timeAdder >= wanted - 0.002) {
      timeAdder -= wanted;
      // run frame
      if(loaded && (!paused || runOne)) {
        runOne = false;
        if(turbo) {
          snes_runFrame(snes);
        }
        snes_runFrame(snes);
        playAudio(snes, device, audioBuffer, wantedSamples);
        renderScreen(snes, renderer, texture);
      }
    }

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer); // should vsync
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

static void playAudio(Snes* snes, SDL_AudioDeviceID device, int16_t* audioBuffer, int wantedSamples) {
  snes_setSamples(snes, audioBuffer, wantedSamples);
  if(SDL_GetQueuedAudioSize(device) <= wantedSamples * 4 * 6) {
    // don't queue audio if buffer is still filled
    SDL_QueueAudio(device, audioBuffer, wantedSamples * 4);
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

static bool loadRom(const char* name, Snes* snes) {
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

static void setTitle(SDL_Window* window, const char* path) {
  // get last occurence of '/'
  const char* filename = strrchr(path, '/');
  if(filename == NULL) {
    filename = path;
  } else {
    filename += 1; // skip past '/' itself
  }
  int length = strlen(filename);
  char* title = malloc(length + 12); // "LakeSnes - ": 11 characters, +1 for '\0'
  strcpy(title, "LakeSnes - ");
  strcat(title, filename);
  SDL_SetWindowTitle(window, title);
  free(title);
}

static uint8_t* readFile(const char* name, size_t* length) {
  FILE* f = fopen(name, "rb");
  if(f == NULL) return NULL;
  fseek(f, 0, SEEK_END);
  int size = ftell(f);
  rewind(f);
  uint8_t* buffer = malloc(size);
  if(fread(buffer, size, 1, f) != 1) {
    fclose(f);
    return NULL;
  }
  fclose(f);
  *length = size;
  return buffer;
}
