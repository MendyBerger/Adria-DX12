#include "GfxFence.h"
#include "GfxDevice.h"
#include "GfxCommandQueue.h"
#include "Utilities/StringUtil.h"

namespace adria
{

	GfxFence::GfxFence() {}

	GfxFence::~GfxFence()
	{
		CloseHandle(event);
	}

	Bool GfxFence::Create(GfxDevice* gfx, Char const* name)
	{
		ID3D12Device5* device = gfx->GetDevice();

		HRESULT hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));
		if (FAILED(hr)) return false;

		fence->SetName(ToWideString(name).c_str());
		event = CreateEvent(NULL, FALSE, FALSE, NULL);
		return true;
	}

	void GfxFence::Wait(Uint64 value)
	{
		if (!IsCompleted(value))
		{
			fence->SetEventOnCompletion(value, event);
			WaitForSingleObjectEx(event, INFINITE, FALSE);
		}
	}

	void GfxFence::Signal(Uint64 value)
	{
		fence->Signal(value);
	}

	Bool GfxFence::IsCompleted(Uint64 value)
	{
		return GetCompletedValue() >= value;
	}

	Uint64 GfxFence::GetCompletedValue() const
	{
		return fence->GetCompletedValue();
	}

}

