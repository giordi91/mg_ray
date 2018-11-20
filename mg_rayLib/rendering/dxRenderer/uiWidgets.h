#pragma once

namespace mg_ray {
namespace core {
struct GlobalSettings;
}
namespace rendering {
namespace dx11 {
class UiWidget {
public:
  UiWidget() = default;
  virtual ~UiWidget() = default;
  virtual void render() = 0;

  inline void show(bool value) { showUi = value; }
  inline void collapsed(bool value) { collapsedUi = value; }

  inline bool isShown() const { return showUi; }
  inline bool isCollapsed() const { return collapsedUi; }
  inline bool isInitialized() const { return initUi; }

protected:
  bool showUi = false;
  bool collapsedUi = false;
  bool initUi = false;
};

class RenderingSettingsWidget : public UiWidget {
public:
  RenderingSettingsWidget() : UiWidget(){};
  void initialize(core::GlobalSettings *settings);
  ~RenderingSettingsWidget() = default;

  void render() override;

private:
  bool m_rangeExtender = false;
  int m_rangeMultiplier = 1;
  core::GlobalSettings *m_settings;
};

class RenderingLabel : public UiWidget {
public:
  RenderingLabel() : UiWidget(){};
  inline void initialize(core::GlobalSettings *settings) {
    m_settings = settings;
  };
  ~RenderingLabel() = default;

  void render() override;

private:
  core::GlobalSettings *m_settings;
};
} // namespace dx11
} // namespace rendering
} // namespace mg_ray
