#pragma once

#include <windows.h>
#include <iostream>
#include <functional>
#include <string>
#include "C:\Users\mendy\Desktop\plugin_runtime.h"
#include "d3dx12.h"
#include <dxgi1_4.h>

typedef void(__cdecl* f_create_runtime)(struct CreatedRuntime** out);
typedef void(__cdecl* f_get_factory)(struct PluginRuntime* self, struct IDXGIFactory4** out);
typedef void(__cdecl* f_get_device)(struct PluginRuntime* self, struct ID3D12Device5** out);
typedef void(__cdecl* f_get_queue)(struct PluginRuntime* self, struct ID3D12CommandQueue** out);
typedef void(__cdecl* f_add_wasm_module_bytes)(struct PluginRuntime* self, const uint8_t* module_ptr, uintptr_t module_len, struct WasmModuleId** out);
typedef void(__cdecl* f_remove_wasm_module)(struct PluginRuntime* self, struct WasmModuleId id);
typedef void(__cdecl* f_pull_create_surface_request)(const PluginRuntime *self, CreateSurfaceRequest **out);
typedef void(__cdecl* f_create_surface_response)(const CreateSurfaceRequest self, SurfaceViewport viewport);
typedef void(__cdecl* f_trigger_event_pointer_up_to_relevant_surface)(const struct PluginRuntime* self, struct PointerEvent event);
typedef void(__cdecl* f_trigger_event_pointer_up)(const struct PluginRuntime* self, const struct WasmModuleId* id, struct PointerEvent event);
typedef void(__cdecl* f_trigger_event_pointer_down_to_relevant_surface)(const struct PluginRuntime* self, struct PointerEvent event);
typedef void(__cdecl* f_trigger_event_pointer_down)(const struct PluginRuntime* self, const struct WasmModuleId* id, struct PointerEvent event);
typedef void(__cdecl* f_trigger_event_pointer_move_to_relevant_surface)(const struct PluginRuntime* self, struct PointerEvent event);
typedef void(__cdecl* f_trigger_event_pointer_move)(const struct PluginRuntime* self, const struct WasmModuleId* id, struct PointerEvent event);
typedef void(__cdecl* f_run_wasm_module)(const struct PluginRuntimeGuests* self, const struct WasmModuleId* id); //
typedef void(__cdecl* f_trigger_event_frame_to_all)(struct PluginRuntimeRender* self);
typedef void(__cdecl* f_paint_frames)(struct PluginRuntimeRender*  self, void* host_back_buffer);
typedef void(__cdecl* f_trigger_event_camera_orientation)(const PluginRuntime *self, const WasmModuleId *id, CameraFfi camera);



struct MySurfaceViewport {
    uint32_t top;
    uint32_t left;
    uint32_t height;
    uint32_t width;
    SurfaceViewport ToFfi() const;
};

struct MyPointerEvent {
    double x;
    double y;
    PointerEvent ToFfi() const;
};

struct MyKeyEvent {
    bool alt_key;
    bool ctrl_key;
    bool meta_key;
    bool shift_key;
    KeyEvent ToFfi() const;
};

struct MyQuaternion {
    float x;
    float y;
    float z;
    float w;
    QuaternionFfi ToFfi() const;
};

struct MyVec3 {
    float x;
    float y;
    float z;
    Vec3Ffi ToFfi() const;
};

struct MyCamera {
    MyQuaternion orientation;
    MyVec3 position;
    CameraFfi ToFfi() const;
};












class MyWasmModuleId {
private:
    
public:
    WasmModuleId* inner;
    MyWasmModuleId(WasmModuleId* inner);
};


class MyCreateSurfaceRequest {
private:
    CreateSurfaceRequest* inner;
    
public:
    MyCreateSurfaceRequest(CreateSurfaceRequest* inner);

    void Respond(MySurfaceViewport viewport);
};


class MyPluginRuntime {
private:
    PluginRuntime* inner;

public:
    MyPluginRuntime(PluginRuntime* inner);

    IDXGIFactory4* GetFactory() const;

    ID3D12Device5* GetDevice() const;

    ID3D12CommandQueue* GetQueue() const;

    MyWasmModuleId AddWasmModuleBytes(const uint8_t *module_ptr, uintptr_t module_len);

    MyCreateSurfaceRequest* PullCreateSurfaceRequest() const;

    void TriggerEventPointerUpToRelevantSurface(MyPointerEvent event) const;

    void TriggerEventPointerDownToRelevantSurface(MyPointerEvent event) const;

    void TriggerEventPointerMoveToRelevantSurface(MyPointerEvent event) const;

    void TriggerEventCameraOrientation(MyWasmModuleId * module_id, MyCamera event) const;
};


class MyPluginRuntimeGuests {
    private:
        PluginRuntimeGuests* inner;
    
    public:
        MyPluginRuntimeGuests(PluginRuntimeGuests* inner);

        
        void RunWasmModule(MyWasmModuleId * module_id) const;
};


class MyPluginRuntimeRender {
    private:
        PluginRuntimeRender* inner;
    
    public:
        MyPluginRuntimeRender(PluginRuntimeRender* inner);

        void TriggerEventFrameToAll() const;

        void PaintFrames(ID3D12Resource * back_buffer) const;
};


struct MyCreatedRuntime {
    MyPluginRuntime runtime;
    MyPluginRuntimeRender render;
    MyPluginRuntimeGuests guests;
};

// TODO: take path to .dll
MyCreatedRuntime my_create_runtime();
