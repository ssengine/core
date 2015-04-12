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

	il_texture = device->get_predefined_technique(SS_PDT_STANDARD)
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

	delete il_texture;
	delete il_notexture;

	delete buf_position;
	delete buf_diffuse;
	delete buf_texcoord;
}

void ss_draw_batch::prepare(ss_primitive_type pt, size_t count)
{
	prepare(device->get_predefined_technique(SS_PDT_STANDARD_NO_TEXTURE), pt, count);
}

void ss_draw_batch::prepare(ss_texture2d* texture, ss_primitive_type pt, size_t count)
{
	prepare(device->get_predefined_technique(SS_PDT_STANDARD), texture, pt, count);
}

unsigned int s_strides[3] = { sizeof(float)* 2, sizeof(float)* 4, sizeof(float)* 2 };
unsigned int s_offsets[3] = { 0, 0, 0 };

void ss_draw_batch::prepare(ss_render_technique* _tech, ss_primitive_type _pt, size_t count)
{
	if (tech != _tech || il != il_notexture || pt != _pt || count + size > MAX_VERTEX_COUNT){
		flush();

		// Do not bind texcoord
		il = il_notexture;
		tech = _tech;
        texture = NULL;
		pt = _pt;

		device->set_input_layout(il);
		device->set_primitive_type(pt);
		device->set_vertex_buffer(0, 2, buffers, s_strides, s_offsets);
	}

	offset = size;
	size += count;
}

void ss_draw_batch::prepare(ss_render_technique* _tech, ss_texture2d* _texture, ss_primitive_type _pt, size_t count)
{
	if (tech != _tech || il != il_texture || pt != _pt || texture != _texture || count + size > MAX_VERTEX_COUNT){
		flush();

		il = il_texture;
		tech = _tech;
		texture = _texture;
		pt = _pt;

		device->set_input_layout(il);
		device->set_primitive_type(pt);
		device->set_vertex_buffer(0, 3, buffers, s_strides, s_offsets);
		device->set_ps_texture2d_resource(0, 1, &texture);
	}

	offset = size;
	size += count;
}

void ss_draw_batch::flush(){
	if (size > 0){
		// Copy vertex data into buffer
		// Why not lock instead? Prepare for DX and cross-thread render.
		buf_position->copy(0, ptr_position, sizeof(float2)*size);
		buf_diffuse->copy(0, ptr_diffuse, sizeof(color)*size);
		if (il == il_texture){
			buf_texcoord->copy(0, ptr_texcoord, sizeof(float2)*size);
		}

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


void ss_db_flush(ss_core_context* C){
	C->draw_batch->flush();
}

void ss_db_draw_line(ss_core_context* C, const ss::matrix& matrix, float x0, float y0, float x1, float y1){
	ss_draw_batch* v = C->draw_batch;
	v->prepare(SS_PT_LINELIST, 2);
    v->pos(0) = matrix.transpose(float2(x0, y0));
    v->pos(1) = matrix.transpose(float2(x1, y1));
	v->diffuse(0) = v->diffuse(1) = color(1, 1, 1);
}

void ss_db_draw_image_rect(
	ss_core_context* C,
    const ss::matrix& matrix,
	ss_texture2d*	 texture,
	float l, float t, float w, float h,
	float tl, float tt, float tw, float th
	){
	float r = l + w;
	float b = t + h;
	float tr = tl + tw;
	float tb = tt + th;

	ss_draw_batch* v = C->draw_batch;
	v->prepare(texture, SS_PT_TRIANGLELIST, 6);

	v->diffuse(0) = v->diffuse(1) =
		v->diffuse(2) = v->diffuse(3) =
		v->diffuse(4) = v->diffuse(5) = color(1, 1, 1);

    v->pos(0) = matrix.transpose(float2(l, t));
	v->texcoord(0) = float2(tl, tt);

    v->pos(4) = v->pos(1) = matrix.transpose(float2(r, t));
	v->texcoord(4) = v->texcoord(1) = float2(tr, tt);

    v->pos(3) = v->pos(2) = matrix.transpose(float2(l, b));
	v->texcoord(3) = v->texcoord(2) = float2(tl, tb);

    v->pos(5) = matrix.transpose(float2(r, b));
	v->texcoord(5) = float2(tr, tb);
}
