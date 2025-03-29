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
#include "d3dx12.h"
#include "my.h"

using namespace adria;

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

void surface_request_loop(MyPluginRuntime* runtime) {
    while (true) {
        auto request = runtime->PullCreateSurfaceRequest();
        if (request != nullptr) {
            request->Respond(MySurfaceViewport {
                80, 15, 200, 200
            });
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

    std::string s = GetLastErrorAsString();


    auto created_runtime = my_create_runtime();
    auto runtime = created_runtime.runtime;
    auto guests = created_runtime.guests;
    auto render = created_runtime.render;    

    auto factory = runtime.GetFactory();
    auto device = runtime.GetDevice();
    auto queue = runtime.GetQueue();
    
    //std::ifstream file("C:\\Users\\mendy\\Desktop\\glb-to-webgpu-component.wasm", std::ios::binary | std::ios::ate);
    std::ifstream file("C:\\Code\\glb-to-webgpu\\glb-to-webgpu-component.wasm", std::ios::binary | std::ios::ate);
	//std::ifstream file("C:\\Code\\plugin-runtime\\guest\\target\\component.wasm", std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        throw std::exception("hello");
    }
    const uint8_t* uint8Ptr = reinterpret_cast<const uint8_t*>(buffer.data());
    auto module_id = runtime.AddWasmModuleBytes(uint8Ptr, size);
    /*
    std::thread guest_thread(&MyPluginRuntimeGuests::RunWasmModule, &guests, &module_id);
    guest_thread.detach();

    std::thread surface_request_thread(surface_request_loop, &runtime);
    surface_request_thread.detach();
    */

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
    g_Editor.Init(std::move(editor_init), &runtime, &render);
    window.GetWindowEvent().AddLambda([](WindowEventData const& msg_data) { g_Editor.OnWindowEvent(msg_data); });
    while (window.Loop())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        render.TriggerEventFrameToAll();
        std::this_thread::sleep_for(std::chrono::milliseconds(4));
        g_Editor.Run(&runtime, &module_id, &render);
    }
    g_Editor.Destroy();
    
}



