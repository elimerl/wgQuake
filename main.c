#include <stdio.h>
#include <webgpu/webgpu.h>

int main_SDL(int argc, char *argv[]);

int main(void) {
  WGPUInstanceDescriptor desc;
  desc.nextInChain = NULL;

#ifdef WEBGPU_BACKEND_EMSCRIPTEN
  WGPUInstance instance = wgpuCreateInstance(NULL);
#else  //  WEBGPU_BACKEND_EMSCRIPTEN
  WGPUInstance instance = wgpuCreateInstance(&desc);
#endif //  WEBGPU_BACKEND_EMSCRIPTEN

  if (!instance) {
    fprintf(stderr, "Could not initialize WebGPU!\n");
    return 1;
  }

  printf("%p\n", (void *)instance);

  wgpuInstanceRelease(instance);

  main_SDL(0, NULL);

  return 0;
}
