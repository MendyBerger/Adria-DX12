#pragma once

typedef void(__cdecl* f_paint_frames)(struct PluginRuntimeRender*  self, void* host_back_buffer);
typedef void(__cdecl* f_trigger_event_camera_orientation)(const PluginRuntime *self, const WasmModuleId *id, CameraFfi camera);
