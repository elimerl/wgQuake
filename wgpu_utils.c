// #include "wgpu_utils.h"
// #include <assert.h>
// #include <stdio.h>

// #ifdef __EMSCRIPTEN__
// #include <emscripten.h>
// #endif

// typedef struct {
//   WGPUAdapter adapter;
//   bool requestEnded;
// } AdapterRequestUserData;
// AdapterRequestUserData adapterReqUserData;

// // by the callback as its last argument.
// void onAdapterRequestEnded(WGPURequestAdapterStatus status, WGPUAdapter
// adapter,
//                            char const *message, void *pUserData) {
//   AdapterRequestUserData *userData = (AdapterRequestUserData *)pUserData;
//   if (status == WGPURequestAdapterStatus_Success) {
//     userData->adapter = adapter;
//   } else {
//     fprintf(stderr, "Could not get WebGPU adapter! %s", message);
//   }
//   userData->requestEnded = true;
// };

// WGPUAdapter requestAdapterSync(WGPUInstance instance,
//                                WGPURequestAdapterOptions const *options) {
//   // A simple structure holding the local information shared with the
//   // onAdapterRequestEnded callback.

//   // Call to the WebGPU request adapter procedure
//   wgpuInstanceRequestAdapter(instance /* equivalent of navigator.gpu */,
//                              options, onAdapterRequestEnded,
//                              (void *)&adapterReqUserData);

// #ifdef __EMSCRIPTEN__
//   while (!adapterReqUserData.requestEnded) {
//     emscripten_sleep(10);
//   }
// #endif
//   assert(adapterReqUserData.requestEnded);

//   return adapterReqUserData.adapter;
// }

// typedef struct {
//   WGPUDevice device;
//   bool requestEnded;
// } DeviceRequestUserData;
// DeviceRequestUserData deviceReqUserData;

// // by the callback as its last argument.
// void onDeviceRequestEnded(WGPURequestDeviceStatus status, WGPUDevice device,
//                           char const *message, void *pUserData) {
//   DeviceRequestUserData *userData = (DeviceRequestUserData *)pUserData;
//   if (status == WGPURequestDeviceStatus_Success) {
//     userData->device = device;
//   } else {
//     fprintf(stderr, "Could not get WebGPU device! %s", message);
//   }
//   userData->requestEnded = true;
// };
// WGPUDevice requestDeviceSync(WGPUAdapter adapter,
//                              WGPUDeviceDescriptor const *descriptor) {

//   wgpuAdapterRequestDevice(adapter, descriptor, onDeviceRequestEnded,
//                            (void *)&deviceReqUserData);

// #ifdef __EMSCRIPTEN__
//   while (!deviceReqUserData.requestEnded) {
//     emscripten_sleep(10);
//   }
// #endif // __EMSCRIPTEN__

//   assert(deviceReqUserData.requestEnded);

//   return deviceReqUserData.device;
// }
