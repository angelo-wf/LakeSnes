
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include "strings.h"

#ifdef SDL2SUBDIR
#include "SDL2/SDL.h"
#else
#include "SDL.h"
#endif

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

static struct {
  // rendering
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* texture;
  // audio
  SDL_AudioDeviceID audioDevice;
  int audioFrequency;
  int16_t* audioBuffer;
  // paths
  char* prefPath;
  char* pathSeparator;
  // snes, timing
  Snes* snes;
  float wantedFrames;
  int wantedSamples;
  // loaded rom
  bool loaded;
  char* romName;
  char* savePath;
  char* statePath;
} glb = {};

static uint8_t* readFile(const char* name, int* length);
static void loadRom(const char* path);
static void closeRom(void);
static void setPaths(const char* path);
static void setTitle(const char* path);
static bool checkExtention(const char* name, bool forZip);
static void playAudio(void);
static void renderScreen(void);
static void handleInput(int keyCode, bool pressed);

int main(int argc, char** argv) {
  // set up SDL
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    printf("Failed to init SDL: %s\n", SDL_GetError());
    return 1;
  }
  glb.window = SDL_CreateWindow("LakeSnes", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 512, 480, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  if(glb.window == NULL) {
    printf("Failed to create window: %s\n", SDL_GetError());
    return 1;
  }
  glb.renderer = SDL_CreateRenderer(glb.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if(glb.renderer == NULL) {
    printf("Failed to create renderer: %s\n", SDL_GetError());
    return 1;
  }
  SDL_RenderSetLogicalSize(glb.renderer, 512, 480); // preserve aspect ratio
  glb.texture = SDL_CreateTexture(glb.renderer, SDL_PIXELFORMAT_RGBX8888, SDL_TEXTUREACCESS_STREAMING, 512, 480);
  if(glb.texture == NULL) {
    printf("Failed to create texture: %s\n", SDL_GetError());
    return 1;
  }
  // get pref path, create directories
  glb.prefPath = SDL_GetPrefPath("", "LakeSnes");
  char* savePath = malloc(strlen(glb.prefPath) + 6); // "saves" (5) + '\0'
  char* statePath = malloc(strlen(glb.prefPath) + 7); // "states" (6) + '\0'
  strcpy(savePath, glb.prefPath);
  strcat(savePath, "saves");
  strcpy(statePath, glb.prefPath);
  strcat(statePath, "states");
#ifdef _WIN32
  mkdir(savePath); // ignore mkdir error, this (should) mean that the directory already exists
  mkdir(statePath);
  glb.pathSeparator = "\\";
#else
  mkdir(savePath, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
  mkdir(statePath, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
  glb.pathSeparator = "/";
#endif
  free(savePath);
  free(statePath);
  // set up audio
  glb.audioFrequency = 48000;
  SDL_AudioSpec want, have;
  SDL_memset(&want, 0, sizeof(want));
  want.freq = glb.audioFrequency;
  want.format = AUDIO_S16;
  want.channels = 2;
  want.samples = 2048;
  want.callback = NULL; // use queue
  glb.audioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
  if(glb.audioDevice == 0) {
    printf("Failed to open audio device: %s\n", SDL_GetError());
    return 1;
  }
  glb.audioBuffer = malloc(glb.audioFrequency / 50 * 4); // *2 for stereo, *2 for sizeof(int16)
  SDL_PauseAudioDevice(glb.audioDevice, 0);
  // print version
  SDL_version version;
  SDL_version compiledVersion;
  SDL_GetVersion(&version);
  SDL_VERSION(&compiledVersion);
  printf(
    "LakeSnes - Running with SDL %d.%d.%d (compiled with %d.%d.%d)\n",
    version.major, version.minor, version.patch, compiledVersion.major, compiledVersion.minor, compiledVersion.patch
  );
  // init snes, load rom
  glb.snes = snes_init();
  glb.wantedFrames = 1.0 / 60.0;
  glb.wantedSamples = glb.audioFrequency / 60;
  glb.loaded = false;
  glb.romName = NULL;
  glb.savePath = NULL;
  glb.statePath = NULL;
  if(argc >= 2) {
    loadRom(argv[1]);
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

  while(running) {
    while(SDL_PollEvent(&event)) {
      switch(event.type) {
        case SDL_KEYDOWN: {
          switch(event.key.keysym.sym) {
            case SDLK_r: snes_reset(glb.snes, false); break;
            case SDLK_e: snes_reset(glb.snes, true); break;
            case SDLK_o: runOne = true; break;
            case SDLK_p: paused = !paused; break;
            case SDLK_t: turbo = true; break;
            case SDLK_j: {
              char* filePath = malloc(strlen(glb.prefPath) + 9); // "dump.bin" (8) + '\0'
              strcpy(filePath, glb.prefPath);
              strcat(filePath, "dump.bin");
              printf("Dumping to %s...\n", filePath);
              FILE* f = fopen(filePath, "wb");
              if(f == NULL) {
                puts("Failed to open file for writing");
                free(filePath);
                break;
              }
              fwrite(glb.snes->ram, 0x20000, 1, f);
              fwrite(glb.snes->ppu->vram, 0x10000, 1, f);
              fwrite(glb.snes->ppu->cgram, 0x200, 1, f);
              fwrite(glb.snes->ppu->oam, 0x200, 1, f);
              fwrite(glb.snes->ppu->highOam, 0x20, 1, f);
              fwrite(glb.snes->apu->ram, 0x10000, 1, f);
              fclose(f);
              free(filePath);
              break;
            }
            case SDLK_l: {
              // run one cpu cycle
              snes_runCpuCycle(glb.snes);
              char line[80];
              getProcessorStateCpu(glb.snes, line);
              puts(line);
              break;
            }
            case SDLK_k: {
              // run one spc cycle
              snes_runSpcCycle(glb.snes);
              char line[57];
              getProcessorStateSpc(glb.snes, line);
              puts(line);
              break;
            }
            case SDLK_m: {
              // save state
              int size = snes_saveState(glb.snes, NULL);
              uint8_t* stateData = malloc(size);
              snes_saveState(glb.snes, stateData);
              FILE* f = fopen(glb.statePath, "wb");
              if(f != NULL) {
                fwrite(stateData, size, 1, f);
                fclose(f);
                puts("Saved state");
              } else {
                puts("Failed to save state");
              }
              free(stateData);
              break;
            }
            case SDLK_n: {
              // load state
              int size = 0;
              uint8_t* stateData = readFile(glb.statePath, &size);
              if(stateData != NULL) {
                if(snes_loadState(glb.snes, stateData, size)) {
                  puts("Loaded state");
                } else {
                  puts("Failed to load state, file contents invalid");
                }
                free(stateData);
              } else {
                puts("Failed to load state, failed to read file");
              }
              break;
            }
            case SDLK_RETURN: {
              if(event.key.keysym.mod & KMOD_ALT) {
                fullscreenFlags ^= SDL_WINDOW_FULLSCREEN_DESKTOP;
                SDL_SetWindowFullscreen(glb.window, fullscreenFlags);
              }
              break;
            }
          }
          if((event.key.keysym.mod & (KMOD_ALT | KMOD_CTRL | KMOD_GUI)) == 0) {
            // only send keypress if not holding ctrl/alt/meta
            handleInput(event.key.keysym.sym, true);
          }
          break;
        }
        case SDL_KEYUP: {
          switch(event.key.keysym.sym) {
            case SDLK_t: turbo = false; break;
          }
          handleInput(event.key.keysym.sym, false);
          break;
        }
        case SDL_QUIT: {
          running = false;
          break;
        }
        case SDL_DROPFILE: {
          char* droppedFile = event.drop.file;
          loadRom(droppedFile);
          SDL_free(droppedFile);
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
    while(timeAdder >= glb.wantedFrames - 0.002) {
      timeAdder -= glb.wantedFrames;
      // run frame
      if(glb.loaded && (!paused || runOne)) {
        runOne = false;
        if(turbo) {
          snes_runFrame(glb.snes);
        }
        snes_runFrame(glb.snes);
        playAudio();
        renderScreen();
      }
    }

    SDL_RenderClear(glb.renderer);
    SDL_RenderCopy(glb.renderer, glb.texture, NULL, NULL);
    SDL_RenderPresent(glb.renderer); // should vsync
  }
  // close rom (saves battery)
  closeRom();
  // free snes
  snes_free(glb.snes);
  // clean sdl and free global allocs
  SDL_PauseAudioDevice(glb.audioDevice, 1);
  SDL_CloseAudioDevice(glb.audioDevice);
  free(glb.audioBuffer);
  SDL_free(glb.prefPath);
  if(glb.romName) free(glb.romName);
  if(glb.savePath) free(glb.savePath);
  if(glb.statePath) free(glb.statePath);
  SDL_DestroyTexture(glb.texture);
  SDL_DestroyRenderer(glb.renderer);
  SDL_DestroyWindow(glb.window);
  SDL_Quit();
  return 0;
}

static void playAudio() {
  snes_setSamples(glb.snes, glb.audioBuffer, glb.wantedSamples);
  if(SDL_GetQueuedAudioSize(glb.audioDevice) <= glb.wantedSamples * 4 * 6) {
    // don't queue audio if buffer is still filled
    SDL_QueueAudio(glb.audioDevice, glb.audioBuffer, glb.wantedSamples * 4);
  }
}

static void renderScreen() {
  void* pixels = NULL;
  int pitch = 0;
  if(SDL_LockTexture(glb.texture, NULL, &pixels, &pitch) != 0) {
    printf("Failed to lock texture: %s\n", SDL_GetError());
    return;
  }
  snes_setPixels(glb.snes, (uint8_t*) pixels);
  SDL_UnlockTexture(glb.texture);
}

static void handleInput(int keyCode, bool pressed) {
  switch(keyCode) {
    case SDLK_z: snes_setButtonState(glb.snes, 1, 0, pressed); break;
    case SDLK_a: snes_setButtonState(glb.snes, 1, 1, pressed); break;
    case SDLK_RSHIFT: snes_setButtonState(glb.snes, 1, 2, pressed); break;
    case SDLK_RETURN: snes_setButtonState(glb.snes, 1, 3, pressed); break;
    case SDLK_UP: snes_setButtonState(glb.snes, 1, 4, pressed); break;
    case SDLK_DOWN: snes_setButtonState(glb.snes, 1, 5, pressed); break;
    case SDLK_LEFT: snes_setButtonState(glb.snes, 1, 6, pressed); break;
    case SDLK_RIGHT: snes_setButtonState(glb.snes, 1, 7, pressed); break;
    case SDLK_x: snes_setButtonState(glb.snes, 1, 8, pressed); break;
    case SDLK_s: snes_setButtonState(glb.snes, 1, 9, pressed); break;
    case SDLK_d: snes_setButtonState(glb.snes, 1, 10, pressed); break;
    case SDLK_c: snes_setButtonState(glb.snes, 1, 11, pressed); break;
  }
}

static bool checkExtention(const char* name, bool forZip) {
  if(name == NULL) return false;
  int length = strlen(name);
  if(length < 4) return false;
  if(forZip) {
    if(strcasecmp(name + length - 4, ".zip") == 0) return true;
  } else {
    if(strcasecmp(name + length - 4, ".smc") == 0) return true;
    if(strcasecmp(name + length - 4, ".sfc") == 0) return true;
  }
  return false;
}

static void loadRom(const char* path) {
  // zip library from https://github.com/kuba--/zip
  int length = 0;
  uint8_t* file = NULL;
  if(checkExtention(path, true)) {
    struct zip_t* zip = zip_open(path, 0, 'r');
    if(zip != NULL) {
      int entries = zip_total_entries(zip);
      for(int i = 0; i < entries; i++) {
        zip_entry_openbyindex(zip, i);
        const char* zipFilename = zip_entry_name(zip);
        if(checkExtention(zipFilename, false)) {
          printf("Read \"%s\" from zip\n", zipFilename);
          size_t size = 0;
          zip_entry_read(zip, (void**) &file, &size);
          length = (int) size;
          break;
        }
        zip_entry_close(zip);
      }
      zip_close(zip);
    }
  } else {
    file = readFile(path, &length);
  }
  if(file == NULL) {
    printf("Failed to read file '%s'\n", path);
    return;
  }
  // close currently loaded rom (saves battery)
  closeRom();
  // load new rom
  if(snes_loadRom(glb.snes, file, length)) {
    // get rom name and paths, set title
    setPaths(path);
    setTitle(glb.romName);
    // set wantedFrames and wantedSamples
    glb.wantedFrames = 1.0 / (glb.snes->palTiming ? 50.0 : 60.0);
    glb.wantedSamples = glb.audioFrequency / (glb.snes->palTiming ? 50 : 60);
    glb.loaded = true;
    // load battery for loaded rom
    int size = 0;
    uint8_t* saveData = readFile(glb.savePath, &size);
    if(saveData != NULL) {
      if(snes_loadBattery(glb.snes, saveData, size)) {
        puts("Loaded battery data");
      } else {
        puts("Failed to load battery data");
      }
      free(saveData);
    }
  } // else, rom load failed, old rom still loaded
  free(file);
}

static void closeRom() {
  if(!glb.loaded) return;
  int size = snes_saveBattery(glb.snes, NULL);
  if(size > 0) {
    uint8_t* saveData = malloc(size);
    snes_saveBattery(glb.snes, saveData);
    FILE* f = fopen(glb.savePath, "wb");
    if(f != NULL) {
      fwrite(saveData, size, 1, f);
      fclose(f);
      puts("Saved battery data");
    } else {
      puts("Failed to save battery data");
    }
    free(saveData);
  }
}

static void setPaths(const char* path) {
  // get rom name
  if(glb.romName) free(glb.romName);
  const char* filename = strrchr(path, '/'); // get last occurence of '/'
  if(filename == NULL) {
    filename = path;
  } else {
    filename += 1; // skip past '/' itself
  }
  glb.romName = malloc(strlen(filename) + 1); // +1 for '\0'
  strcpy(glb.romName, filename);
  // get extension length
  const char* extStart = strrchr(glb.romName, '.'); // last occurence of '.'
  int extLen = extStart == NULL ? 0 : strlen(extStart);
  // get save name
  if(glb.savePath) free(glb.savePath);
  glb.savePath = malloc(strlen(glb.prefPath) + strlen(glb.romName) + 11); // "saves/" (6) + ".srm" (4) + '\0'
  strcpy(glb.savePath, glb.prefPath);
  strcat(glb.savePath, "saves");
  strcat(glb.savePath, glb.pathSeparator);
  strncat(glb.savePath, glb.romName, strlen(glb.romName) - extLen); // cut off extension
  strcat(glb.savePath, ".srm");
  // get state name
  if(glb.statePath) free(glb.statePath);
  glb.statePath = malloc(strlen(glb.prefPath) + strlen(glb.romName) + 12); // "states/" (7) + ".lss" (4) + '\0'
  strcpy(glb.statePath, glb.prefPath);
  strcat(glb.statePath, "states");
  strcat(glb.statePath, glb.pathSeparator);
  strncat(glb.statePath, glb.romName, strlen(glb.romName) - extLen); // cut off extension
  strcat(glb.statePath, ".lss");
}

static void setTitle(const char* romName) {
  if(romName == NULL) {
    SDL_SetWindowTitle(glb.window, "LakeSnes");
    return;
  }
  char* title = malloc(strlen(romName) + 12); // "LakeSnes - " (11) + '\0'
  strcpy(title, "LakeSnes - ");
  strcat(title, romName);
  SDL_SetWindowTitle(glb.window, title);
  free(title);
}

static uint8_t* readFile(const char* name, int* length) {
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
