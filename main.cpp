#include "core.h"
#include "hooks/hooks.h"
#include "sdk/sdk.h"

namespace console {
	void init() {
		AllocConsole();
		SetConsoleTitleA("");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xF);

		freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);
		freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
		freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);
	}

	void uninit() {
		FreeConsole();

		fclose(stdin);
		fclose(stdout);
		fclose(stderr);
	}
}

bool DllMain(const void* instance, const uint32_t reason, const void* reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
		console::init();
		if (!interfaces::init()) {
			std::cerr << "[!] interfaces not inited" << std::endl;
			return false;
		}

		hooks::init();
    }
    return true;
}
