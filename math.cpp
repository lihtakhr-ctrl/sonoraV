#include "core.h"
#include "../sdk.h"

bool math::world_to_screen(
	const vec3_t& in,
	vec2_t& out) 
{
	const auto view_renderer = *reinterpret_cast<uintptr_t*>(interfaces::process_base + 0xd4138f0);
	if (!view_renderer)
		return false;

	const auto view_matrix_temp = *reinterpret_cast<uintptr_t*>(view_renderer + 0x1a93d0);
	if (!view_matrix_temp)
		return false;

	const auto view_matrix = *reinterpret_cast<matrix16_t*>(view_matrix_temp);
	const auto matrix = view_matrix.matrix;

	const float w = matrix[12] * in.x + matrix[13] * in.y + matrix[14] * in.z + matrix[15];
	if (w < 0.001f)
		return false;

	out.x = matrix[0] * in.x + matrix[1] * in.y + matrix[2] * in.z + matrix[3];
	out.y = matrix[4] * in.x + matrix[5] * in.y + matrix[6] * in.z + matrix[7];

	out.x /= w;
	out.y /= w;

	out.x = (globals::screen_size.x / 2.0f) + (out.x * globals::screen_size.x) / 2.0f;
	out.y = (globals::screen_size.y / 2.0f) - (out.y * globals::screen_size.y) / 2.0f;
	out.x = std::trunc(out.x);
	out.y = std::trunc(out.y);

	if (out.is_zero())
		return false;

	return true;
}

bool math::calc_bbox(
	vec3_t& origin,
	vec3_t& obb_mins, 
	vec3_t& obb_maxs,
	vec2_t& pos,
	vec2_t& size) 
{
	static const std::vector<vec3_t> corner_array = {
		vec3_t(0, 0, 0), vec3_t(0, 0, 1),
		vec3_t(0, 1, 0), vec3_t(0, 1, 1),
		vec3_t(1, 0, 0), vec3_t(1, 0, 1),
		vec3_t(1, 1, 0), vec3_t(1, 1, 1)
	};

	bool found_first = false;

	float left = 0.f, top = 0.f, right = 0.f, bottom = 0.f;

	for (const auto& corner : corner_array) {
		vec3_t corner_3d(
			obb_mins.x + corner.x * (obb_maxs.x - obb_mins.x),
			obb_mins.y + corner.y * (obb_maxs.y - obb_mins.y),
			obb_mins.z + corner.z * (obb_maxs.z - obb_mins.z)
		);

		vec2_t screen_pos;
		if (!world_to_screen(origin + corner_3d, screen_pos))
			continue;

		screen_pos.x = std::round(screen_pos.x);
		screen_pos.y = std::round(screen_pos.y);

		if (!found_first) {
			left = right = screen_pos.x;
			top = bottom = screen_pos.y;
			found_first = true;
		}
		else {
			if (screen_pos.x < left)   left = screen_pos.x;
			if (screen_pos.x > right)  right = screen_pos.x;
			if (screen_pos.y < top)    top = screen_pos.y;
			if (screen_pos.y > bottom) bottom = screen_pos.y;
		}
	}

	if (found_first) {
		pos.x = left;
		pos.y = top;
		size.x = right - left;
		size.y = bottom - top;
	}

	return found_first;
}

qangle math::to_angle(vec3_t src, vec3_t dst) {
	auto origin = dst - src;
	return qangle(
		RAD2DEG(atan2(-origin.z, hypot(origin.x, origin.y))),
		RAD2DEG(atan2(origin.y, origin.x)),
		0.0f);
}
