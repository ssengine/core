#include <windows.h>
#include <wincodec.h>
#include <assert.h>

#include <ssengine/log.h>
#include <ssengine/uri.h>
#include <ssengine/render/device.h>
#include <atlcomcli.h>

#include <ssengine/details/win32/CStreamWrapper.h>

#include "decoder.h"

#include "../core.h"

struct ss_image_decoder {
	CComPtr<IWICImagingFactory> factory;
};

void _ss_init_image_decoder(ss_core_context* C) {
    if (C->image_decoder == nullptr) {
        C->image_decoder = new ss_image_decoder();
		
		HRESULT hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			nullptr,
			CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&C->image_decoder->factory)
			);
		if (hr != S_OK) {
			SS_WLOGE(L"Failed to load WIC");
			exit(1);
		}
	}
}

void _ss_dispose_image_decoder(ss_core_context* C) {
    if (C->image_decoder != nullptr) {
        delete C->image_decoder;
        C->image_decoder = nullptr;
	}
}

static unsigned int _get_image_stride(unsigned int width, unsigned int byteCount) {
	unsigned int stride = (width * byteCount + 3) & ~3;

	assert(stride % sizeof(DWORD) == 0);
	return stride;
}

int load_image(ss_core_context *C, const char *uri, ss_userdata_image *data) {
    CStreamWrapper wrapper(ss_uri_open_for_read(C, uri));

    CComPtr<IWICBitmapDecoder> decoder;

    HRESULT hr = C->image_decoder->factory->CreateDecoderFromStream(
            &wrapper,
            nullptr,
            WICDecodeMetadataCacheOnDemand,
            &decoder
            );

    if (hr != S_OK) {
        SS_LOGE("WIC create decoder failed from uri: [%s]", uri);
		return -1;
	}

    CComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);

    if (hr != S_OK) {
        SS_LOGE("WIC decode image failed! uri: [%s]", uri);
        return -1;
    }
    UINT width, height;
    frame->GetSize(&width, &height);

    GUID pixel_format = { 0 };
    frame->GetPixelFormat(&pixel_format);

    unsigned int stride = 0;
    ss_render_format format;

    if (IsEqualGUID(pixel_format, GUID_WICPixelFormat32bppRGBA)) {
        format = SS_FORMAT_BYTE_RGBA;
    }
    else if (IsEqualGUID(pixel_format, GUID_WICPixelFormat32bppBGRA)) {
        format = SS_FORMAT_BYTE_BGRA;
    }
    else {
        //TODO: tranlate format into known format.
        SS_LOGE("WIC Unsupport image format! uri: [%s]", uri);
        return -1;
    }
    stride = _get_image_stride(width, ss_render_format_sizeof(format));

    unsigned int size = stride * height;
    BYTE* buffer = new BYTE[size];
    hr = frame->CopyPixels(
        nullptr,
        stride,
        size,
        buffer
        );
    if (hr != S_OK) {
        delete[] buffer;
        SS_LOGE("WIC copyPixels error! uri: [%s]", uri);
        return -1;
    }

    data->width = width;
    data->height = height;
    data->format = format;
    data->pixels = buffer;

    return 0;
}
