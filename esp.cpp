#include "core.h"
#include "../sdk/sdk.h"
#include "esp.h"

void esp::render() {
	auto local_player = interfaces::entity_list->get_client_entity(
		interfaces::engine_client->get_local_player());

	if (!local_player)
		return;

	auto font = interfaces::surface->get_font();

	for (int i = 0; i <= interfaces::entity_list->get_highest_entity_index(); i++) {
		auto entity = interfaces::entity_list->get_client_entity(i);
		if (!entity || entity == local_player)
			continue;

		auto signifier_name = entity->m_signifier_name();
		if (!signifier_name || (strcmp(signifier_name, "npc_dummie") && strcmp(signifier_name, "player")))
			continue;

		if (entity->m_life_state() > 0)
			continue;

		auto origin = entity->m_vec_origin();
		if (origin.is_zero())
			continue;

		auto obb_mins = entity->get_collideable()->obb_mins();
		auto obb_maxs = entity->get_collideable()->obb_maxs();

		vec2_t pos{}, size{};
		if (!math::calc_bbox(
			origin,
			obb_mins,
			obb_maxs,
			pos,
			size))
			continue;

		interfaces::surface->draw_set_color(0, 0, 0, 255);
		interfaces::surface->draw_outlined_rect(pos.x - 1, pos.y - 1, pos.x + size.x + 1, pos.y + size.y + 1);
		interfaces::surface->draw_outlined_rect(pos.x + 1, pos.y + 1, pos.x + size.x - 1, pos.y + size.y - 1);

		interfaces::surface->draw_set_color(255, 255, 255, 255);
		interfaces::surface->draw_outlined_rect(pos.x, pos.y, pos.x + size.x, pos.y + size.y);

		auto hp_bar = [pos, size](int& current_hp, int& max_hp) -> void {
			if (current_hp > max_hp)
				current_hp = max_hp;

			const int bar_size = 1;

			const auto bar_min = vec2_t(pos.x - 4, pos.y);
			const auto bar_max = vec2_t(bar_min.x + bar_size, bar_min.y + size.y);

			interfaces::surface->draw_set_color(0, 0, 0, 255);
			interfaces::surface->draw_filled_rect(bar_min.x - 1, bar_min.y - 1, bar_max.x + 1, bar_max.y + 1);

			const int fill_height = static_cast<int>(size.y * current_hp / max_hp);

			interfaces::surface->draw_set_color(159, 211, 191, 255);
			interfaces::surface->draw_filled_rect(bar_min.x, bar_max.y - fill_height, bar_max.x, bar_max.y);
			};

		auto current_hp = entity->m_health();
		auto max_hp = entity->m_max_health();

		hp_bar(current_hp, max_hp);

		entity->enable_glow(glow_color(131, 105, 83), glow_mode(0, 6, 32, 255));
	}
}
