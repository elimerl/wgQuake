
#define SDL_MAIN_HANDLED
#include "sdl2webgpu.h"
#include <SDL2/SDL.h>
#include <webgpu/webgpu.h>

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "wgpu_app.h"

int main(int argc, char *argv[]) {
  RenderApp_Init();
  return 0;
}