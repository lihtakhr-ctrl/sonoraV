#pragma once

class i_entity_list {
public:
	c_base_entity* get_client_entity(int ent_num) {
		return mem::call_virtual_func<c_base_entity*>(this, 4, this, ent_num);
	}

	int get_highest_entity_index() {
		return mem::call_virtual_func<int>(this, 8, this);
	}
};
