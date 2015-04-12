#include <ssengine/render/types.h>

#include <math.h>

namespace ss{
	const matrix matrix::identity = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};


	matrix::matrix(float(&arr)[16]){
		memcpy(data, arr, sizeof(float)* 16);
	}
	matrix::matrix(std::initializer_list<float> l){
		std::copy(l.begin(), l.end(), data);
	}
	matrix::matrix(const matrix& other){
		memcpy(data, other.data, sizeof(float)* 16);
	}

	bool matrix::operator==(const matrix& other) const{
		return memcmp(data, other.data, sizeof(float)* 16) == 0;
	}

	matrix& matrix::operator = (const matrix& other){
		memcpy(data, other.data, sizeof(float)* 16);
		return (*this);
	}

	//(x,y,z,w)*M*A == (x+dx,y+dy,z+dz, w)*A
	//M =
	//  1  0  0  0
	//  0  1  0  0
	//  0  0  1  0
	// dx dy dz  1
	//let A = M*A
	void matrix::push_translate(float dx, float dy){
		for (int i = 0; i < 4; i++){
			arr[3][i] += arr[0][i] * dx + arr[1][i] * dy;
		}
	}
	void matrix::push_translate(float dx, float dy, float dz){
		for (int i = 0; i < 4; i++){
			arr[3][i] += arr[0][i] * dx + arr[1][i] * dy + arr[2][i]*dz;
		}
	}

	//(x,y,z,w)*M*A == (x*sx,y*sy,z*dz, w)*A
	//M = 
	// sx  0  0  0
	//  0 sy  0  0
	//  0  0 sz  0
	//  0  0  0  1
	//let A = M*A
	void matrix::push_scale(float sx, float sy){
		for (int i = 0; i < 4; i++){
			arr[0][i] *= sx;
			arr[1][i] *= sy;
		}
	}
	void matrix::push_scale(float sx, float sy, float sz){
		for (int i = 0; i < 4; i++){
			arr[0][i] *= sx;
			arr[1][i] *= sy;
			arr[2][i] *= sz;
		}
	}

	//(x,y,z,w)*M*A == (x*cos-y*sin, y*cos+x*sin, z, w)*A
	//M = 
	// cos -sin  0 0
	// sin  cos  0 0
	//   0    0  1 0
	//   0    0  0 1
	void matrix::push_rotate2d(float cos, float sin){
		for (int i = 0; i < 4; i++){
			float v1 = cos*arr[0][i] - sin*arr[1][i];
			float v2 = sin*arr[0][i] + cos*arr[1][i];
			arr[0][i] = v1;
			arr[1][i] = v2;
		}
	}

	void matrix::push_rotate2d(float angel){
		push_rotate2d(cosf(angel), sinf(angel));
	}

	matrix ortho2d(float width, float height){
		return matrix({
			2 / width, 0, 0, 0,
			0,  2/height, 0, 0,
			0, 0, 1, 0
			-1, -1, 0, 1
		});
	}

	float2 matrix::transpose(const float2& vec) const{
		float w;
		float2 ret;
		ret.x = vec.x * arr[0][0] + vec.y * arr[1][0] + arr[3][0];
		ret.y = vec.x * arr[0][1] + vec.y * arr[1][1] + arr[3][1];
		w = vec.x * arr[0][3] + vec.y * arr[1][3] + arr[3][3];
		return ret / w;
	}

    float3 matrix::transpose(const float3& vec) const{
		float w;
		float3 ret;
		ret.x = vec.x * arr[0][0] + vec.y * arr[1][0] + vec.z * arr[2][0] + arr[3][0];
		ret.y = vec.x * arr[0][1] + vec.y * arr[1][1] + vec.z * arr[2][1] + arr[3][1];
		ret.z = vec.x * arr[0][2] + vec.y * arr[1][2] + vec.z * arr[2][2] + arr[3][2];
		w = vec.x * arr[0][3] + vec.y * arr[1][3] + vec.z * arr[2][3] + arr[3][3];
		return ret / w;
	}

    float4 matrix::transpose(const float4& vec) const{
		float4 ret;
		ret.x = vec.x * arr[0][0] + vec.y * arr[1][0] + vec.z * arr[2][0] + vec.w * arr[3][0];
		ret.y = vec.x * arr[0][1] + vec.y * arr[1][1] + vec.z * arr[2][1] + vec.w * arr[3][1];
		ret.z = vec.x * arr[0][2] + vec.y * arr[1][2] + vec.z * arr[2][2] + vec.w * arr[3][2];
		ret.w = vec.x * arr[0][3] + vec.y * arr[1][3] + vec.z * arr[2][3] + vec.w * arr[3][3];
		return ret;
	}
}
