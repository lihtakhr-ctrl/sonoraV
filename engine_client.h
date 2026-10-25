#pragma once

class c_engine_client {
public:
	int get_local_player() {
		return mem::call_virtual_func<int>(this, 36);
	}
};
