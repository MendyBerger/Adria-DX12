#include "DirectMLPassBase.h"
#include "Graphics/GfxDevice.h"
#include "Graphics/GfxBuffer.h"
#include "ShaderManager.h"
#include "Graphics/GfxPipelineState.h"
#include "Utilities/FloatCompressor.h"


namespace adria
{
	
	DirectMLPassBase::DirectMLPassBase(GfxDevice* gfx) : gfx(gfx), dml_device(gfx->GetDMLDevice()), dml_command_recorder(gfx->GetDMLCommandRecorder())
	{
		tensor_layout = gfx->GetVendor() == GfxVendor::Nvidia ? TensorLayout::NHWC : TensorLayout::Default;
		CreatePSOs();
	}

	void DirectMLPassBase::CreatePSOs()
	{
		GfxComputePipelineStateDesc pso_desc{};
		pso_desc.CS = CS_TensorToTexture;
		tensor_to_texture_pso = gfx->CreateComputePipelineState(pso_desc);

		pso_desc.CS = CS_TextureToTensor;
		texture_to_tensor_pso = gfx->CreateComputePipelineState(pso_desc);
	}

	void DirectMLPassBase::GetTensorStrides(std::span<Uint32 const> sizes, std::span<Uint32> strides)
	{
		switch (tensor_layout)
		{
		case TensorLayout::NHWC:
			strides[0] = sizes[1] * sizes[2] * sizes[3];
			strides[1] = 1;
			strides[2] = sizes[1] * sizes[3];
			strides[3] = sizes[1];
			break;
		default:
			strides[0] = sizes[1] * sizes[2] * sizes[3];
			strides[1] = sizes[2] * sizes[3];
			strides[2] = sizes[3];
			strides[3] = 1;
		}
	}


	std::unique_ptr<GfxBuffer> DirectMLPassBase::CreateFilterTensor(std::vector<Float> const& filter_weights, std::vector<Float> const& scale_weights, std::span<const Uint32> filter_sizes)
	{
		Bool const use_scale_shift = !scale_weights.empty();
		std::vector<Uint16> compressed_filter_weights = CompressFilterWeights(filter_weights, scale_weights, filter_sizes);
		Uint32 filter_strides[4];
		GetTensorStrides(filter_sizes, filter_strides);
		Uint64 filter_buffer_size = DMLCalcBufferTensorSize(DML_TENSOR_DATA_TYPE_FLOAT16, 4, filter_sizes.data(), filter_strides);

		GfxBufferDesc filter_tensor_desc{};
		filter_tensor_desc.bind_flags = GfxBindFlag::ShaderResource | GfxBindFlag::UnorderedAccess;
		filter_tensor_desc.size = filter_buffer_size;
		filter_tensor_desc.resource_usage = GfxResourceUsage::Default;
		GfxBufferData gfx_filter_data(compressed_filter_weights.data());
		return gfx->CreateBuffer(filter_tensor_desc, gfx_filter_data);
	}

	std::unique_ptr<GfxBuffer> DirectMLPassBase::CreateBiasTensor(std::vector<Float> const& shift_weights, std::span<const Uint32> filter_sizes)
	{
		Uint32 bias_sizes[] = { 1, filter_sizes[0], 1, 1 };
		std::vector<Uint16> compressed_bias_weights = CompressBiasWeights(shift_weights, filter_sizes);

		Uint32 bias_strides[4];
		GetTensorStrides(bias_sizes, bias_strides);
		Uint64 bias_buffer_size = DMLCalcBufferTensorSize(DML_TENSOR_DATA_TYPE_FLOAT16, 4, bias_sizes, bias_strides);

		GfxBufferDesc bias_tensor_desc{};
		bias_tensor_desc.bind_flags = GfxBindFlag::ShaderResource | GfxBindFlag::UnorderedAccess;
		bias_tensor_desc.size = bias_buffer_size;
		bias_tensor_desc.resource_usage = GfxResourceUsage::Default;
		GfxBufferData gfx_bias_data(compressed_bias_weights.data());
		return gfx->CreateBuffer(bias_tensor_desc, gfx_bias_data);
	}

	std::vector<Uint16> DirectMLPassBase::CompressFilterWeights(std::vector<Float> const& filter_weights, std::vector<Float> const& scale_weights, std::span<const Uint32> sizes)
	{
		std::vector<Uint16> result;
		Uint32 N = sizes[0], C = sizes[1], H = sizes[2], W = sizes[3];
		Uint64 total_size = N * C * H * W;
		Bool const use_scale_shift = !scale_weights.empty();
		result.reserve(total_size);
		for (Uint32 n = 0; n < N; ++n)
		{
			if (tensor_layout == TensorLayout::Default)
			{
				for (Uint32 i = 0; i < C * H * W; ++i)
				{
					Uint32 idx = n * C * H * W + i;
					Float scaled_weight = use_scale_shift ? filter_weights[idx] * scale_weights[n] : filter_weights[idx];
					result.push_back(FloatCompressor::Compress(scaled_weight));
				}
			}
			else
			{
				for (Uint32 h = 0; h < H; h++)
				{
					for (Uint32 w = 0; w < W; w++)
					{
						for (Uint32 c = 0; c < C; c++)
						{
							Uint32 idx = w + h * W + c * H * W + n * C * H * W;
							Float scaled_weight = use_scale_shift ? filter_weights[idx] * scale_weights[n] : filter_weights[idx];
							result.push_back(FloatCompressor::Compress(scaled_weight));
						}
					}
				}
			}
		}
		return result;
	}

	std::vector<Uint16> DirectMLPassBase::CompressBiasWeights(std::vector<Float> const& shift_weights, std::span<const Uint32> sizes)
	{
		std::vector<Uint16> result;
		Uint32 N = sizes[0], C = sizes[1], H = sizes[2], W = sizes[3];
		Uint64 total_size = N * C * H * W;
		result.reserve(N);
		for (Uint32 n = 0; n < N; ++n)
		{
			result.push_back(FloatCompressor::Compress(shift_weights[n]));
		}
		return result;
	}

}