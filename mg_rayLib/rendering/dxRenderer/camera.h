#pragma once
#include <directxmath.h>

struct ID3D11Buffer;
struct ID3D11DeviceContext;

namespace mg_ray {
namespace rendering {
namespace dx11 {
class D3DClass;

struct PerspectiveBufferDef {
  DirectX::XMMATRIX viewInv;
  DirectX::XMFLOAT4 cameraPosition;
  DirectX::XMFLOAT4 perspectiveValues;
};

struct ObjectBufferDef {
  DirectX::XMMATRIX MVP;
  DirectX::XMMATRIX normalMatrix;
};

class Camera3dPivot {

public:
  Camera3dPivot(D3DClass *d3dClass, float width, float height, float nearPlane,
                float farPlane);
  ~Camera3dPivot();
  inline void setSize(float width, float height) {
    m_width = width, m_height = height;
  }
  // virtual void setCameraMatrixToShader(DirectX::XMMATRIX modelMatrix);
  DirectX::XMMATRIX getMVP(DirectX::XMMATRIX modelM);
  DirectX::XMMATRIX getMVPInverse(DirectX::XMMATRIX modelM);
  DirectX::XMMATRIX getViewInverse(DirectX::XMMATRIX modelM);

  inline void setPosition(float x, float y, float z) {
    posV = DirectX::XMLoadFloat3(&DirectX::XMFLOAT3(x, y, z));
  };
  inline void setLookAt(float x, float y, float z) {
    lookAtPosV = DirectX::XMLoadFloat3(&DirectX::XMFLOAT3(x, y, z));
  }

  inline DirectX::XMFLOAT3 getPosition() const {
    DirectX::XMFLOAT3 toReturn;
    DirectX::XMStoreFloat3(&toReturn, posV);
    return toReturn;
  }
  inline DirectX::XMFLOAT3 getLookAt() const {
    DirectX::XMFLOAT3 toReturn;
    DirectX::XMStoreFloat3(&toReturn, lookAtPosV);
    return toReturn;
  }
  inline ID3D11Buffer **getPointerToCameraBuffer() {
    return &m_perspValuesBuffer;
  }

  inline DirectX::XMMATRIX getViewMatrix() const { return m_viewMatrix; };

  void setupCameraBuffer(ID3D11DeviceContext *context, int slot);
  void panCamera(float deltaX, float deltaY);
  void rotCamera(float deltaX, float deltaY);
  void zoomCamera(float deltaX, float deltaY);

  void Render() {
    m_viewMatrix = DirectX::XMMatrixLookAtLH(posV, lookAtPosV, upVector);
  }
  DirectX::XMMATRIX getProjCamera(float screenWidth, float screenHeight) {
    constexpr float fieldOfView = DirectX::XM_PI / 4.0f;
    float screenAspect = (float)screenWidth / (float)screenHeight;

    return DirectX::XMMatrixPerspectiveFovLH(fieldOfView, screenAspect,
                                             m_nearPlane, m_farPlane);
  };
  inline DirectX::XMFLOAT4 getProjParams() {
    // preparing camera values for deferred
    auto proj = getProjCamera(m_width, m_height);
    DirectX::XMFLOAT4X4 projView;
    DirectX::XMStoreFloat4x4(&projView, proj);
    DirectX::XMFLOAT4 perspValues;
    perspValues.x = 1.0f / projView.m[0][0];
    perspValues.y = 1.0f / projView.m[1][1];
    perspValues.z = projView.m[3][2];
    perspValues.w = -projView.m[2][2];
    return perspValues;
  }

private:
  // Constants
  static const DirectX::XMFLOAT3 upVec3;
  static const DirectX::XMVECTOR upVector;
  static const float MOUSE_ROT_SPEED_SCALAR;
  static const float MOUSE_PAN_SPEED_SCALAR;
  static const DirectX::XMVECTOR MOUSE_ROT_SPEED_VECTOR;
  static const DirectX::XMVECTOR MOUSE_PAN_SPEED_VECTOR;
  float m_nearPlane;
  float m_farPlane;

  DirectX::XMVECTOR posV;
  DirectX::XMVECTOR lookAtPosV;
  DirectX::XMMATRIX m_viewMatrix;
  ID3D11Buffer *m_cameraBuffer;
  // buffer used to setup and prepare the camera
  PerspectiveBufferDef m_persp;
  ID3D11Buffer *m_perspValuesBuffer = nullptr;
  float m_width;
  float m_height;
  D3DClass *m_d3dClass = nullptr;
};
} // namespace dx11
} // namespace rendering
} // namespace mg_ray
