#pragma once

#include "ssengine/core.h"
#include "ssengine/render/device.h"
#include "ssengine/render/resources.h"
#include "../core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ss_userdata_image {
	ss_texture_resource_ref *pointer;
	unsigned int width;
	unsigned int height;
	enum ss_render_format format;
	unsigned char *pixels;

	void init() {
		pointer = nullptr;
		width = height = 0;
		format = SS_FORMAT_NULL;
		pixels = nullptr;
	}

	void gen_texture(ss_core_context *C) {
		ss_render_device *device = ss_get_render_device(C);
		ss_texture2d *texture = device->create_texture2d(width, height, format, pixels);
		pointer->set(texture);
		delete[] pixels;
		pixels = nullptr;
	}
} ss_userdata_image;

SS_CORE_API void ss_init_decoder();
SS_CORE_API void ss_dispose_decoder();
SS_CORE_API int ss_decode_image(const char *path, ss_userdata_image *data);

#ifdef __cplusplus
}
#endif