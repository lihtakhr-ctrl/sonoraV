#pragma once

class i_surface;
class c_input;
class i_entity_list;
class c_engine_client;
class i_input_system;
class i_engine_trace;

namespace interfaces {
	inline uintptr_t process_base = 0;
	inline void* engine_vgui = nullptr;
	inline i_surface* surface = nullptr;
	inline c_input* input = nullptr;
	inline void* client = nullptr;
	inline i_entity_list* entity_list = nullptr;
	inline c_engine_client* engine_client = nullptr;
	inline i_input_system* input_system = nullptr;
	inline i_engine_trace* engine_trace = nullptr;

	bool init();
}
