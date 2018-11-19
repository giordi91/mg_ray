#pragma once
#include <DirectXMath.h>

namespace mg_ray {
namespace rendering {
namespace dx11 {
class UiWidget {
public:
  UiWidget() = default;
  virtual ~UiWidget() = default;
  virtual void render() = 0;

  inline void show(bool value) { showUi = value; }
  inline void collapsed(bool value) { collapsedUi = value; }
  inline void initialized(bool value) { initUi = value; }

  inline bool isShown() const { return showUi; }
  inline bool isCollapsed() const { return collapsedUi; }
  inline bool isInitialized() const { return initUi; }

protected:
  bool showUi = false;
  bool collapsedUi = false;
  bool initUi = false;
};


class RenderingSettingsWidget: public UiWidget {
public:
  RenderingSettingsWidget() : UiWidget(){};
  ~RenderingSettingsWidget() = default;

  void render() override;

private:
  float m_aperture = 0.1f;
  float m_focusDist= 10.0f;
  int m_SPP = 10;
  bool m_rangeExtender = false;
  int m_rangeMultiplier= 1;

};
} // namespace dx11
} // namespace rendering
} // namespace mg_ray
