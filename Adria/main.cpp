#include <windows.h>
#include <iostream>
#include <functional>
#include <string>

#include "Core/Window.h"
#include "Core/Engine.h"
#include "Core/Input.h"
#include "Core/CommandLineOptions.h"
#include "Logging/FileLogger.h"
#include "Logging/OutputDebugStringLogger.h"
#include "Editor/Editor.h"
#include "Utilities/MemoryDebugger.h"
#include "Utilities/CLIParser.h"
#include "C:\Users\mendy\Desktop\plugin_runtime.h"
#include "helper.h"
#include "d3dx12.h"

using namespace adria;

typedef void(__cdecl* f_create_runtime)(struct CreatedRuntime** out);
typedef void(__cdecl* f_get_factory)(struct PluginRuntime* self, struct IDXGIFactory4** out);
typedef void(__cdecl* f_get_device)(struct PluginRuntime* self, struct ID3D12Device5** out);
typedef void(__cdecl* f_get_queue)(struct PluginRuntime* self, struct ID3D12CommandQueue** out);
typedef void(__cdecl* f_add_wasm_module_bytes)(struct PluginRuntime* self, const uint8_t* module_ptr, uintptr_t module_len, struct WasmModuleId** out);
typedef void(__cdecl* f_remove_wasm_module)(struct PluginRuntime* self, struct WasmModuleId id);
typedef void(__cdecl* f_pull_create_surface_request)(const PluginRuntime *self, CreateSurfaceRequest **out);
typedef void(__cdecl* f_trigger_event_pointer_up)(const struct PluginRuntime* self, const struct WasmModuleId* id, struct PointerEvent event);
typedef void(__cdecl* f_trigger_event_pointer_down)(const struct PluginRuntime* self, const struct WasmModuleId* id, struct PointerEvent event);
typedef void(__cdecl* f_trigger_event_pointer_move)(const struct PluginRuntime* self, const struct WasmModuleId* id, struct PointerEvent event);
typedef void(__cdecl* f_run_wasm_module)(const struct PluginRuntimeGuests* self, const struct WasmModuleId* id);
typedef void(__cdecl* f_trigger_event_frame_to_all)(struct PluginRuntimeRender* self);


class dbg_stream_for_cout
    : public std::stringbuf
{
public:
    ~dbg_stream_for_cout() { sync(); }
    int sync()
    {
        ::OutputDebugStringA(str().c_str());
        str(std::string()); // Clear the string buffer
        return 0;
    }
};
dbg_stream_for_cout g_DebugStreamFor_cout;


//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString()
{
    //Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return std::string(); //No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);

    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}


using Microsoft::WRL::ComPtr;

void surface_request_loop(f_pull_create_surface_request pull_create_surface_request, PluginRuntime* runtime) {
    while (true) {
        CreateSurfaceRequest * request = nullptr;
        pull_create_surface_request(runtime, &request);
        if (request != nullptr) {
            
        }
    }
}

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);

    HINSTANCE hGetProcIDDLL = LoadLibrary(L"C:\\Users\\mendy\\Desktop\\plugin_runtime.dll");


    std::string s = GetLastErrorAsString();
    //The specified module could not be found.
    if (!hGetProcIDDLL) {
        std::cout << s << std::endl;
        std::cout << "=============== could not load the dynamic library" << std::endl;
        return EXIT_FAILURE;
    }

    f_create_runtime create_runtime = (f_create_runtime)GetProcAddress(hGetProcIDDLL, "create_runtime");
    f_get_factory get_factory = (f_get_factory)GetProcAddress(hGetProcIDDLL, "get_factory");
    f_get_device get_device = (f_get_device)GetProcAddress(hGetProcIDDLL, "get_device");
    f_get_queue get_queue = (f_get_queue)GetProcAddress(hGetProcIDDLL, "get_queue");
    f_add_wasm_module_bytes add_wasm_module_bytes = (f_add_wasm_module_bytes)GetProcAddress(hGetProcIDDLL, "add_wasm_module_bytes");
    f_remove_wasm_module remove_wasm_module = (f_remove_wasm_module)GetProcAddress(hGetProcIDDLL, "remove_wasm_module");
    f_pull_create_surface_request pull_create_surface_request = (f_pull_create_surface_request)GetProcAddress(hGetProcIDDLL, "pull_create_surface_request");
    f_trigger_event_pointer_up trigger_event_pointer_up = (f_trigger_event_pointer_up)GetProcAddress(hGetProcIDDLL, "trigger_event_pointer_up");
    f_trigger_event_pointer_down trigger_event_pointer_down = (f_trigger_event_pointer_down)GetProcAddress(hGetProcIDDLL, "trigger_event_pointer_down");
    f_trigger_event_pointer_move trigger_event_pointer_move = (f_trigger_event_pointer_move)GetProcAddress(hGetProcIDDLL, "trigger_event_pointer_move");
    f_run_wasm_module run_wasm_module = (f_run_wasm_module)GetProcAddress(hGetProcIDDLL, "run_wasm_module");
    f_trigger_event_frame_to_all trigger_event_frame_to_all = (f_trigger_event_frame_to_all)GetProcAddress(hGetProcIDDLL, "trigger_event_frame_to_all");
    f_paint_frames paint_frames = (f_paint_frames)GetProcAddress(hGetProcIDDLL, "paint_frames");


    if (!create_runtime) {
        std::cout << "=============== could not locate create_runtime function" << std::endl;
        return EXIT_FAILURE;
    }
    if (!get_factory) {
        std::cout << "=============== could not locate get_factory function" << std::endl;
        return EXIT_FAILURE;
    }
    if (!get_device) {
        std::cout << "=============== could not locate get_device function" << std::endl;
        return EXIT_FAILURE;
    }
    if (!get_queue) {
        std::cout << "=============== could not locate get_queue function" << std::endl;
        return EXIT_FAILURE;
    }
    if (!remove_wasm_module) {
        std::cout << "=============== could not locate remove_wasm_module function" << std::endl;
        return EXIT_FAILURE;
    }
    if (!pull_create_surface_request) {
        std::cout << "=============== could not locate pull_create_surface_request function" << std::endl;
        return EXIT_FAILURE;
    }
    if (!trigger_event_pointer_up) {
        std::cout << "=============== could not locate trigger_event_pointer_up function" << std::endl;
        return EXIT_FAILURE;
    }
    if (!trigger_event_pointer_down) {
        std::cout << "=============== could not locate trigger_event_pointer_down function" << std::endl;
        return EXIT_FAILURE;
    }
    if (!trigger_event_pointer_move) {
        std::cout << "=============== could not locate trigger_event_pointer_move function" << std::endl;
        return EXIT_FAILURE;
    }
    if (!run_wasm_module) {
        std::cout << "=============== could not locate run_wasm_module function" << std::endl;
        return EXIT_FAILURE;
    }
    if (!trigger_event_frame_to_all) {
        std::cout << "=============== could not locate trigger_event_frame_to_all function" << std::endl;
        return EXIT_FAILURE;
    }



    CreatedRuntime* created_runtime = nullptr;
    create_runtime(&created_runtime);
    PluginRuntime* runtime = created_runtime->runtime;
    PluginRuntimeGuests* guests = created_runtime->guests;
    PluginRuntimeRender* render = created_runtime->render;
    IDXGIFactory4* factory = nullptr;
    get_factory(runtime, &factory);
    IDXGIAdapter1* adapter = nullptr;


    ID3D12Device5* device = nullptr;
    get_device(runtime, &device);


    ID3D12CommandQueue* queue = nullptr;
    get_queue(runtime, &queue);


    std::ifstream file("C:\\Users\\mendy\\Desktop\\component.wasm", std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        throw std::exception("hello");
    }
    WasmModuleId* module_id = nullptr;
    const uint8_t* uint8Ptr = reinterpret_cast<const uint8_t*>(buffer.data());
    add_wasm_module_bytes(runtime, uint8Ptr, size, &module_id);

    std::thread guest_thread(run_wasm_module, guests, module_id);
    guest_thread.detach();

    std::thread surface_request_thread(surface_request_loop, pull_create_surface_request, runtime);
    surface_request_thread.detach();


    CommandLineOptions::Initialize(lpCmdLine);
    
    std::string log_file = CommandLineOptions::GetLogFile();  
    LogLevel log_level = static_cast<LogLevel>(CommandLineOptions::GetLogLevel());
    g_Log.Register(new FileLogger(log_file.c_str(), log_level));
    g_Log.Register(new OutputDebugStringLogger(log_level));

    std::string window_title = CommandLineOptions::GetWindowTitle();
    WindowInit window_init{};
    window_init.width = CommandLineOptions::GetWindowWidth();
    window_init.height = CommandLineOptions::GetWindowHeight();
    window_init.title = window_title.c_str();
    window_init.maximize = CommandLineOptions::GetMaximizeWindow();
    Window window(window_init);
    g_Input.Initialize(&window);

    EditorInit editor_init{ .window = &window, .scene_file = CommandLineOptions::GetSceneFile() };
    IDXGIFactory6* factory6 = nullptr;
    factory->QueryInterface(IID_PPV_ARGS(&factory6));
    g_Editor.Init(std::move(editor_init), render, factory6, device, queue);
    window.GetWindowEvent().AddLambda([](WindowEventData const& msg_data) { g_Editor.OnWindowEvent(msg_data); });
    while (window.Loop())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        trigger_event_frame_to_all(render);
        std::this_thread::sleep_for(std::chrono::milliseconds(4));
        g_Editor.Run(paint_frames, render);
    }
    g_Editor.Destroy();
    
}



