#define THREAD_GROUP_SIZE 16

struct TensorTextureConversionsConstants
{
    float2 resolution;   
    bool   nhwc;             
    uint   outputIdx;
    uint   inputIdx;
};
ConstantBuffer<TensorTextureConversionsConstants> TensorTextureConversionsPassCB : register(b1);

void ConvertRGB(uint3 DTid, float2 outputResolution, RWTexture2D<float4> outputTexture, RWBuffer<half> inputTensor)
{
    uint index = DTid.y * (uint)outputResolution.x + DTid.x;
    float4 color;
    
    if (TensorTextureConversionsPassCB.nhwc)
    {
        color.r = inputTensor[index * 3];
        color.g = inputTensor[index * 3 + 1];
        color.b = inputTensor[index * 3 + 2];
        color.a = 1.0f;
    }
    else
    {
        uint blockSize = (uint)outputResolution.x * (uint)outputResolution.y;
        color.r = inputTensor[index];
        color.g = inputTensor[index + blockSize];
        color.b = inputTensor[index + 2 * blockSize];
        color.a = 1.0f;
    }
    
    outputTexture[DTid.xy] = color;
}

// BGR conversion
void ConvertBGR(uint3 DTid, float2 outputResolution, RWTexture2D<float4> outputTexture, RWBuffer<half> inputTensor)
{
    uint index = DTid.y * (uint)outputResolution.x + DTid.x;
    float4 color;
    
    if (TensorTextureConversionsPassCB.nhwc)
    {
        color.b = inputTensor[index * 3];
        color.g = inputTensor[index * 3 + 1];
        color.r = inputTensor[index * 3 + 2];
        color.a = 1.0f;
    }
    else
    {
        uint blockSize = (uint)outputResolution.x * (uint)outputResolution.y;
        color.b = inputTensor[index];
        color.g = inputTensor[index + blockSize];
        color.r = inputTensor[index + 2 * blockSize];
        color.a = 1.0f;
    }
    
    outputTexture[DTid.xy] = color;
}

// Grayscale conversion
void ConvertGRAY(uint3 DTid, float2 outputResolution, RWTexture2D<float4> outputTexture, RWBuffer<half> inputTensor)
{
    uint index = DTid.y * (uint)outputResolution.x + DTid.x;
    float4 color;
    
    color.b = inputTensor[index];
    color.g = color.b;
    color.r = color.b;
    color.a = 1.0f;
    
    outputTexture[DTid.xy] = color;
}

[numthreads(THREAD_GROUP_SIZE, THREAD_GROUP_SIZE, 1)]
void TensorToTextureCS(uint3 DTid : SV_DispatchThreadID)
{
    const float2 outputResolution = TensorTextureConversionsPassCB.resolution;
    if (DTid.x >= outputResolution.x || DTid.y >= outputResolution.y)
        return;

    RWTexture2D<float4> outputTexture = ResourceDescriptorHeap[TensorTextureConversionsPassCB.outputIdx];
    RWBuffer<half> inputTensor = ResourceDescriptorHeap[TensorTextureConversionsPassCB.inputIdx];

    ConvertRGB(DTid, outputResolution, outputTexture, inputTensor);
    // ConvertBGR(DTid, outputResolution, outputTexture, inputTensor);
    // ConvertGRAY(DTid, outputResolution, outputTexture, inputTensor);
}

[numthreads(THREAD_GROUP_SIZE, THREAD_GROUP_SIZE, 1)]
void TextureToTensorCS( uint3 DTid : SV_DispatchThreadID )
{
    const float2 inputResolution = TensorTextureConversionsPassCB.resolution;
    if (DTid.x >= inputResolution.x || DTid.y >= inputResolution.y)
        return;

    Texture2D<float4> inputTexture  = ResourceDescriptorHeap[TensorTextureConversionsPassCB.inputIdx];
    RWBuffer<half> outputTensor = ResourceDescriptorHeap[TensorTextureConversionsPassCB.outputIdx];

    uint index = DTid.y * inputResolution.x + DTid.x;
    float3 val = inputTexture[DTid.xy].xyz;
    if (TensorTextureConversionsPassCB.nhwc)
    {
        outputTensor[index * 3 + 0] = val.x;
        outputTensor[index * 3 + 1] = val.y;
        outputTensor[index * 3 + 2] = val.z;
    }
    else
    {
        uint planeSize = inputResolution.x * inputResolution.y;
        outputTensor[index] = val.x;
        outputTensor[index + planeSize] = val.y;
        outputTensor[index + planeSize * 2] = val.z;
    }
}

