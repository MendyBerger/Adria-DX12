#pragma once
#include <string>

namespace adria
{
	namespace CommandLineOptions
	{
		void Initialize(std::wstring const& cmd_line);

		std::string const& GetLogFile();
		Int GetLogLevel();
		std::string const& GetWindowTitle();
		Int GetWindowWidth();
		Int GetWindowHeight();
		Bool GetMaximizeWindow();
		std::string const& GetSceneFile();
		Bool GetVSync();
		Bool GetDebugDevice();
		Bool GetDebugDML();
		Bool GetShaderDebug();
		Bool GetDRED();
		Bool GetGpuValidation();
		Bool GetPIX();
		Bool GetAftermath();
		Bool GetPerfReport();
		Bool GetPerfHUD();
	}
}

