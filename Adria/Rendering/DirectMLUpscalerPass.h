#pragma once
#include "UpscalerPass.h"
#include "DirectMLPassBase.h"

namespace adria
{
	template<Bool>
	class GfxRingDescriptorAllocator;
	using GfxOnlineDescriptorAllocator = GfxRingDescriptorAllocator<GFX_MULTITHREADED>;
	class RenderGraph;
	class PostProcessor;

	class DirectMLUpscalerPass : public UpscalerPass, public DirectMLPassBase
	{
		static constexpr Uint32 UPSAMPLE_LAYER_COUNT = 2;
		static constexpr Uint32 CONV_LAYER_COUNT = 7;
		static constexpr Uint32 INTERMEDIATE_BUFFER_COUNT = 2;

		static Bool LoadWeights(std::string const& weights_file_name, std::unordered_map<std::string, std::vector<Float>>& weight_map);

	public:
		explicit DirectMLUpscalerPass(GfxDevice* gfx, Uint32 w, Uint32 h);

		virtual void OnResize(Uint32 w, Uint32 h) override;
		virtual void AddPass(RenderGraph&, PostProcessor*) override;
		virtual Bool IsEnabled(PostProcessor const*) const override;
		virtual Bool IsSupported() const override;
		virtual void GUI() override;

	private:
		Uint32 display_width;
		Uint32 display_height;
		Uint32 render_width;
		Uint32 render_height;
		Bool supported = false;
		Bool needs_init = true;

		std::unique_ptr<GfxBuffer>   model_input;
		std::unique_ptr<GfxBuffer>   model_output;
		std::unique_ptr<GfxBuffer>   model_conv_filter_weights[CONV_LAYER_COUNT];
		std::unique_ptr<GfxBuffer>   model_conv_bias_weights[CONV_LAYER_COUNT];

		std::unique_ptr<GfxBuffer>   model_persistent_resource;
		std::unique_ptr<GfxBuffer>   model_temporary_resource;

		std::unique_ptr<GfxOnlineDescriptorAllocator> dml_heap;
		Ref<IDMLCompiledOperator>    dml_graph;
		Ref<IDMLBindingTable>        dml_binding_table;
		Ref<IDMLOperatorInitializer> dml_op_initializer;

	private:
		void AddTextureToTensorPass(RenderGraph&, PostProcessor const*);
		void AddUpscalingPass(RenderGraph&);
		void AddTensorToTexturePass(RenderGraph&, PostProcessor*);

		void CreateDirectMLResources();
		void InitializeDirectMLResources();
	};
}