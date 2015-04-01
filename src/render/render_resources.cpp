#include <ssengine/resource.h>
#include <ssengine/core.h>
#include <ssengine/render/resources.h>
#include <ssengine/render/device.h>

#include "../decoders/decoder.h"

#include "../core.h"

static ss_resource_type texture2d_resource_type_tag;

static int (texture2d_sync_load)(ss_core_context* C, ss_resource_ref* ref){
    ss_render_device* device = ss_get_render_device(C);

    ss_resource_ref_impl* ref_impl = wrap(ref);
    ss_image_data data;
    int ret = load_image(C, ref_impl->uri.c_str(), &data);
    if (ret != 0){
        return ret;
    }

    ref->ptr = device->create_texture2d(data.width, data.height, data.format, data.pixels);

    return 0;
}

static void (texture2d_unload)(ss_core_context* C, ss_resource_ref* ref){
	if (ref->ptr){
		delete reinterpret_cast<ss_texture2d*>(ref->ptr);
		ref->ptr = NULL;
	}
}

static ss_resource_prototype  texture2d_prototype = {
	&texture2d_resource_type_tag,
    texture2d_sync_load,
	NULL,
	texture2d_unload
};


ss_resource_ref* ss_texture2d_resource(ss_core_context* C, const char* uri){
	ss_resource_ref* ret;
	ret = ss_resource_from_uri(C, uri);
	if (ret){
        return ret;
	}
    ret = ss_resource_create(C, &texture2d_prototype, uri, SS_DT_RENDER);
    return ret;
}