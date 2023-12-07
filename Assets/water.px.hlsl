struct PixelInput
{
    float3 color : COLOR;
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float height : TEXCOORD2;
};

struct PixelOutput
{
    float4 attachment0 : SV_Target0;
};

cbuffer objectTint : register(b2)
{
    float3 tint : packoffset(c0.x);
}

PixelOutput main(PixelInput pixelInput)
{
    PixelOutput output;
    //output.attachment0 = g_texture.Sample(s1, pixelInput.uv) * 0.00001;
	output.attachment0 = float4(0,0,0,0);
    //float3 col = (0.5 + max(0, dot(pixelInput.normal, float3(0., 0.77, 0.77)))) * pixelInput.color + tint;
    float3 blue = float3(0.161, 0.329, 0.678);
    float3 cyan = float3(0.467, 0.824, 0.91);
    float3 col = blue + (cyan * (pixelInput.height + 3.0f) / 6.0f);
    //col = float3(1.0f, 1.0f, 1.0f);
    col += (tint * 0.00001);
    output.attachment0 += float4(col, 1.0);
    //output.attachment0 = float4(1.0f, 1.0f, 1.0f, 1.0f);

    //output.attachment0 = float4(pixelInput.normal + float3(0.5f), 1.0f);
    ////output.attachment0 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    return output;
}
