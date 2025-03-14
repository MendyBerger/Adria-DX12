#include "CommonResources.hlsli"
#include "Constants.hlsli"

#define BLOCK_SIZE 16

static const float g = 9.81f;
static const float KM = 370.0f;
static const float CM = 0.23f;


float Omega(float k)
{
	return sqrt(g * k * (1.f + ((k * k) / (KM * KM))));
}
float Square(float x)
{
	return x * x;
}
float2 MultiplyComplex(float2 a, float2 b)
{
	return float2(a.x * b.x - a.y * b.y, a.y * b.x + a.x * b.y);
}
float2 MultiplyByI(float2 z)
{
	return float2(-z.y, z.x);
}

struct SpectrumConstants
{
	float fftResolution;
	float oceanSize;
	float oceanChoppiness;

	uint  initialSpectrumIdx;
	uint  phasesIdx;
	uint  outputIdx;
};
ConstantBuffer<SpectrumConstants> SpectrumPassCB : register(b1);

struct CSInput
{
	uint3 GroupId : SV_GroupID;
	uint3 GroupThreadId : SV_GroupThreadID;
	uint3 DispatchThreadId : SV_DispatchThreadID;
	uint  GroupIndex : SV_GroupIndex;
};

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void SpectrumCS(CSInput input)
{
	Texture2D<float4> phasesTexture = ResourceDescriptorHeap[SpectrumPassCB.phasesIdx];
	Texture2D<float4> initialSpectrumTexture = ResourceDescriptorHeap[SpectrumPassCB.initialSpectrumIdx];
	RWTexture2D<float4> spectrumTexture = ResourceDescriptorHeap[SpectrumPassCB.outputIdx];

	uint2 pixelCoord = input.DispatchThreadId.xy;
	float n = (pixelCoord.x < 0.5f * uint(SpectrumPassCB.fftResolution)) ? pixelCoord.x : pixelCoord.x - SpectrumPassCB.fftResolution;
	float m = (pixelCoord.y < 0.5f * uint(SpectrumPassCB.fftResolution)) ? pixelCoord.y : pixelCoord.y - SpectrumPassCB.fftResolution;
	float2 waveVector = (2.f * M_PI * float2(n, m)) / SpectrumPassCB.oceanSize;

	float phase = phasesTexture.Load(uint3(pixelCoord, 0)).r;
	float2 phaseVector = float2(cos(phase), sin(phase));
	uint2 coords = (uint2(SpectrumPassCB.fftResolution, SpectrumPassCB.fftResolution) - pixelCoord) % (SpectrumPassCB.fftResolution - 1);

	float2 h0 = float2(initialSpectrumTexture.Load(uint3(pixelCoord, 0)).r, 0.f);
	float2 h0Star = float2(initialSpectrumTexture.Load(uint3(coords, 0)).r, 0.f);
	h0Star.y *= -1.f;

	float2 h = MultiplyComplex(h0, phaseVector) + MultiplyComplex(h0Star, float2(phaseVector.x, -phaseVector.y));
	float2 hX = -MultiplyByI(h * (waveVector.x / length(waveVector))) * SpectrumPassCB.oceanChoppiness;
	float2 hZ = -MultiplyByI(h * (waveVector.y / length(waveVector))) * SpectrumPassCB.oceanChoppiness;

	// No DC term
	if (waveVector.x == 0.0 && waveVector.y == 0.0)
	{
		h = float2(0.f, 0.f);
		hX = float2(0.f, 0.f);
		hZ = float2(0.f, 0.f);
	}
	spectrumTexture[pixelCoord] = float4(hX + MultiplyByI(h), hZ);
}