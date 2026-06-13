#include "Graphics/Material.h"
#include "Graphics/Shader.h"
#include <fstream>
#include <vector>
#include <cstring>
#include <wincodec.h>   // WIC (Windows Imaging Component)
#pragma comment(lib, "windowscodecs.lib")

namespace Engine
{
    bool Material::Create(ID3D11Device* device, Shader* shaderRef, const Color& color)
    {
        shader = shaderRef;
        tint = color;

        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ByteWidth = sizeof(ColorBuffer);
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        return SUCCEEDED(device->CreateBuffer(&desc, nullptr, &colorBuffer));
    }

    bool Material::CreateTexture(ID3D11Device* device, const unsigned int* pixels, int width, int height)
    {
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = pixels;
        initData.SysMemPitch = width * sizeof(unsigned int);

        ID3D11Texture2D* texture = nullptr;
        HRESULT hr = device->CreateTexture2D(&textureDesc, &initData, &texture);
        if (FAILED(hr)) return false;

        hr = device->CreateShaderResourceView(texture, nullptr, &textureView);
        texture->Release();
        if (FAILED(hr)) return false;

        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // 스프라이트는 Linear
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

        hr = device->CreateSamplerState(&samplerDesc, &samplerState);
        hasTexture = SUCCEEDED(hr);
        return hasTexture;
    }

    // PNG 파일을 직접 로딩 — Windows GDI+ 없이 순수 PNG 헤더 파싱
    // 간단한 구현: stb_image 스타일의 직접 파싱 대신
    // WIC(Windows Imaging Component)를 사용
    bool Material::LoadTextureFromFile(ID3D11Device* device, const std::wstring& path)
    {
        // WIC로 PNG 로딩
        IWICImagingFactory* wicFactory = nullptr;
        IWICBitmapDecoder* decoder = nullptr;
        IWICBitmapFrameDecode* frame = nullptr;
        IWICFormatConverter* converter = nullptr;

        HRESULT hr = CoCreateInstance(
            CLSID_WICImagingFactory, nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&wicFactory));
        if (FAILED(hr)) return false;

        hr = wicFactory->CreateDecoderFromFilename(
            path.c_str(), nullptr,
            GENERIC_READ,
            WICDecodeMetadataCacheOnLoad,
            &decoder);
        if (FAILED(hr)) { wicFactory->Release(); return false; }

        hr = decoder->GetFrame(0, &frame);
        if (FAILED(hr)) { decoder->Release(); wicFactory->Release(); return false; }

        hr = wicFactory->CreateFormatConverter(&converter);
        if (FAILED(hr)) { frame->Release(); decoder->Release(); wicFactory->Release(); return false; }

        hr = converter->Initialize(
            frame,
            GUID_WICPixelFormat32bppRGBA,
            WICBitmapDitherTypeNone, nullptr, 0.0,
            WICBitmapPaletteTypeCustom);
        if (FAILED(hr)) { converter->Release(); frame->Release(); decoder->Release(); wicFactory->Release(); return false; }

        UINT imgWidth = 0, imgHeight = 0;
        converter->GetSize(&imgWidth, &imgHeight);

        std::vector<BYTE> pixels(imgWidth * imgHeight * 4);
        hr = converter->CopyPixels(nullptr, imgWidth * 4, static_cast<UINT>(pixels.size()), pixels.data());

        converter->Release();
        frame->Release();
        decoder->Release();
        wicFactory->Release();

        if (FAILED(hr)) return false;

        // RGBA → DirectX RGBA (순서 동일하므로 그대로 업로드)
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = imgWidth;
        textureDesc.Height = imgHeight;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = pixels.data();
        initData.SysMemPitch = imgWidth * 4;

        ID3D11Texture2D* texture = nullptr;
        hr = device->CreateTexture2D(&textureDesc, &initData, &texture);
        if (FAILED(hr)) return false;

        hr = device->CreateShaderResourceView(texture, nullptr, &textureView);
        texture->Release();
        if (FAILED(hr)) return false;

        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

        hr = device->CreateSamplerState(&samplerDesc, &samplerState);
        hasTexture = SUCCEEDED(hr);
        return hasTexture;
    }

    void Material::Bind(ID3D11DeviceContext* context)
    {
        shader->Bind(context);

        ColorBuffer data = {};
        data.color[0] = tint.r;
        data.color[1] = tint.g;
        data.color[2] = tint.b;
        data.color[3] = tint.a;
        data.useTexture = hasTexture ? 1.0f : 0.0f;
        data.useSprite = useSprite ? 1.0f : 0.0f;
        data.uvOffsetU = uvOffsetU;
        data.uvOffsetV = uvOffsetV;
        data.uvScaleU = uvScaleU;
        data.uvScaleV = uvScaleV;

        context->UpdateSubresource(colorBuffer, 0, nullptr, &data, 0, 0);
        context->PSSetConstantBuffers(1, 1, &colorBuffer);

        if (hasTexture)
        {
            context->PSSetShaderResources(0, 1, &textureView);
            context->PSSetSamplers(0, 1, &samplerState);
        }
    }

    void Material::Release()
    {
        if (colorBuffer) { colorBuffer->Release();  colorBuffer = nullptr; }
        if (textureView) { textureView->Release();  textureView = nullptr; }
        if (samplerState) { samplerState->Release(); samplerState = nullptr; }
        hasTexture = false;
    }
}