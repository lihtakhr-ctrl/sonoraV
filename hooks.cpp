#include "core.h"
#include "hooks.h"
#include "minhook/hook.h"
#include "../sdk/sdk.h"
#include "../features/esp/esp.h"
#include "../features/aimbot/aimbot.h"

using paint_t = int(__thiscall*)(void*, int);
paint_t o_paint;

int paint(void* thisptr, int mode) {
	auto ret = o_paint(thisptr, mode);

	int wide = 0, tall = 0;
	interfaces::surface->get_screen_size(wide, tall);

	globals::screen_size = vec2_t(wide, tall);
	globals::screen_center = vec2_t(wide / 2, tall / 2);

	if (mode == 1 || mode == 2) {
		interfaces::surface->start_drawing();
		esp::render();
		interfaces::surface->finish_drawing();
	}

	return ret;
}

using create_move_t = __int64(__thiscall*)(void*, int, float, bool);
create_move_t o_create_move;

__int64 create_move(void* thisptr, int sequence_number, float input_sample_frametime, bool active) {
	auto ret = o_create_move(thisptr, sequence_number, input_sample_frametime, active);

	auto cmd = interfaces::input->get_usercmd(sequence_number);
	if (!cmd || !cmd->command_number)
		return ret;

	auto old_cmd = *cmd;
	aimbot::run(cmd);
	return ret;
}

void hooks::init() {
	if (MH_Initialize() != MH_OK)
		return;

	if (MH_CreateHook(
		mem::get_virtual_func<void*>(interfaces::engine_vgui, 25),
		reinterpret_cast<void*>(&paint),
		reinterpret_cast<void**>(&o_paint)
	) != MH_OK)
		return;

	if (MH_CreateHook(
		mem::get_virtual_func<void*>(interfaces::input, 4),
		reinterpret_cast<void*>(&create_move),
		reinterpret_cast<void**>(&o_create_move)
	) != MH_OK)
		return;

	if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
		return;
}
