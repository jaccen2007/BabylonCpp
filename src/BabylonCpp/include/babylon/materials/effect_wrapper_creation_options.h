#ifndef BABYLON_MATERIALS_EFFECT_WRAPPER_CREATION_OPTIONS_H
#define BABYLON_MATERIALS_EFFECT_WRAPPER_CREATION_OPTIONS_H

#include <string>
#include <vector>

#include <babylon/babylon_api.h>

namespace BABYLON {

class ThinEngine;

/**
 * @brief Options to create an EffectWrapper.
 */
struct BABYLON_SHARED_EXPORT EffectWrapperCreationOptions {
  /**
   * Engine to use to create the effect
   */
  ThinEngine* engine = nullptr;
  /**
   * Fragment shader for the effect
   */
  std::string fragmentShader;
  /**
   * Vertex shader for the effect
   */
  std::string vertexShader;
  /**
   * Attributes to use in the shader
   */
  std::vector<std::string> attributeNames;
  /**
   * Uniforms to use in the shader
   */
  std::vector<std::string> uniformNames;
  /**
   * Texture sampler names to use in the shader
   */
  std::vector<std::string> samplerNames;
  /**
   * The friendly name of the effect displayed in Spector.
   */
  std::string name;
}; // end of struct EffectWrapperCreationOptions

} // end of namespace BABYLON

#endif // end of BABYLON_MATERIALS_EFFECT_WRAPPER_CREATION_OPTIONS_H
