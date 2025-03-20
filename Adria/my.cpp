#include <windows.h>
#include <iostream>
#include <functional>
#include <string>
#include "C:\Users\mendy\Desktop\plugin_runtime.h"
#include "d3dx12.h"
#include <dxgi1_4.h>
#include "my.h"

f_create_runtime ffi_create_runtime;
f_get_factory ffi_get_factory;
f_get_device ffi_get_device;
f_get_queue ffi_get_queue;
f_add_wasm_module_bytes ffi_add_wasm_module_bytes;
f_remove_wasm_module ffi_remove_wasm_module;
f_pull_create_surface_request ffi_pull_create_surface_request;
f_create_surface_response ffi_create_surface_response;
f_trigger_event_pointer_up_to_relevant_surface ffi_trigger_event_pointer_up_to_relevant_surface;
f_trigger_event_pointer_up ffi_trigger_event_pointer_up;
f_trigger_event_pointer_down_to_relevant_surface ffi_trigger_event_pointer_down_to_relevant_surface;
f_trigger_event_pointer_down ffi_trigger_event_pointer_down;
f_trigger_event_pointer_move_to_relevant_surface ffi_trigger_event_pointer_move_to_relevant_surface;
f_trigger_event_pointer_move ffi_trigger_event_pointer_move;
f_run_wasm_module ffi_run_wasm_module;
f_trigger_event_frame_to_all ffi_trigger_event_frame_to_all;
f_paint_frames ffi_paint_frames;
f_trigger_event_camera_orientation ffi_trigger_event_camera_orientation;



SurfaceViewport MySurfaceViewport::ToFfi() const {
    return SurfaceViewport {
        top,
        left,
        height,
        width,
    };
}

PointerEvent MyPointerEvent::ToFfi() const {
    return PointerEvent {
        x,
        y,
    };
}

KeyEvent MyKeyEvent::ToFfi() const {
    return KeyEvent {
        alt_key,
        ctrl_key,
        meta_key,
        shift_key,
    };
}

QuaternionFfi MyQuaternion::ToFfi() const {
    return QuaternionFfi {
        x,
        y,
        z,
        w,
    };
}

Vec3Ffi MyVec3::ToFfi() const {
    return Vec3Ffi {
        x,
        y,
        z,
    };
}

CameraFfi MyCamera::ToFfi() const {
    auto orientation = this->orientation.ToFfi();
    auto position = this->position.ToFfi();
    return CameraFfi {
        orientation,
        position,
    };
}


MyWasmModuleId::MyWasmModuleId(WasmModuleId* inner) {
    if (inner == nullptr)
        throw std::runtime_error("inner can't be null");
    this->inner = inner;
}



MyCreateSurfaceRequest::MyCreateSurfaceRequest(CreateSurfaceRequest* inner) {
    if (inner == nullptr) {
        throw std::runtime_error("inner can't be null");
    }
    this->inner = inner;
}

void MyCreateSurfaceRequest::Respond(MySurfaceViewport viewport) {
    ffi_create_surface_response(*inner, viewport.ToFfi());
}



MyPluginRuntime::MyPluginRuntime(PluginRuntime* inner) {
    if (inner == nullptr)
        throw std::runtime_error("inner can't be null");
    this->inner = inner;
}

IDXGIFactory4* MyPluginRuntime::GetFactory() const {
    IDXGIFactory4* factory = nullptr;
    ffi_get_factory(inner, &factory);
    return factory;
}

ID3D12Device5* MyPluginRuntime::GetDevice() const {
    ID3D12Device5* device = nullptr;
    ffi_get_device(inner, &device);
    return device;
}

ID3D12CommandQueue* MyPluginRuntime::GetQueue() const {
    ID3D12CommandQueue* queue = nullptr;
    ffi_get_queue(inner, &queue);
    return queue;
}

MyWasmModuleId MyPluginRuntime::AddWasmModuleBytes(const uint8_t *module_ptr, uintptr_t module_len) {
    WasmModuleId* module_id = nullptr;
    ffi_add_wasm_module_bytes(inner, module_ptr, module_len, &module_id);
    return MyWasmModuleId(module_id);
}

MyCreateSurfaceRequest* MyPluginRuntime::PullCreateSurfaceRequest() const {
    CreateSurfaceRequest * request = nullptr;
    ffi_pull_create_surface_request(inner, &request);
    if (request == nullptr) {
        return nullptr;
    }
    return new MyCreateSurfaceRequest(request);
}

void MyPluginRuntime::TriggerEventPointerUpToRelevantSurface(MyPointerEvent event) const {
    ffi_trigger_event_pointer_up_to_relevant_surface(inner, event.ToFfi());
}

void MyPluginRuntime::TriggerEventPointerDownToRelevantSurface(MyPointerEvent event) const {
    ffi_trigger_event_pointer_down_to_relevant_surface(inner, event.ToFfi());
}

void MyPluginRuntime::TriggerEventPointerMoveToRelevantSurface(MyPointerEvent event) const {
    ffi_trigger_event_pointer_move_to_relevant_surface(inner, event.ToFfi());
}

void MyPluginRuntime::TriggerEventCameraOrientation(MyWasmModuleId * module_id, MyCamera event) const {
    ffi_trigger_event_camera_orientation(inner, module_id->inner, event.ToFfi());
}


MyPluginRuntimeGuests::MyPluginRuntimeGuests(PluginRuntimeGuests* inner) {
    if (inner == nullptr)
        throw std::runtime_error("inner can't be null");
    this->inner = inner;
}


void MyPluginRuntimeGuests::RunWasmModule(MyWasmModuleId * module_id) const {
    ffi_run_wasm_module(inner, module_id->inner);
}



MyPluginRuntimeRender::MyPluginRuntimeRender(PluginRuntimeRender* inner) {
    if (inner == nullptr)
        throw std::runtime_error("inner can't be null");
    this->inner = inner;
}

void MyPluginRuntimeRender::TriggerEventFrameToAll() const {
    ffi_trigger_event_frame_to_all(inner);
}

void MyPluginRuntimeRender::PaintFrames(ID3D12Resource * back_buffer) const {
    ffi_paint_frames(inner, back_buffer);
}


// TODO: take path to .dll
MyCreatedRuntime my_create_runtime() {
    HINSTANCE hGetProcIDDLL = LoadLibrary(L"C:\\Users\\mendy\\Desktop\\plugin_runtime.dll");

    if (!hGetProcIDDLL) {
        throw std::runtime_error("Could not load the dynamic library");
    }

    ffi_create_runtime = (f_create_runtime)GetProcAddress(hGetProcIDDLL, "create_runtime");
    ffi_get_factory = (f_get_factory)GetProcAddress(hGetProcIDDLL, "get_factory");
    ffi_get_device = (f_get_device)GetProcAddress(hGetProcIDDLL, "get_device");
    ffi_get_queue = (f_get_queue)GetProcAddress(hGetProcIDDLL, "get_queue");
    ffi_add_wasm_module_bytes = (f_add_wasm_module_bytes)GetProcAddress(hGetProcIDDLL, "add_wasm_module_bytes");
    ffi_remove_wasm_module = (f_remove_wasm_module)GetProcAddress(hGetProcIDDLL, "remove_wasm_module");
    ffi_pull_create_surface_request = (f_pull_create_surface_request)GetProcAddress(hGetProcIDDLL, "pull_create_surface_request");
    ffi_create_surface_response = (f_create_surface_response)GetProcAddress(hGetProcIDDLL, "create_surface_response");
    ffi_trigger_event_pointer_up_to_relevant_surface = (f_trigger_event_pointer_up_to_relevant_surface)GetProcAddress(hGetProcIDDLL, "trigger_event_pointer_up_to_relevant_surface");
    ffi_trigger_event_pointer_up = (f_trigger_event_pointer_up)GetProcAddress(hGetProcIDDLL, "trigger_event_pointer_up");
    ffi_trigger_event_pointer_down_to_relevant_surface = (f_trigger_event_pointer_down_to_relevant_surface)GetProcAddress(hGetProcIDDLL, "trigger_event_pointer_down_to_relevant_surface");
    ffi_trigger_event_pointer_down = (f_trigger_event_pointer_down)GetProcAddress(hGetProcIDDLL, "trigger_event_pointer_down");
    ffi_trigger_event_pointer_move_to_relevant_surface = (f_trigger_event_pointer_move_to_relevant_surface)GetProcAddress(hGetProcIDDLL, "trigger_event_pointer_move_to_relevant_surface");
    ffi_trigger_event_pointer_move = (f_trigger_event_pointer_move)GetProcAddress(hGetProcIDDLL, "trigger_event_pointer_move");
    ffi_run_wasm_module = (f_run_wasm_module)GetProcAddress(hGetProcIDDLL, "run_wasm_module");
    ffi_trigger_event_frame_to_all = (f_trigger_event_frame_to_all)GetProcAddress(hGetProcIDDLL, "trigger_event_frame_to_all");
    ffi_paint_frames = (f_paint_frames)GetProcAddress(hGetProcIDDLL, "paint_frames");
    ffi_trigger_event_camera_orientation = (f_trigger_event_camera_orientation)GetProcAddress(hGetProcIDDLL, "trigger_event_camera_orientation");

    if (!ffi_create_runtime) {
        throw std::runtime_error("Could not locate create_runtime function");
    }
    if (!ffi_get_factory) {
        throw std::runtime_error("Could not locate get_factory function");
    }
    if (!ffi_get_device) {
        throw std::runtime_error("Could not locate get_device function");
    }
    if (!ffi_get_queue) {
        throw std::runtime_error("Could not locate get_queue function");
    }
    if (!ffi_remove_wasm_module) {
        throw std::runtime_error("Could not locate remove_wasm_module function");
    }
    if (!ffi_trigger_event_pointer_up_to_relevant_surface) {
        throw std::runtime_error("Could not locate trigger_event_pointer_up_to_relevant_surface function");
    }
    if (!ffi_pull_create_surface_request) {
        throw std::runtime_error("inner can't be null");
    }
    if (!ffi_create_surface_response) {
        throw std::runtime_error("Could not locate create_surface_response function");
    }
    if (!ffi_trigger_event_pointer_down_to_relevant_surface) {
        throw std::runtime_error("Could not locate trigger_event_pointer_down_to_relevant_surface function");
    }
    if (!ffi_trigger_event_pointer_up) {
        throw std::runtime_error("Could not locate trigger_event_pointer_up function");
    }
    if (!ffi_trigger_event_pointer_move_to_relevant_surface) {
        throw std::runtime_error("Could not locate trigger_event_pointer_move_to_relevant_surface function");
    }
    if (!ffi_trigger_event_pointer_down) {
        throw std::runtime_error("Could not locate trigger_event_pointer_down function");
    }
    if (!ffi_trigger_event_pointer_move) {
        throw std::runtime_error("Could not locate trigger_event_pointer_move function");
    }
    if (!ffi_run_wasm_module) {
        throw std::runtime_error("Could not locate run_wasm_module function");
    }
    if (!ffi_trigger_event_frame_to_all) {
        throw std::runtime_error("Could not locate trigger_event_frame_to_all function");
    }

    CreatedRuntime* created_runtime = nullptr;
    ffi_create_runtime(&created_runtime);
    return MyCreatedRuntime {
        MyPluginRuntime(created_runtime->runtime),
        MyPluginRuntimeRender(created_runtime->render),
        MyPluginRuntimeGuests(created_runtime->guests),
    };
}
