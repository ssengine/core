#include <ssengine/resource.h>
#include "../core.h"

//TODO: Implement async load.

int ss_resource_load(ss_core_context* C, ss_resource_ref* ref)
{
	if (ref->ptr){
		return 0;
	}

	if ((wrap(ref)->status & SS_RTM_LOADING) != 0){
		//TODO: wait for signal;
		return wrap(ref)->error_code;
	}

	// Try sync load first.
	if (ref->prototype->sync_load){
		return ref->prototype->sync_load(C, ref);
	}
	
	if (ref->prototype->async_load){
		wrap(ref)->status |= SS_RTM_LOADING;
		ref->prototype->async_load(C, ref);
		//TODO: wait for signal;
		return wrap(ref)->error_code;
	}
	
	return -1;
}

void ss_resource_async_load(ss_core_context* C, ss_resource_ref* ref)
{
	if (ref->ptr || (wrap(ref)->status & SS_RTM_LOADING) != 0){
		return;
	}
	/*
	if (ref->prototype->async_load){
		C->status |= SS_RTM_LOADING;
		ref->prototype->async_load(C, ref);
		return;
	}
	*/
	if (ref->prototype->sync_load){
		ref->prototype->sync_load(C, ref);
	}
}

void ss_resource_unload(ss_core_context* C, ss_resource_ref* ref)
{
	if (ref->ptr){
		ref->prototype->unload(C, ref);
		return;
	}
	auto* res = wrap(ref);
	if (res->status & SS_RTM_LOADING){
		//TODO: Try to cancel async work.

		// is loading.
		res->status |= SS_RTM_CANCELED;

		//TODO: wait for signal;

		if (ref->ptr){
			ref->prototype->unload(C, ref);
			return;
		}
	}
}

void ss_resource_async_unload(ss_core_context* C, ss_resource_ref* ref)
{
	if (ref->ptr){
		ref->prototype->unload(C, ref);
		return;
	}
	auto* res = wrap(ref);
	if (res->status & SS_RTM_LOADING){
		//TODO: Try to cancel async work if possible.

		// is loading.
		res->status |= SS_RTM_CANCELED;
	}
}

bool ss_resource_is_loading(ss_core_context* C, ss_resource_ref* ref)
{
	return (wrap(ref)->status & SS_RTM_LOADING) != 0;
}

