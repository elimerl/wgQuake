#include "wgpu_app.h"
#include "sdl2webgpu.h"
#include "wgpu_utils.h"
#include <stdio.h>
#include <webgpu/webgpu.h>
#if defined(WEBGPU_BACKEND_WGPU)
#include <webgpu/wgpu.h>
#endif
RenderAppData app;

void onDeviceError(WGPUErrorType type, char const *message, void *userData) {
  fprintf(stderr, "Uncaptured device error: type %i", type);
  if (message)
    fprintf(stderr, "(%s)", message);
  fprintf(stderr, "\n");
}

int RenderApp_Init() {
  // Init WebGPU
  WGPUInstanceDescriptor desc = {.nextInChain = NULL};
  app.instance = wgpuCreateInstance(&desc);

  // Init SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Could not init SDL!");
    return 1;
  }
  app.window = SDL_CreateWindow("wgQuake", SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);

  if (!app.window) {
    fprintf(stderr, "Could not init window!");
    return 1;
  }

  app.surface = SDL_GetWGPUSurface(app.instance, app.window);

  WGPURequestAdapterOptions adapterOpt = {
      .compatibleSurface = app.surface,
      .powerPreference = WGPUPowerPreference_HighPerformance};

  app.adapter = requestAdapterSync(app.instance, &adapterOpt);
  if (!app.adapter) {
    fprintf(stderr, "Could not init WebGPU adapter!");
    return 1;
  }

  WGPUDeviceDescriptor deviceDesc = {};
  app.device = requestDeviceSync(app.adapter, &deviceDesc);

  if (!app.device) {
    fprintf(stderr, "Could not init WebGPU device!");
    return 1;
  }
  wgpuDeviceSetUncapturedErrorCallback(app.device, onDeviceError, NULL);

  app.queue = wgpuDeviceGetQueue(app.device);

  int wwidth, wheight;
  SDL_GetWindowSize(app.window, &wwidth, &wheight);
  WGPUTextureFormat surfaceFormat =
      wgpuSurfaceGetPreferredFormat(app.surface, app.adapter);
  WGPUSurfaceConfiguration config = {.nextInChain = NULL,
                                     .width = wwidth,
                                     .height = wheight,
                                     .usage = WGPUTextureUsage_RenderAttachment,
                                     .format = surfaceFormat,
                                     .viewFormatCount = 0,
                                     .viewFormats = NULL,
                                     .device = app.device,
                                     .presentMode = WGPUPresentMode_Fifo,
                                     .alphaMode = WGPUCompositeAlphaMode_Auto};
  wgpuSurfaceConfigure(app.surface, &config);

  // Wait for close
  SDL_Event event;
  while (1) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        break;
      else if (event.type == SDL_WINDOWEVENT &&
               event.window.event == SDL_WINDOWEVENT_CLOSE)
        break;
    }

    printf("frame\n");

    WGPUTextureView targetView = RenderApp_NextSurfaceTextureView();
    if (!targetView)
      return 1;

    WGPUCommandEncoderDescriptor encoderDesc = {};
    encoderDesc.nextInChain = NULL;
    WGPUCommandEncoder encoder =
        wgpuDeviceCreateCommandEncoder(app.device, &encoderDesc);

    WGPURenderPassColorAttachment renderPassColorAttachment = {
        .view = targetView,
        .resolveTarget = NULL,
        .loadOp = WGPULoadOp_Clear,
        .storeOp = WGPUStoreOp_Store,
        .clearValue = {1.0, 0.0, 0.0, 1.0}};

    WGPURenderPassDescriptor renderPassDesc = {.nextInChain = NULL,
                                               .colorAttachmentCount = 1,
                                               .colorAttachments =
                                                   &renderPassColorAttachment,
                                               .depthStencilAttachment = NULL,
                                               .timestampWrites = NULL};

    WGPURenderPassEncoder renderPass =
        wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
    wgpuRenderPassEncoderEnd(renderPass);
    wgpuRenderPassEncoderRelease(renderPass);
    printf("encoded\n");

    WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
    cmdBufferDescriptor.nextInChain = NULL;
    WGPUCommandBuffer command =
        wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
    wgpuCommandEncoderRelease(encoder);

    wgpuQueueSubmit(app.queue, 1, &command);
    wgpuCommandBufferRelease(command);

    wgpuTextureViewRelease(targetView);
    wgpuSurfacePresent(app.surface);

#if defined(WEBGPU_BACKEND_DAWN)
    wgpuDeviceTick(device);
#elif defined(WEBGPU_BACKEND_WGPU)
    wgpuDevicePoll(app.device, false, NULL);
#endif
  }

  wgpuQueueRelease(app.queue);
  wgpuDeviceRelease(app.device);
  wgpuAdapterRelease(app.adapter);
  wgpuInstanceRelease(app.instance);

  wgpuSurfaceRelease(app.surface);

  // Terminate SDL
  SDL_DestroyWindow(app.window);
  SDL_Quit();

  return 0;
}

WGPUTextureView RenderApp_NextSurfaceTextureView() {
  WGPUSurfaceTexture surfaceTexture;
  wgpuSurfaceGetCurrentTexture(app.surface, &surfaceTexture);
  if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
    return NULL;
  }
  printf("%p\n", surfaceTexture.texture);

  WGPUTextureViewDescriptor viewDescriptor = {
      .nextInChain = NULL,
      .label = NULL,
      .format = wgpuTextureGetFormat(surfaceTexture.texture),
      .dimension = WGPUTextureViewDimension_2D,
      .baseMipLevel = 0,
      .mipLevelCount = 1,
      .baseArrayLayer = 0,
      .arrayLayerCount = 1,
      .aspect = WGPUTextureAspect_All,
  };

  WGPUTextureView view =
      wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

  printf("view %p\n", view);

  return view;
}