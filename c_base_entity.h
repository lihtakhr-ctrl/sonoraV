#pragma once

class c_collision_property {
public:
	vec3_t& obb_mins() {
		using t = vec3_t&(__thiscall*)(void*);
		auto fn = mem::get_virtual_func<t>(this, 1);
		return fn(this);
	}

	vec3_t& obb_maxs() {
		using t = vec3_t&(__thiscall*)(void*);
		auto fn = mem::get_virtual_func<t>(this, 2);
		return fn(this);
	}
};

struct glow_mode {
	int8_t general_glow_mode, border_glow_mode, border_size, transparent_level;
};

struct glow_color {
	float r, g, b;
};

class c_base_entity {
public:
	c_collision_property* get_collideable() {
		return mem::call_virtual_func<c_collision_property*>(this, 2, this);
	}

	const char* m_signifier_name() {
		return *reinterpret_cast<const char**>(this + 0x518);
	}

	vec3_t m_vec_origin() {
		return *reinterpret_cast<vec3_t*>(this + 0x14C);
	}

	int m_life_state() {
		return *reinterpret_cast<int*>(this + 0x730);
	}

	int m_health() {
		return *reinterpret_cast<int*>(this + 0x3E0);
	}

	int m_max_health() {
		return *reinterpret_cast<int*>(this + 0x510);
	}

	void enable_glow(glow_color color, glow_mode mode) {
		*reinterpret_cast<int*>(this + 0x310) = 1;
		*reinterpret_cast<int*>(this + 0x320) = 2;
		*reinterpret_cast<glow_mode*>(this + 0x27C) = mode;
		*reinterpret_cast<glow_color*>(this + 0x1D0) = color;
	}

	void disable_glow() {
		*reinterpret_cast<int*>(this + 0x310) = 0;
		*reinterpret_cast<int*>(this + 0x320) = 0;
	}

	vec3_t get_bone_position(int bone_id) {
		auto bone_array = *reinterpret_cast<uintptr_t*>(this + 0xEE0);
		if (!bone_array)
			return {};

		auto out = m_vec_origin();
		out.x += *reinterpret_cast<float*>(bone_array + 0xCC + (bone_id * 0x30));
		out.y += *reinterpret_cast<float*>(bone_array + 0xDC + (bone_id * 0x30));
		out.z += *reinterpret_cast<float*>(bone_array + 0xEC + (bone_id * 0x30));
		return out;
	}

	vec3_t m_camera_origin() {
		return *reinterpret_cast<vec3_t*>(this + 0x1b68);
	}

	qangle m_vec_punch_angle() {
		return *reinterpret_cast<qangle*>(this + 0x20bc);
	}

	qangle m_vec_sway_angle() {
		return *reinterpret_cast<qangle*>(this + 0x2178);
	}
};
