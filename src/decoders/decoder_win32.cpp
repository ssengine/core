#include "decoder.h"

#include <windows.h>
#include <wincodec.h>
#include <assert.h>

#include "ssengine/log.h"

struct ss_image_decoder {
	IWICImagingFactory *factory;
	IWICBitmapDecoder *decoder;
	IWICBitmapFrameDecode *frame;
};

static ss_image_decoder *s_decoder = nullptr;

void ss_init_decoder() {
	if (s_decoder == nullptr) {
		s_decoder = new ss_image_decoder();
		
		HRESULT hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&s_decoder->factory)
			);
		if (hr != S_OK) {
			SS_WLOGE(L"Failed to load WIC");
			exit(1);
		}
	}
}

void ss_dispose_decoder() {
	if (s_decoder != nullptr) {
		if (s_decoder->factory != nullptr)
			s_decoder->factory->Release();
		if (s_decoder->decoder != nullptr)
			s_decoder->decoder->Release();
		if (s_decoder->frame != nullptr)
			s_decoder->frame->Release();
		delete s_decoder;
		s_decoder = nullptr;
	}
}

static unsigned int _get_image_stride(unsigned int width, unsigned int bpp) {
	assert(bpp % 8 == 0);

	unsigned int byteCount = bpp / 8;
	unsigned int stride = (width * byteCount + 3) & ~3;

	assert(stride % sizeof(DWORD) == 0);
	return stride;
}

//TODO : need a relative path !
int ss_decode_image(const char *path, ss_image_data *data) {
	if (s_decoder == nullptr) {
		SS_WLOGE(L"you must call ss_init_decoder() method first!");
		return -1;
	}

	wchar_t *wpath = char2wchar_t(path);
	HRESULT hr = s_decoder->factory->CreateDecoderFromFilename(
			wpath,
			nullptr,
			GENERIC_READ,
			WICDecodeMetadataCacheOnDemand,
			&s_decoder->decoder
		);
	if (hr != S_OK) {
		SS_WLOGE(L"WIC create decoder failed from the file:[%s]", wpath);
		return -1;
	}

	if (s_decoder->frame != nullptr)
		s_decoder->frame->Release();

	hr = s_decoder->decoder->GetFrame(0, &s_decoder->frame);
	if (hr != S_OK) {
		SS_WLOGE(L"WIC decode image failed! the filename is:[%s]", wpath);
		return -1;
	}

	s_decoder->frame->GetSize(&data->width, &data->height);
	GUID pixel_format = { 0 };
	s_decoder->frame->GetPixelFormat(&pixel_format);

	//TODO ss_render_format is not enough !!
	unsigned int stride = 0;
	if (IsEqualGUID(pixel_format, GUID_WICPixelFormat32bppRGBA)) {
		data->format = SS_FORMAT_BYTE_RGBA;
		stride = _get_image_stride(data->width, 32);
	}
	else if (IsEqualGUID(pixel_format, GUID_WICPixelFormat32bppBGRA)) {
		data->format = SS_FORMAT_BYTE_BGRA;
		stride = _get_image_stride(data->width, 32);
	}
	else if (IsEqualGUID(pixel_format, GUID_WICPixelFormat32bppBGR)) {
		data->format = SS_FORMAT_BYTE_RGBA;
		stride = _get_image_stride(data->width, 32);
	}
	else if (IsEqualGUID(pixel_format, GUID_WICPixelFormat24bppRGB)) {
		//TODO ss_render_format should support RGB.
		stride = _get_image_stride(data->width, 24);
	}
	else {
		data->format = SS_FORMAT_NULL;
		SS_WLOGE(L"WIC Unsupport image format! the filename is:[%s]", wpath);
		return -1;
	}

	//copy pixels.
	unsigned int size = stride * data->height;
	if (data->data != nullptr)
		delete[] data->data;
	data->data = new unsigned char[size];
	hr = s_decoder->frame->CopyPixels(
			nullptr,
			stride,
			size,
			data->data
		);
	if (hr != S_OK) {
		SS_WLOGE(L"WIC copyPixels error! the filename is:[%s]", wpath);
		return -1;
	}

	return 0;
}