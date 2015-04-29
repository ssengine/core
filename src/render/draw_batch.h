#pragma once

#include <ssengine/render/drawbatch.h>
#include <ssengine/render/device.h>
#include <ssengine/render/types.h>

typedef struct ss_render_device ss_render_device;

struct ss_draw_batch_impl : public ss_draw_batch{
    ss_draw_batch_impl(ss_render_device* _device);
    ~ss_draw_batch_impl();

	void flush();

	void prepare(ss_primitive_type pt, size_t count);
	void prepare(ss_texture2d* texture, ss_primitive_type pt, size_t count);
	void prepare(ss_render_technique* tech, ss_primitive_type pt, size_t count);
	void prepare(ss_render_technique* tech, ss_texture2d* texture, ss_primitive_type pt, size_t count);

	ss::float2& pos(size_t idx){
		return ptr_position[idx + offset];
	}
	ss::color& diffuse(size_t idx){
		return ptr_diffuse[idx + offset];
	}
	ss::float2& texcoord(size_t idx){
		return ptr_texcoord[idx + offset];
	}

private:
	ss_render_device* device;

	// Input layouts
	ss_render_input_layout*  il_texture;
	ss_render_input_layout*  il_notexture;

	// Vertex buffers
	union {
		struct {
			ss_buffer_memory*		buf_position;
			ss_buffer_memory*		buf_diffuse;
			ss_buffer_memory*		buf_texcoord;
		};
		// Is this safe? Check it out everywhere.
		// It should be safe.
		ss_buffer*					buffers[3];
	};

	ss::float2*		ptr_position;
	ss::color*		ptr_diffuse;
	ss::float2*		ptr_texcoord;

	// Active status:
	ss_primitive_type		 pt;
	ss_render_technique*	 tech;
	ss_render_input_layout*  il;
	ss_texture2d*		     texture;
	
	size_t					 size;
	size_t					 offset;
};

