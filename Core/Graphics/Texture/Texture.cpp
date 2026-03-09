#include "Texture.h"
#include <iostream>
#include <combaseapi.h>
#include <wincodec.h>
#include <d3d11.h>
#include <vector>
#include "Graphics/Dx11/Dx11Renderer.h"
//VULKAN WHERE ARE YOU
#if DIRECTX11
ID3D11ShaderResourceView* Texture::Load(std::string path, Dx11Renderer& dx11Renderer)
{
    if (path.empty()) {
        MakeAError("Texture path is empty!");
        return nullptr;
    }

    if (!dx11Renderer.GetDevice()) {
        MakeAError("Device is null!");
        return nullptr;
    }

    std::filesystem::path fsPath(path);

    if (!std::filesystem::exists(path)) {
        MakeAError("Texture Doesnt Exist: " + path);
        return nullptr;
    }

    std::string ext = fsPath.extension().string();
    if (ext != ".png" && ext != ".jpg" && ext != ".jpeg" && ext != ".bmp" && ext != ".dds") {
        MakeAError("Unsupported texture format: " + ext);
        return nullptr;
    }

    std::wstring wpath(path.begin(), path.end());

    IWICImagingFactory* factory = nullptr;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&factory)
    );

    if (FAILED(hr) || !factory) {
        MakeAError("Failed to create WIC factory");
        return nullptr;
    }

    IWICBitmapDecoder* decoder = nullptr;
    hr = factory->CreateDecoderFromFilename(
        wpath.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &decoder
    );

    if (FAILED(hr) || !decoder) {
        MakeAError("Failed to create decoder for: " + path);
        factory->Release();
        return nullptr;
    }

    IWICBitmapFrameDecode* frame = nullptr;
    hr = decoder->GetFrame(0, &frame);

    if (FAILED(hr) || !frame) {
        MakeAError("Failed to get frame");
        decoder->Release();
        factory->Release();
        return nullptr;
    }

    IWICFormatConverter* converter = nullptr;
    hr = factory->CreateFormatConverter(&converter);

    if (FAILED(hr) || !converter) {
        MakeAError("Failed to create converter");
        frame->Release();
        decoder->Release();
        factory->Release();
        return nullptr;
    }

    hr = converter->Initialize(
        frame,
        GUID_WICPixelFormat32bppRGBA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom
    );

    if (FAILED(hr)) {
        MakeAError("Failed to initialize converter");
        converter->Release();
        frame->Release();
        decoder->Release();
        factory->Release();
        return nullptr;
    }

    UINT width, height;
    hr = frame->GetSize(&width, &height);

    if (FAILED(hr) || width == 0 || height == 0) {
        MakeAError("Invalid texture dimensions");
        converter->Release();
        frame->Release();
        decoder->Release();
        factory->Release();
        return nullptr;
    }

    std::vector<BYTE> pixels(width * height * 4);

    hr = converter->CopyPixels(
        nullptr,
        width * 4,
        pixels.size(),
        pixels.data()
    );

    if (FAILED(hr)) {
        MakeAError("Failed to copy pixels");
        converter->Release();
        frame->Release();
        decoder->Release();
        factory->Release();
        return nullptr;
    }

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem = pixels.data();
    data.SysMemPitch = width * 4;
    data.SysMemSlicePitch = 0;

    ID3D11Texture2D* texture = nullptr;

    UGE_ASSERT(dx11Renderer.GetDevice(), "Device Cant Be nullptr");
    hr = dx11Renderer.GetDevice()->CreateTexture2D(&desc, &data, &texture);

    if (FAILED(hr) || !texture) {
        MakeAError("Failed to create texture");
        converter->Release();
        frame->Release();
        decoder->Release();
        factory->Release();
        return nullptr;
    }

    ID3D11ShaderResourceView* srv = nullptr;
    hr = dx11Renderer.GetDevice()->CreateShaderResourceView(texture, nullptr, &srv);

    texture->Release();

    if (FAILED(hr) || !srv) {
        MakeAError("Failed to create shader resource view");
        converter->Release();
        frame->Release();
        decoder->Release();
        factory->Release();
        return nullptr;
    }

    pTexture = srv;

    converter->Release();
    frame->Release();
    decoder->Release();
    factory->Release();

    Loaded = true;
    return srv;
}
#endif