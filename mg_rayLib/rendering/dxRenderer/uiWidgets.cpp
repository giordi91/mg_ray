#include "mg_rayLib/rendering/dxRenderer/uiWidgets.h"
#include "middleware/imgui/imgui.h"

#include <iostream>
namespace mg_ray {
namespace rendering {
namespace dx11 {

void RenderingSettingsWidget::render() {
  ImGui::Begin("Rendering Settings", &showUi);
  ImGui::Checkbox("Extend SPP range", &m_rangeExtender);
  if (m_rangeExtender) {
    ImGui::InputInt("Range multiplier", &m_rangeMultiplier);
  }
  ImGui::SetWindowCollapsed(ImGui::IsWindowCollapsed());
  ImGui::SliderInt("SPP", &m_SPP, 1, 100 * m_rangeMultiplier);
  ImGui::SliderFloat("Aperture", &m_aperture, 0.0001f, 5.0f);
  ImGui::SliderFloat("Focus distance", &m_focusDist, 0.1f, 60.0f);
  ImGui::End();
}
} // namespace dx11
} // namespace rendering
} // namespace mg_ray
