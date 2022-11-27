
#ifndef TRACING_H
#define TRACING_H

#include <stdint.h>
#include <stdbool.h>

#include "snes.h"

void getProcessorStateCpu(Snes* snes, char* line);
void getProcessorStateSpc(Snes* snes, char* line);

#endif
