#pragma once

#include <d3d11.h>
#include <string>
#include <vector>
#include "Core/EngineTypes.h"

namespace Engine
{
    class Shader;

    class Material
    {
    public:
        bool Create(ID3D11Device* device, Shader* shader, const Color& color);
        bool CreateTexture(ID3D11Device* device, const unsigned int* pixels, int width, int height);
        bool LoadTextureFromFile(ID3D11Device* device, const std::wstring& path);
        void Bind(ID3D11DeviceContext* context);
        void Release();

        void SetColor(const Color& value) { tint = value; }

        // 스프라이트 시트용 UV 설정
        // frameU, frameV : 시트 내 프레임 시작 위치 (0~1 정규화)
        // scaleU, scaleV : 프레임 1장의 UV 크기 (0~1 정규화)
        void SetSpriteUV(float frameU, float frameV, float scaleU, float scaleV)
        {
            uvOffsetU = frameU;
            uvOffsetV = frameV;
            uvScaleU = scaleU;
            uvScaleV = scaleV;
            useSprite = true;
        }
        void DisableSprite() { useSprite = false; }

    private:
        // cbuffer 레이아웃 — BasicColor.hlsl MaterialBuffer 와 1:1 일치해야 함
        // 총 48 bytes (16의 배수)
        struct ColorBuffer
        {
            float color[4];     // tintColor      16 bytes
            float useTexture;   //                 4 bytes
            float useSprite;    //                 4 bytes
            float uvOffsetU;    //                 4 bytes
            float uvOffsetV;    //                 4 bytes
            float uvScaleU;     //                 4 bytes
            float uvScaleV;     //                 4 bytes
            float padding[2];   //                 8 bytes  → 합계 48 bytes
        };

        Shader* shader = nullptr;
        Color                      tint;
        ID3D11Buffer* colorBuffer = nullptr;
        ID3D11ShaderResourceView* textureView = nullptr;
        ID3D11SamplerState* samplerState = nullptr;
        bool                       hasTexture = false;

        bool   useSprite = false;
        float  uvOffsetU = 0.0f;
        float  uvOffsetV = 0.0f;
        float  uvScaleU = 1.0f;
        float  uvScaleV = 1.0f;
    };
}