#pragma once


namespace adria
{
	class GfxDevice;
	class GfxBuffer;
	class GfxComputePipelineState;

	enum class TensorLayout
	{
		Default,
		NHWC
	};

	class DirectMLPassBase
	{
	protected:
		explicit DirectMLPassBase(GfxDevice* gfx);

		std::unique_ptr<GfxBuffer> CreateFilterTensor(std::vector<Float> const&, std::vector<Float> const&, std::span<const Uint32>);
		std::unique_ptr<GfxBuffer> CreateBiasTensor(std::vector<Float> const&, std::span<const Uint32>);
		std::vector<Uint16> CompressFilterWeights(std::vector<Float> const&, std::vector<Float> const&, std::span<const Uint32>);
		std::vector<Uint16> CompressBiasWeights(std::vector<Float> const&, std::span<const Uint32>);

	protected:
		GfxDevice* gfx;
		IDMLDevice* dml_device;
		IDMLCommandRecorder* dml_command_recorder;
		TensorLayout tensor_layout;
		std::unique_ptr<GfxComputePipelineState> tensor_to_texture_pso;
		std::unique_ptr<GfxComputePipelineState> texture_to_tensor_pso;
		Bool dml_managed_weights = true;

	private:
		void CreatePSOs();
		void GetTensorStrides(std::span<Uint32 const> sizes, std::span<Uint32> strides);
	};
}