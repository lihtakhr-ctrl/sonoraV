#pragma once

class i_surface {
public:
	void start_drawing() {
		using t = void(__thiscall*)(void*);
		static auto fn = reinterpret_cast<t>(mem::pattern_scan(
			"40 53 57 48 81 EC ? ? ? ? 48 8B F9",
			interfaces::process_base));

		fn(this);
	}

	void finish_drawing() {
		using t = void(__thiscall*)();
		static auto fn = reinterpret_cast<t>(mem::pattern_scan(
			"40 53 48 83 EC ? 48 8B 0D ? ? ? ? C6 05 ? ? ? ? ? 48 8B 01",
			interfaces::process_base));

		fn();
	}

	void draw_set_color(int r, int g, int b, int a) {
		return mem::call_virtual_func<void>(this, 14, this, r, g, b, a);
	}

	void draw_filled_rect(int x0, int y0, int x1, int y1) {
		return mem::call_virtual_func<void>(this, 16, this, x0, y0, x1, y1);
	}

	void draw_outlined_rect(int x0, int y0, int x1, int y1) {
		return mem::call_virtual_func<void>(this, 19, this, x0, y0, x1, y1);
	}

	void draw_line(int x0, int y0, int x1, int y1) {
		return mem::call_virtual_func<void>(this, 20, this, x0, y0, x1, y1);
	}

	void get_screen_size(int& wide, int& tall) {
		return mem::call_virtual_func<void>(this, 53, this, &wide, &tall);
	}

	unsigned long get_font() {
		return mem::call_virtual_func<unsigned long>(this, 163, this);
	}

	template <typename... Args>
	void draw_colored_text(unsigned long font, int font_height, int x, int y,
		int r, int g, int b, int a, const char* fmt, Args... args)
	{
		return mem::call_virtual_func<void>(this, 200, this, font, font_height, x, y, r, g, b, a, fmt, args...);
	}
};
