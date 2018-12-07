#ifndef BABYLON_LIGHTS_SHADOWS_SHADOW_GENERATOR_SCENE_COMPONENT_H
#define BABYLON_LIGHTS_SHADOWS_SHADOW_GENERATOR_SCENE_COMPONENT_H

#include <babylon/babylon_api.h>
#include <babylon/engine/iscene_component.h>
#include <babylon/engine/iscene_serializable_component.h>
#include <babylon/engine/scene_component_constants.h>

namespace BABYLON {

class AbstractScene;
class ShadowGeneratorSceneComponent;
class RenderTargetTexture;
using ShadowGeneratorSceneComponentPtr
  = std::shared_ptr<ShadowGeneratorSceneComponent>;
using RenderTargetTexturePtr = std::shared_ptr<RenderTargetTexture>;

/**
 * @brief Defines the shadow generator component responsible to manage any
 * shadow generators in a given scene.
 */
class BABYLON_SHARED_EXPORT ShadowGeneratorSceneComponent
    : public ISceneSerializableComponent {

public:
  /**
   * The component name helpfull to identify the component in the list of scene
   * components.
   */
  static constexpr const char* name
    = SceneComponentConstants::NAME_SHADOWGENERATOR;

public:
  template <typename... Ts>
  static ShadowGeneratorSceneComponentPtr New(Ts&&... args)
  {
    return std::shared_ptr<ShadowGeneratorSceneComponent>(
      new ShadowGeneratorSceneComponent(std::forward<Ts>(args)...));
  }
  virtual ~ShadowGeneratorSceneComponent();

  /**
   * @brief Registers the component in a given scene.
   */
  void _register() override;

  /**
   * @brief Rebuilds the elements related to this component in case of
   * context lost for instance.
   */
  void rebuild() override;

  /**
   * @brief Serializes the component data to the specified json object
   * @param serializationObject The object to serialize to
   */
  void serialize(const nlohmann::json& serializationObject) override;

  /**
   * @brief Adds all the element from the container to the scene
   * @param container the container holding the elements
   */
  void addFromContainer(AbstractScene* container) override;

  /**
   * Removes all the elements in the container from the scene
   * @param container contains the elements to remove
   */
  void removeFromContainer(AbstractScene* container) override;

  /**
   * @brief Disposes the component and the associated resources.
   */
  void dispose() override;

protected:
  /**
   * @brief Creates a new instance of the component for the given scene.
   * @param scene Defines the scene to register the component in
   */
  ShadowGeneratorSceneComponent(Scene* scene);

private:
  void _gatherRenderTargets(std::vector<RenderTargetTexturePtr>& renderTargets);

}; // end of class ShadowGeneratorSceneComponent

} // end of namespace BABYLON

#endif // end of BABYLON_LIGHTS_SHADOWS_SHADOW_GENERATOR_SCENE_COMPONENT_H