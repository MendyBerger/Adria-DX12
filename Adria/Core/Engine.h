#pragma once
#include "Rendering/ViewportData.h"
#include "Rendering/SceneConfig.h"
#include "entt/entity/registry.hpp"
#include "C:\Users\mendy\Desktop\plugin_runtime.h"
#include "helper.h"

namespace adria
{
	struct WindowEventData;
	class Window;
	struct SceneConfig;
	class GfxDevice;
	class Renderer;
	class SceneLoader;
	struct EditorEvents;
	class ImGuiManager;
	class Camera;

	class Engine
	{
		friend class Editor;

	public:
		Engine(Window* window, std::string const& scene_file, struct PluginRuntimeRender*  pr_render, IDXGIFactory6* factory, ID3D12Device5* device, ID3D12CommandQueue* queue);
		ADRIA_NONCOPYABLE_NONMOVABLE(Engine)
		~Engine();

		void OnWindowEvent(WindowEventData const& msg_data);
		void Run(f_paint_frames paint_frames, struct PluginRuntimeRender*  pr_render);

	private:
		Window* window = nullptr;
		entt::registry reg;
		std::unique_ptr<Camera> camera;
		std::unique_ptr<GfxDevice> gfx;
		std::unique_ptr<Renderer> renderer;
		std::unique_ptr<SceneLoader> scene_loader;
		ViewportData viewport_data;
		std::optional<SceneConfig> scene_request;
		PluginRuntimeRender*  pr_render = nullptr;

	private:
		void InitializeScene(SceneConfig const&);
		void ProcessCVarIniFile(std::string const&);

		void NewSceneRequest(SceneConfig const& scene_cfg)
		{
			scene_request = scene_cfg;
		}
		void HandleSceneRequest();

		void Update(Float dt);
		void Render(f_paint_frames paint_frames, struct PluginRuntimeRender*  pr_render);

		void SetViewportData(ViewportData*);
		void RegisterEditorEventCallbacks(EditorEvents&);
	};
}