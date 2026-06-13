cbuffer TransformBuffer : register(b0)
{
    matrix world;
};

cbuffer MaterialBuffer : register(b1)
{
    float4 tintColor;
    float useTexture;
    float useSprite; // 1이면 스프라이트 시트 UV 오프셋 적용
    float2 uvOffset; // (frameU, rowV) - 프레임 위치
    float2 uvScale; // (frameWidth, frameHeight) - 프레임 크기(0~1 정규화)
    float2 padding;
};

Texture2D diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);

struct VS_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    output.position = mul(float4(input.position, 1.0f), world);
    output.color = input.color;
    output.uv = input.uv;
    return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
    float2 uv = input.uv;

    // 스프라이트 시트 모드: UV를 프레임 범위로 변환
    if (useSprite > 0.5f)
    {
        uv = uvOffset + input.uv * uvScale;
    }

    float4 textureColor = diffuseTexture.Sample(diffuseSampler, uv);
    float4 baseColor = lerp(input.color, textureColor, useTexture);
    float4 result = baseColor * tintColor;

    // 알파 0에 가까운 픽셀 완전 투명 처리 (배경 잔재 제거)
    if (useSprite > 0.5f && result.a < 0.05f)
    {
        discard;
    }

    return result;
}
