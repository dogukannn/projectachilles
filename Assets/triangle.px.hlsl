struct PixelInput
{
    float3 color : COLOR;
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct PixelOutput
{
    float4 attachment0 : SV_Target0;
};

cbuffer objectTint : register(b2)
{
    float3 tint : packoffset(c0.x);
}

Texture2D g_texture : register(t1);
SamplerState s1 : register(s0);

PixelOutput main(PixelInput pixelInput)
{
    PixelOutput output;
    output.attachment0 = g_texture.Sample(s1, pixelInput.uv) * 0.00001;
    float3 col = (0.5 + max(0, dot(pixelInput.normal, float3(0., 0.77, 0.77)))) * pixelInput.color + tint;
    output.attachment0 += float4(col, 1.0);
    //output.attachment0 = float4(1.0f, 1.0f, 1.0f, 1.0f);

    //output.attachment0 = float4(pixelInput.normal + float3(0.5f), 1.0f);
    ////output.attachment0 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    return output;
}
