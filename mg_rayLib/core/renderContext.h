#pragma once

namespace mg_ray {
namespace core {

struct GlobalSettings;
class Scene;
struct SceneCamera;

enum class TextureOutputType { CPU, CUDA, DX11 };

struct TextureOutput {
  TextureOutputType type;
  int width;
  int height;
  void *data;
};

class RenderContext {

public:
  virtual ~RenderContext() = default;
  virtual bool initialize(GlobalSettings *settings) = 0;
  virtual bool loadScene(Scene *scene) = 0;
  virtual void run() = 0;
  virtual void cleanup() = 0;
  virtual void setSceneCamera(SceneCamera* camera) =0;
  virtual TextureOutput getTextureOutput()=0;


};

} // namespace core
} // namespace mg_ray
