#pragma once

#define PI 3.14159265358979323846f
#define PIX2 6.28318530718
#define PI_2 1.57079632679489661923
#define M_PI_2 1.57079632679489661923
#define M_PI 3.14159265358979323846f
#define M_RADPI	57.295779513082f
#define M_PI_F ( ( float )( M_PI ) )
#define RAD2DEG( x ) ( ( float )( x ) * ( float )( 180.f / M_PI_F ) )
#define DEG2RAD( x ) ( ( float )( x ) * ( float )( M_PI_F / 180.f ) )

#include "classes/vec2.h"
#include "classes/vec3.h"
#include "classes/qangle.h"

struct matrix16_t {
	float matrix[16];
};

namespace math {
	bool world_to_screen(
		const vec3_t& in,
		vec2_t& out);

	bool calc_bbox(
		vec3_t& origin,
		vec3_t& obb_mins,
		vec3_t& obb_maxs,
		vec2_t& pos,
		vec2_t& size);

	qangle to_angle(
		vec3_t src,
		vec3_t dst);
}
