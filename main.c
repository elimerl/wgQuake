
#define SDL_MAIN_HANDLED
#include "sdl2webgpu.h"
#include <SDL2/SDL.h>
#include <webgpu/webgpu.h>

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "wgpu_app.h"

int main(int argc, char *argv[]) {
  RenderApp *app = RenderApp_Instance();
  RenderApp_Init();
  while (!app->shouldStop) {
    if (RenderApp_Tick()) {
      return 1;
    }
  }
  RenderApp_Destroy();
  return 0;
}