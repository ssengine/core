#pragma once

#include "ssengine/core.h"
#include "ssengine/render/device.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ss_image_data {
	ss_image_data()
	:width(0), height(0), format(SS_FORMAT_NULL),
	data(nullptr), texture(nullptr) {

	}

	~ss_image_data() {
		if (data != nullptr) {
			delete[] data;
		}
		if (texture != nullptr) {
			delete texture;
		}
	}

	void gen_texture(ss_core_context *C) {
		ss_render_device *device = ss_get_render_device(C);
		texture = device->create_texture2d(width, height, format, data);
	}

	unsigned int width;
	unsigned int height;
	enum ss_render_format format;
	unsigned char *data;
	ss_texture2d *texture; //for lua interface.
};
typedef struct ss_image_data ss_image_data;

SS_CORE_API void ss_init_decoder();
SS_CORE_API void ss_dispose_decoder();
SS_CORE_API int ss_decode_image(const char *path, ss_image_data *data);

#ifdef __cplusplus
}
#endif