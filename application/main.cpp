#include "windows.h"
#include <cstdio>
#include <iostream>

#include "mg_rayLib/application/renderingManager.h"
#include <string>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline,
                   int iCmdshow) {
  AllocConsole();
  auto *c = (stdout);
  freopen_s(&c, "CONOUT$", "w", stdout);

  // lovely windows crap to get the startup path
  TCHAR szPath[_MAX_PATH];
  GetModuleFileName(hInstance, szPath, _MAX_PATH);
  std::wstring temp(&szPath[0]); // convert to wstring
  std::string engineExectuable(temp.begin(),
                               temp.end()); // and convert to string.
  std::cout << "[ENGINE] Staring up from path: " << engineExectuable
            << std::endl;

  mg_ray::core::GlobalSettings settings ;
  mg_ray::application::RenderingManager manager(settings);
  manager.initialize();
  //manager.loadSceneFromFile("scenes/demo1.json");
  manager.loadSceneFromFile("scenes/demo3.json");
  manager.run();

  return 0;
}
