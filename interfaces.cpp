#include "core.h"
#include "interfaces.h"
#include "../sdk.h"

bool interfaces::init() {
	process_base = mem::get_module_base(nullptr);
	std::cout << "[>] process_base -> " << std::hex << process_base << std::endl;
	if (!process_base)
		return false;

	engine_vgui = reinterpret_cast<void*>(
		mem::rel_to_abs(
			mem::pattern_scan(
				"48 8B 05 ? ? ? ? 48 8D 0D ? ? ? ? C6 05 ? ? ? ? ? 48 83 C4",
				process_base), 0x3, 0x7)
		);

	std::cout << "[>] engine_vgui -> " << std::hex << engine_vgui << std::endl;
	if (!engine_vgui)
		return false;

	surface = reinterpret_cast<i_surface*>(
		mem::rel_to_abs(
			mem::pattern_scan(
				"48 89 05 ? ? ? ? 44 8D 42 ? 66 0F 7F 0D",
				process_base), 0x3, 0x7)
		);

	std::cout << "[>] surface -> " << std::hex << surface << std::endl;
	if (!surface)
		return false;

	input = reinterpret_cast<c_input*>(
		mem::rel_to_abs(
			mem::pattern_scan(
				"48 8B 05 ? ? ? ? 41 8D 57 ? 48 8D 0D ? ? ? ? FF 90",
				process_base), 0x3, 0x7)
		);

	std::cout << "[>] input -> " << std::hex << input << std::endl;
	if (!input)
		return false;

	client = reinterpret_cast<void*>(
		mem::rel_to_abs(
			mem::pattern_scan(
				"48 8B 05 ? ? ? ? FF 10 48 8B 0D",
				process_base), 0x3, 0x7)
		);

	std::cout << "[>] client -> " << std::hex << client << std::endl;
	if (!client)
		return false;

	entity_list = reinterpret_cast<i_entity_list*>(
		mem::rel_to_abs(
			mem::pattern_scan(
				"48 89 05 ? ? ? ? 66 0F 7F 05 ? ? ? ? 4C 89 0D",
				process_base), 0x3, 0x7)
		);

	std::cout << "[>] entity_list -> " << std::hex << entity_list << std::endl;
	if (!entity_list)
		return false;

	engine_client = reinterpret_cast<c_engine_client*>(
		mem::rel_to_abs(
			mem::pattern_scan(
				"48 8B 05 ? ? ? ? FF 50 ? 0F 28 35",
				process_base), 0x3, 0x7)
		);

	std::cout << "[>] engine_client -> " << std::hex << engine_client << std::endl;
	if (!engine_client)
		return false;

	input_system = reinterpret_cast<i_input_system*>(
		mem::rel_to_abs(
			mem::pattern_scan(
				"48 89 05 ? ? ? ? 48 85 C9 74 ? FF 15",
				process_base), 0x3, 0x7)
		);

	std::cout << "[>] input_system -> " << std::hex << input_system << std::endl;
	if (!input_system)
		return false;

	engine_trace = reinterpret_cast<i_engine_trace*>(
		mem::rel_to_abs(
			mem::pattern_scan(
				"48 8B 05 ? ? ? ? 48 8D 4D ? 48 89 4C 24",
				process_base), 0x3, 0x7)
		);

	std::cout << "[>] engine_trace -> " << std::hex << engine_trace << std::endl;
	if (!engine_trace)
		return false;

	return true;
}
