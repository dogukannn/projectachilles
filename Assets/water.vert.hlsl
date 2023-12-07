cbuffer scene : register(b0)
{
    row_major float4x4 vp : packoffset(c0);
    float3 eye : packoffset(c4.x);
    float time : packoffset(c4.w);
}

cbuffer object : register(b1)
{
    row_major float4x4 model : packoffset(c0);
};

struct VertexInput
{
    float3 inPos : POSITION;
    float3 inNormal : NORMAL;
    float3 inColor : COLOR;
    float2 inUV : TEXCOORD;
};

struct VertexOutput
{
    float3 color : COLOR;
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float height : TEXCOORD2;
};

float rand(in float2 uv)
{
    float2 noise = (frac(sin(dot(uv ,float2(12.9898,78.233)*2.0)) * 43758.5453));
    return abs(noise.x + noise.y) * 0.5;
}

struct WaveConstants
{
    float speed;
    float2 dir;
    float frequency;
    float amplitude;
    float exponent;
    float steepness;
};

static WaveConstants waveConstantConstructor(float speed, float2 dir, float frequency, float amplitude, float exponent, float steepness)
{
    WaveConstants wc;
    wc.speed = speed;
    wc.dir = dir;
    wc.frequency = frequency;
    wc.amplitude = amplitude;
    wc.exponent = exponent;
    wc.steepness = steepness;
    return wc;
}

float3 GerstnerWavePoint(float3 inPoint, float time, WaveConstants waveConstants[4])
{
    float3 res = float3(0,inPoint.y,0);
    for (int i = 0; i < 4; i++)
    {
        res.x += waveConstants[i].steepness * waveConstants[i].amplitude * waveConstants[i].dir.x * cos((time * waveConstants[i].speed) + (dot(waveConstants[i].dir,  float2(inPoint.x, inPoint.z)) * waveConstants[i].frequency));
        res.z += waveConstants[i].steepness * waveConstants[i].amplitude * waveConstants[i].dir.y * cos((time * waveConstants[i].speed) + (dot(waveConstants[i].dir,  float2(inPoint.x, inPoint.z)) * waveConstants[i].frequency));
        res.y += waveConstants[i].amplitude * sin((time * waveConstants[i].speed) + (dot(waveConstants[i].dir,  float2(inPoint.x, inPoint.z)) * waveConstants[i].frequency));
    }
    res.x += inPoint.x;
    res.z += inPoint.z;
    return res;
}

float wave(float time, float speed, float2 pos, float2 dir, float frequency, float amplitude, float exponent)
{
    return 2.0f * amplitude * pow((sin((time * speed) + (dot(dir, pos) * frequency)) + 1.0f) / 2.0f, exponent);
}



VertexOutput main(VertexInput vertexInput)
{
    float3 inColor = vertexInput.inColor;
    float3 inPos = vertexInput.inPos;
    //float2 waveDir = float2(1.5, 1.);

    //inPos.y += wave(time,4.5f, float2(inPos.x, inPos.z), float2(1, 1.32), 1.0f, 0.1f, 1.0f);
    //inPos.y += wave(time, 2.1f, float2(inPos.x, inPos.z), float2(1.233, 1), 0.5f, 0.2f, 1.5f);
    //inPos.y += wave(time, 1.1f, float2(inPos.x, inPos.z), float2(1, 0.1), 0.2f, 1.0f,2.0f);
    //inPos.y += wave(time, 2.1f, float2(inPos.x, inPos.z), float2(-1.9, 1), 0.5f, 0.02f, 3.4f);

    WaveConstants waveConstants[4];
    waveConstants[0] = waveConstantConstructor(4.5, float2(1.0, 1.32), 1.0, 0.1, 1.0, 2.0f);
    waveConstants[1] = waveConstantConstructor(2.1, float2(1.233, 1.0), 0.5, 0.2, 1.0, 1.4f);
    waveConstants[2] = waveConstantConstructor(1.1, float2(1, 1.01), 0.2, 1.0, 1.0, 1.0f);
    waveConstants[3] = waveConstantConstructor(2.1, float2(-1.9, 1.0), 0.5, 0.02, 1.0, 1.0f);

    float3 wavePos = GerstnerWavePoint(inPos, time, waveConstants);
    inPos = wavePos;

    float4 position = mul(float4(inPos, 1.0f), mul(model, vp));

    VertexOutput output;
    output.position = position;
    output.uv = vertexInput.inUV;
    //output.position = float4(inPos, 1.0f);
    output.color = inColor;
    output.normal = vertexInput.inNormal;
    output.height = inPos.y;
    return output;
}
