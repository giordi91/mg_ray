#include "mg_rayLib/rendering/dxRenderer/uiWidgets.h"
#include "mg_rayLib/core/globalSettings.h"
#include "middleware/imgui/imgui.h"

#include <iostream>
namespace mg_ray {
namespace rendering {
namespace dx11 {
void RenderingSettingsWidget::initialize(core::GlobalSettings *settings) {
  m_settings = settings;
}
void RenderingSettingsWidget::render() {
  assert(m_settings != nullptr &&
         "rendering widget has not been initialized with a settings pointer");
  ImGui::Begin("Rendering Settings", &showUi);
  ImGui::Checkbox("Extend SPP range", &m_rangeExtender);
  if (m_rangeExtender) {
    ImGui::InputInt("Range multiplier", &m_rangeMultiplier);
  }
  ImGui::SetWindowCollapsed(ImGui::IsWindowCollapsed());
  ImGui::SliderInt("SPP", &m_settings->SPP, 1, 100 * m_rangeMultiplier);
  ImGui::SliderInt("Max recursion", &m_settings->maxRecursion, 1, 50);
  ImGui::SliderFloat("Aperture", &m_settings->aperture, 0.0001f, 5.0f);
  ImGui::SliderFloat("Focus distance", &m_settings->focusDistance, 0.1f, 60.0f);
  ImGui::SliderFloat("Ray min", &m_settings->tMin, 0.001f, 1.0f);
  ImGui::SliderFloat("Ray max", &m_settings->tMax, 100.0f, 1000.0f);
  ImGui::End();
}
void RenderingLabel::render() {
  if (showUi) {
  ImGui::Begin("Rendering in progress ...", &showUi);
  ImGui::End();

  }
}
} // namespace dx11
} // namespace rendering
} // namespace mg_ray
