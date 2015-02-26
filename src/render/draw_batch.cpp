#include "draw_batch.h"
#include "../core.h"

#define MAX_VERTEX_COUNT (1024)

using namespace ss;

//TODO: use index buffer for less vertex usage.

//TODO: use Map(Lock) instead of copy may be good for performance, but needs test.

ss_draw_batch::ss_draw_batch(ss_render_device* _device)
	: device(_device), size(0), offset(0)
{
	ss_render_input_element layout_elements[] = {
		{ SS_USAGE_POSITION, 0, SS_FORMAT_FLOAT32_RG, 0, 0 },
		{ SS_USAGE_DIFFUSE, 0, SS_FORMAT_FLOAT32_RGBA, 1, 0 },
		{ SS_USAGE_TEXCOORD, 0, SS_FORMAT_FLOAT32_RG, 2, 0 }
	};

	// Do not use texcoord if there's no texture.
	il_notexture = 
		device->get_predefined_technique(SS_PDT_STANDARD_NO_TEXTURE)
		->create_input_layout(layout_elements, 2);

	il = device->get_predefined_technique(SS_PDT_STANDARD)
		->create_input_layout(layout_elements, 3);

	// Create vertex buffers;
	buf_position = device->create_memory_buffer(sizeof(float)* 2 * MAX_VERTEX_COUNT);
	buf_diffuse = device->create_memory_buffer(sizeof(float)* 4 * MAX_VERTEX_COUNT);
	buf_texcoord = device->create_memory_buffer(sizeof(float)* 2 * MAX_VERTEX_COUNT);

	ptr_position = new ss::float2[MAX_VERTEX_COUNT];
	ptr_diffuse = new ss::color[MAX_VERTEX_COUNT];
	ptr_texcoord = new ss::float2[MAX_VERTEX_COUNT];
}

ss_draw_batch::~ss_draw_batch()
{
	this->flush();

	delete[] ptr_texcoord;
	delete[] ptr_diffuse;
	delete[] ptr_position;
}

void ss_draw_batch::prepare(ss_primitive_type pt, size_t count)
{
	prepare(device->get_predefined_technique(SS_PDT_STANDARD_NO_TEXTURE), pt, count);
}

void ss_draw_batch::prepare(ss_texture2d* texture, ss_primitive_type pt, size_t count)
{
	prepare(device->get_predefined_technique(SS_PDT_STANDARD), texture, pt, count);
}

size_t s_strides[3] = { sizeof(float)* 2, sizeof(float)* 4, sizeof(float)* 2 };
size_t s_offsets[3] = { 0, 0, 0 };

void ss_draw_batch::prepare(ss_render_technique* _tech, ss_primitive_type _pt, size_t count)
{
	if (tech != _tech || il != il_notexture || pt != _pt || count + size > MAX_VERTEX_COUNT){
		flush();
	}
	// Do not bind texcoord
	il = il_notexture;
	tech = _tech;
	pt = _pt;

	device->set_input_layout(il);
	device->set_primitive_type(pt);
	device->set_vertex_buffer(0, 2, buffers, s_strides, s_offsets);

	offset = size;
	size += count;
}

void ss_draw_batch::prepare(ss_render_technique* _tech, ss_texture2d* _texture, ss_primitive_type _pt, size_t count)
{
	if (tech != _tech || il != il_texture || pt != _pt || texture != _texture || count + size > MAX_VERTEX_COUNT){
		flush();
	}
	il = il_texture;
	tech = _tech;
	texture = _texture;
	pt = _pt;

	device->set_input_layout(il);
	device->set_primitive_type(pt);
	device->set_vertex_buffer(0, 3, buffers, s_strides, s_offsets);
	device->set_ps_texture2d_resource(0, 1, &texture);

	offset = size;
	size += count;
}

void ss_draw_batch::flush(){
	if (size > 0){
		// Copy vertex data into buffer
		// Why not lock instead? Prepare for DX and cross-thread render.
		buf_position->copy(0, ptr_position, sizeof(float2)*size);
		buf_diffuse->copy(0, ptr_diffuse, sizeof(color)*size);
		buf_texcoord->copy(0, ptr_texcoord, sizeof(float2)*size);

		// Do draw
		for (size_t i = 0; i < tech->pass_count(); i++){
			ss_render_pass* pass = tech->get_pass(i);
			pass->begin();
			device->draw(size, 0);
			pass->end();
		}
	}
	size = 0;
	offset = 0;

#ifndef NDEBUG
	tech = NULL;
	il = NULL;
	texture = NULL;
#endif
}


SS_CORE_API void ss_db_flush(ss_core_context* C){
	C->draw_batch->flush();
}

SS_CORE_API void ss_db_draw_line(ss_core_context* C, float x0, float y0, float x1, float y1){
	ss_draw_batch* v = C->draw_batch;
	v->prepare(SS_PT_LINELIST, 2);
	v->pos(0) = float2(x0, y0);
	v->pos(1) = float2(x1, y1);
	v->diffuse(0) = v->diffuse(1) = color(1, 1, 1);
}

