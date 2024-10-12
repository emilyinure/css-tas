#include "cheat.h"
#include "core/input/input.h"

#include <thread>
#include <chrono>

#include <Windows.h>

DWORD WINAPI initialize(void* arg) {
  if (cheat::initialize()) {
    while (!cheat::input.key_down(VK_DELETE)) {
      Sleep(250);
    }
  }

  printf("detach\n");

  cheat::detach();

  FreeLibraryAndExitThread(static_cast<HMODULE>(arg), EXIT_SUCCESS);
}

BOOL WINAPI DllMain(HINSTANCE self, DWORD reason, LPVOID reserved) {
  if (reason == DLL_PROCESS_ATTACH) {
    if (auto handle = CreateThread(nullptr, 0, initialize, self, 0, nullptr)) {
      CloseHandle(handle);
    }
  }

  return 1;
}
