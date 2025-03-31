#pragma once
#include "Rendering/ViewportData.h"
#include "Rendering/SceneConfig.h"
#include "entt/entity/registry.hpp"
#include "my.h"

namespace adria
{
	struct WindowEventInfo;
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
		Engine(Window* window, std::string const& scene_file, MyPluginRuntime* p_runtime, MyPluginRuntimeRender* pr_render);
		ADRIA_NONCOPYABLE_NONMOVABLE(Engine)
		~Engine();

		void Run(MyPluginRuntime* p_runtime, MyWasmModuleId* module_id, MyPluginRuntimeRender*  pr_render);
		void OnWindowEvent(WindowEventInfo const& msg_data);

		Renderer* GetRenderer()   const { return renderer.get(); }
		GfxDevice* GetGfxDevice() const { return gfx.get(); }

	private:
		Window* window = nullptr;
		entt::registry reg;
		std::unique_ptr<Camera> camera;
		std::unique_ptr<GfxDevice> gfx;
		std::unique_ptr<Renderer> renderer;
		std::unique_ptr<SceneLoader> scene_loader;
		ViewportData viewport_data;
		std::optional<SceneConfig> scene_request;
		MyPluginRuntimeRender*  pr_render = nullptr;

	private:
		void InitializeScene(SceneConfig const&);
		void ProcessCVarIniFile(std::string const&);

		void NewSceneRequest(SceneConfig const& scene_cfg)
		{
			scene_request = scene_cfg;
		}
		void HandleSceneRequest();

		void Update(Float dt, MyPluginRuntime*  p_runtime, MyWasmModuleId* module_id);
		void Render(MyPluginRuntimeRender*  pr_render);

		void SetViewportData(ViewportData*);
		void RegisterEditorEventCallbacks(EditorEvents&);
	};
}