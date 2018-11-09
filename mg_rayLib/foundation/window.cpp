#include "mg_rayLib/foundation/window.h"

#ifdef WIN32
#include "mg_rayLib/foundation/MSWindows/dxWindow.h"
#endif

namespace mg_ray {
namespace foundation {
Window* mg_ray::foundation::createWindow(int width, int height,
                                                         char *name) {

#ifdef WIN32
  auto win = new DxWindow();
  win->initialize(width, height, name);
  return win;
#else
  return nullptr;
#endif

}
} // namespace foundation
} // namespace mg_ray
