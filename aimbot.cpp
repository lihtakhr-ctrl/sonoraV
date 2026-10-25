#include "core.h"
#include "../sdk/sdk.h"
#include "aimbot.h"

void aimbot::run(c_usercmd* cmd) {
	auto local_player = interfaces::entity_list->get_client_entity(
		interfaces::engine_client->get_local_player());

	if (!local_player)
		return;

	if (!interfaces::input_system->is_button_down(button_code_t::mouse_left))
		return;

	auto camera_origin = local_player->m_camera_origin();

	c_base_entity* closest_player = nullptr;
	float closest_distance = FLT_MAX;
	vec3_t closest_pos = {};
	bool closest_found = false;

	for (int i = 0; i <= interfaces::entity_list->get_highest_entity_index(); i++) {
		auto entity = interfaces::entity_list->get_client_entity(i);
		if (!entity || entity == local_player)
			continue;

		auto signifier_name = entity->m_signifier_name();
		if (!signifier_name || (strcmp(signifier_name, "npc_dummie") && strcmp(signifier_name, "player")))
			continue;

		if (entity->m_life_state() > 0)
			continue;

		auto pos = entity->get_bone_position(8);
		if (pos.is_zero())
			continue;

		vec2_t screen_pos{};
		if (!math::world_to_screen(pos, screen_pos))
			continue;

		ray_t ray;
		ray.init(camera_origin, pos);

		trace_t trace;

		c_trace_filter filter;
		filter.skip = local_player;

		interfaces::engine_trace->trace_ray_filtered(ray, TRACE_MASK_SHOT, &filter, &trace);

		if (trace.hit_entity != nullptr)
			continue;

		float distance = globals::screen_center.distance(screen_pos);

		if (distance < closest_distance) {
			closest_distance = distance;
			closest_player = entity;
			closest_pos = pos;
			closest_found = true;
		}
	}

	if (!closest_found)
		return;

	auto desired_angles = math::to_angle(camera_origin, closest_pos);
	auto punch_angles = local_player->m_vec_punch_angle();

	desired_angles -= punch_angles;
	desired_angles.normalize();

	cmd->viewangles = desired_angles;

	*reinterpret_cast<bool*>((uintptr_t)cmd + 0x41) = true;
}
