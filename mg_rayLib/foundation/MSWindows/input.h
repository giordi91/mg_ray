#pragma once
#include <cstring>

namespace mg_ray {
namespace foundation{
class Input {

public:
  Input() = default;
  static constexpr int SIZE_OF_KEYS = 256;
  ~Input() = default;

  void Initialize() { memset(m_keys, 0, sizeof(bool) * SIZE_OF_KEYS); };

  inline void KeyDown(unsigned int input) { m_keys[input] = true; }
  inline void KeyUp(unsigned int input) { m_keys[input] = false; }
  inline bool IsKeyDown(unsigned int input) const { return m_keys[input]; }

  int m_mouse[4];
  int selection = 0;
  int m_mouse_posX = 0;
  int m_mouse_posY = 0;
  bool m_keys[SIZE_OF_KEYS];

private:
  Input(const Input &) = delete;
  Input &operator=(const Input &) = delete;
};

} // namespace core
} // namespace mg_ray
