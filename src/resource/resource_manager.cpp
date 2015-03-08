#include <ssengine/resource.h>
#include "../core.h"

#include <assert.h>
#include <vector>

ss_resource_ref* ss_resource_from_uri(ss_core_context* C, const char* uri)
{
	auto itor = C->resource_from_uris.find(uri);
	if (itor != C->resource_from_uris.end()){
		ss_resource_ref *ref = itor->second->ref();
		ss_resource_addref(C, ref);
		return ref;
	}
	return NULL;
}

ss_resource_ref* ss_resource_create(ss_core_context* C, const ss_resource_prototype* prototype, const char* uri, int device_type)
{
	assert(C->resource_from_uris.find(uri) == C->resource_from_uris.end());
	ss_resource_ref_impl* ret = new ss_resource_ref_impl(prototype, device_type, uri);
	C->resource_from_uris.insert(std::make_pair(std::string(uri), ret));

	return ret->ref();
}

void ss_resource_release(ss_core_context* C, ss_resource_ref* _ref){
	auto* ref = wrap(_ref);
	assert(ref->ref_count > 0);
	if (--ref->ref_count == 0){
		// "Loading" process will hold a reference.
		// So there's no need to check whether resource is loading.

		// Release loaded resource;
		if (ref->ptr){
			ss_resource_unload(C, _ref);
		}
		C->resource_from_uris.erase(ref->uri);

		delete ref;
	}
}

void ss_resource_addref(ss_core_context* C, ss_resource_ref* _ref){
	auto* ref = wrap(_ref);

	assert(ref->ref_count > 0);
	++ref->ref_count;
}

void ss_close_device(ss_core_context* C, int device_type)
{
	std::vector<ss_resource_ref_impl*> copies;
	{
		auto itor = C->resource_with_device_type.find(device_type);
		if (itor == C->resource_with_device_type.end()){
			return;
		}
		copies.insert(copies.begin(), itor->second.begin(), itor->second.end());
	}
	C->resource_with_device_type.erase(device_type);

	for (auto itor = copies.begin(); itor != copies.end(); ++itor)
	{
		// I should "close" a device instead of just unload them.
		ss_resource_unload(C, (*itor)->ref());
	}
}

void ss_open_device(ss_core_context* C, int device_type)
{
	if (C->resource_with_device_type.find(device_type) 
			== C->resource_with_device_type.end())
	{
		C->resource_with_device_type.insert(
			std::make_pair(device_type, std::set<ss_resource_ref_impl*>() )
			);
	}
}

bool ss_is_device_open(ss_core_context* C, int device_type)
{
	return C->resource_with_device_type.find(device_type)
		!= C->resource_with_device_type.end();
}

int ss_get_user_device_type(ss_core_context* C, const char* name)
{
	auto itor = C->user_defined_device_type.find(name);
	if (itor != C->user_defined_device_type.end()){
		return itor->second;
	}
	int id = C->user_defined_device_type.size() + SS_DT_USER;

	C->user_defined_device_type.insert(
		std::make_pair(std::string(name), id)
		);
	return id;
}