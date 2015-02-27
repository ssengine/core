#include <ssengine/resource.h>
#include <ssengine/core.h>
#include <ssengine/render/resources.h>
#include <ssengine/render/device.h>

static ss_resource_type texture_resource_type_tag;

#ifdef WIN32
void load_image(){

}
#else
#endif

static int (texture_sync_load)(ss_core_context* C, ss_resource_ref* ref){
	return -1;
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