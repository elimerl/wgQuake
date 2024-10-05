#include "SDL_video.h"
#include "quakedef.h"
#include "sdl2webgpu.h"
#include <stdio.h>
#include <webgpu/webgpu.h>

#if defined(WEBGPU_BACKEND_WGPU)
#include <webgpu/wgpu.h>
#endif

WG *WG_Instance() { return &wgpu; }

typedef struct {
  WGPUAdapter adapter;
  bool requestEnded;
} AdapterRequestUserData;
static AdapterRequestUserData adapterReqUserData;

// by the callback as its last argument.
void onAdapterRequestEnded(WGPURequestAdapterStatus status, WGPUAdapter adapter,
                           char const *message, void *pUserData) {
  AdapterRequestUserData *userData = (AdapterRequestUserData *)pUserData;
  if (status == WGPURequestAdapterStatus_Success) {
    userData->adapter = adapter;
  } else {
    fprintf(stderr, "Could not get WebGPU adapter! %s", message);
  }
  userData->requestEnded = true;
};

WGPUAdapter requestAdapterSync(WGPUInstance instance,
                               WGPURequestAdapterOptions const *options) {
  // A simple structure holding the local information shared with the
  // onAdapterRequestEnded callback.

  // Call to the WebGPU request adapter procedure
  wgpuInstanceRequestAdapter(instance /* equivalent of navigator.gpu */,
                             options, onAdapterRequestEnded,
                             (void *)&adapterReqUserData);

#ifdef __EMSCRIPTEN__
  while (!adapterReqUserData.requestEnded) {
    emscripten_sleep(10);
  }
#endif
  assert(adapterReqUserData.requestEnded);

  return adapterReqUserData.adapter;
}

typedef struct {
  WGPUDevice device;
  bool requestEnded;
} DeviceRequestUserData;
static DeviceRequestUserData deviceReqUserData;

// by the callback as its last argument.
void onDeviceRequestEnded(WGPURequestDeviceStatus status, WGPUDevice device,
                          char const *message, void *pUserData) {
  DeviceRequestUserData *userData = (DeviceRequestUserData *)pUserData;
  if (status == WGPURequestDeviceStatus_Success) {
    userData->device = device;
  } else {
    fprintf(stderr, "Could not get WebGPU device! %s", message);
  }
  userData->requestEnded = true;
};
WGPUDevice requestDeviceSync(WGPUAdapter adapter,
                             WGPUDeviceDescriptor const *descriptor) {
  printf("%p %p %p %p\n", adapter, descriptor->nextInChain,
         onDeviceRequestEnded, (void *)&deviceReqUserData);
  wgpuAdapterRequestDevice(adapter, descriptor, onDeviceRequestEnded,
                           (void *)&deviceReqUserData);

#ifdef __EMSCRIPTEN__
  while (!deviceReqUserData.requestEnded) {
    emscripten_sleep(10);
  }
#endif // __EMSCRIPTEN__

  assert(deviceReqUserData.requestEnded);

  return deviceReqUserData.device;
}

void onDeviceError(WGPUErrorType type, char const *message, void *userData) {
  if (message)
    Sys_Error(stderr, "WG Uncaptured device error: type %i (%s)", type);
  else
    Sys_Error(stderr, "WG Uncaptured device error: type %i", type);
}

int WG_Init(SDL_Window *window) {
  wgpu.window = window;
  // Init WebGPU
  WGPUInstanceDescriptor desc = {
      .nextInChain = NULL,
  };
#ifdef __EMSCRIPTEN__
  wgpu.instance = wgpuCreateInstance(NULL);
#else
  wgpu.instance = wgpuCreateInstance(&desc);
#endif

  wgpu.surface = SDL_GetWGPUSurface(wgpu.instance, wgpu.window);

  WGPURequestAdapterOptions adapterOpt = {
      .compatibleSurface = wgpu.surface,
      .powerPreference = WGPUPowerPreference_HighPerformance};

  wgpu.adapter = requestAdapterSync(wgpu.instance, &adapterOpt);
  if (!wgpu.adapter) {
    fprintf(stderr, "Could not init WebGPU adapter!");
    return 1;
  }
  printf("%p\n", wgpu.adapter);

  WGPUDeviceDescriptor deviceDesc = {0};
  wgpu.device = requestDeviceSync(wgpu.adapter, &deviceDesc);

  if (!wgpu.device) {
    fprintf(stderr, "Could not init WebGPU device!");
    return 1;
  }
  wgpuDeviceSetUncapturedErrorCallback(wgpu.device, onDeviceError, NULL);

  wgpu.queue = wgpuDeviceGetQueue(wgpu.device);

  int wwidth, wheight;
  SDL_GetWindowSize(wgpu.window, &wwidth, &wheight);
  wgpu.surfaceFormat =
      wgpuSurfaceGetPreferredFormat(wgpu.surface, wgpu.adapter);
  WGPUSurfaceConfiguration config = {.nextInChain = NULL,
                                     .width = wwidth,
                                     .height = wheight,
                                     .usage = WGPUTextureUsage_RenderAttachment,
                                     .format = wgpu.surfaceFormat,
                                     .viewFormatCount = 0,
                                     .viewFormats = NULL,
                                     .device = wgpu.device,
                                     .presentMode = WGPUPresentMode_Fifo,
                                     .alphaMode = WGPUCompositeAlphaMode_Auto};
  wgpuSurfaceConfigure(wgpu.surface, &config);

  return 0;
}

void WG_Destroy() {
  wgpuQueueRelease(wgpu.queue);
  wgpuDeviceRelease(wgpu.device);
  wgpuAdapterRelease(wgpu.adapter);
  wgpuInstanceRelease(wgpu.instance);

  wgpuSurfaceRelease(wgpu.surface);

  // Terminate SDL
  SDL_DestroyWindow(wgpu.window);
  SDL_Quit();
}

WGPUTextureView WG_NextSurfaceTextureView() {
  WGPUSurfaceTexture surfaceTexture;

  wgpuSurfaceGetCurrentTexture(wgpu.surface, &surfaceTexture);
  if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
    return NULL;
  }
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

  return view;
}

WGPUShaderModule WG_CreateShaderModule(const char *shaderSource) {
  WGPUShaderModuleWGSLDescriptor shaderCodeDesc = {
      .code = shaderSource,
      .chain = {NULL, WGPUSType_ShaderModuleWGSLDescriptor}};
  WGPUShaderModuleDescriptor shaderDesc = {
      .label = NULL, .nextInChain = &shaderCodeDesc.chain};
#ifdef WEBGPU_BACKEND_WGPU
  shaderDesc.hintCount = 0;
  shaderDesc.hints = NULL;
#endif
  return wgpuDeviceCreateShaderModule(wgpu.device, &shaderDesc);
}

WGPURenderPipeline WG_CreateRenderPipeline(WGPUShaderModule shader,
                                           const char *vertexEntryPoint,
                                           const char *fragEntryPoint) {
  WGPUVertexState vertexState = {.nextInChain = NULL,
                                 .bufferCount = 0,
                                 .buffers = NULL,
                                 .module = shader,
                                 .entryPoint = vertexEntryPoint,
                                 .constantCount = 0,
                                 .constants = NULL};

  WGPUBlendState blendState = {
      .color = {WGPUBlendOperation_Add, WGPUBlendFactor_SrcAlpha,
                WGPUBlendFactor_OneMinusSrcAlpha},
      .alpha = {WGPUBlendOperation_Add, WGPUBlendFactor_Zero,
                WGPUBlendFactor_One}};

  WGPUColorTargetState colorTarget = {.format = wgpu.surfaceFormat,
                                      .blend = &blendState,
                                      .writeMask = WGPUColorWriteMask_All};

  WGPUFragmentState fragmentState = {.nextInChain = NULL,
                                     .module = shader,
                                     .entryPoint = fragEntryPoint,
                                     .constantCount = 0,
                                     .constants = NULL,
                                     .targetCount = 1,
                                     .targets = &colorTarget};
  WGPURenderPipelineDescriptor desc = {
      .nextInChain = NULL,
      .label = NULL,

      .vertex = vertexState,
      .fragment = &fragmentState,
      .primitive = {.topology = WGPUPrimitiveTopology_TriangleList,
                    .stripIndexFormat = WGPUIndexFormat_Undefined,
                    .frontFace = WGPUFrontFace_CCW,
                    .cullMode = WGPUCullMode_Back},
      .layout = NULL,
      .multisample = {NULL, 1, ~0u, false},
      .depthStencil = NULL,
  };

  return wgpuDeviceCreateRenderPipeline(wgpu.device, &desc);
}

WGPUBuffer WG_CreateBuffer(WGPUBufferUsageFlags usage, uint64_t size) {
  WGPUBufferDescriptor desc = {.nextInChain = NULL,
                               .label = NULL,

                               .mappedAtCreation = false,
                               .size = size,
                               .usage = usage};

  return wgpuDeviceCreateBuffer(wgpu.device, &desc);
}

void WG_WriteBuffer(WGPUBuffer buffer, int size, char *data) {
  wgpuQueueWriteBuffer(wgpu.queue, buffer, 0, data, size);
}