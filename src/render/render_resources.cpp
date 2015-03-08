#include <ssengine/resource.h>
#include <ssengine/core.h>
#include <ssengine/render/resources.h>
#include <ssengine/render/device.h>

#include "../core.h"
#include "../decoders/decoder.h"

static ss_resource_type texture_resource_type_tag;

#ifdef WIN32
int load_image(ss_core_context *C, ss_resource_ref *ref) {
	ss_resource_ref_impl *ref_impl = wrap(ref);
	const char *uri = ref_impl->uri.c_str();
	ss_image_data *img_data = reinterpret_cast<ss_image_data*>(ref->extra_info_ptr);

	ss_init_decoder();

	return ss_decode_image(uri, img_data);
}
#elif ANDROID
int load_image(ss_core_context *C, ss_resource_ref *ref) {
	//TODO
}
#elif IOS
int load_image(ss_core_context *C, ss_resource_ref *ref) {
	//TODO
}
#endif

static int (texture_sync_load)(ss_core_context* C, ss_resource_ref* ref){
	return load_image(C, ref);
}

static void (texture_unload)(ss_core_context* C, ss_resource_ref* ref){
	if (ref->ptr){
		delete reinterpret_cast<ss_texture*>(ref->ptr);
		ref->ptr = NULL;
	}
}

static ss_resource_prototype  texture_prototype = {
	&texture_resource_type_tag,
	texture_sync_load,
	NULL,
	texture_unload
};


ss_texture_resource_ref* ss_texture_resource(ss_core_context* C, const char* uri){
	ss_resource_ref* ret;
	ret = ss_resource_from_uri(C, uri);
	if (ret){
		return ss_texture_resource_ref::wrap(ret);
	}
	ret = ss_resource_create(C, &texture_prototype, uri, SS_DT_RENDER);
	return ss_texture_resource_ref::wrap(ret);
}