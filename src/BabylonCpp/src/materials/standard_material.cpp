#include <babylon/materials/standard_material.h>

#include <babylon/animations/animation.h>
#include <babylon/babylon_stl_util.h>
#include <babylon/bones/skeleton.h>
#include <babylon/cameras/camera.h>
#include <babylon/core/json_util.h>
#include <babylon/engine/engine.h>
#include <babylon/engine/scene.h>
#include <babylon/lights/directional_light.h>
#include <babylon/lights/hemispheric_light.h>
#include <babylon/lights/light.h>
#include <babylon/lights/point_light.h>
#include <babylon/lights/shadows/shadow_generator.h>
#include <babylon/lights/spot_light.h>
#include <babylon/materials/color_curves.h>
#include <babylon/materials/effect.h>
#include <babylon/materials/effect_creation_options.h>
#include <babylon/materials/effect_fallbacks.h>
#include <babylon/materials/fresnel_parameters.h>
#include <babylon/materials/image_processing_configuration.h>
#include <babylon/materials/material_helper.h>
#include <babylon/materials/standard_material_defines.h>
#include <babylon/materials/textures/base_texture.h>
#include <babylon/materials/textures/color_grading_texture.h>
#include <babylon/materials/textures/cube_texture.h>
#include <babylon/materials/textures/refraction_texture.h>
#include <babylon/materials/textures/render_target_texture.h>
#include <babylon/materials/uniform_buffer.h>
#include <babylon/mesh/abstract_mesh.h>
#include <babylon/mesh/mesh.h>
#include <babylon/mesh/sub_mesh.h>
#include <babylon/mesh/vertex_buffer.h>
#include <babylon/tools/serialization_helper.h>

namespace BABYLON {

bool StandardMaterial::_DiffuseTextureEnabled      = true;
bool StandardMaterial::_AmbientTextureEnabled      = true;
bool StandardMaterial::_OpacityTextureEnabled      = true;
bool StandardMaterial::_ReflectionTextureEnabled   = true;
bool StandardMaterial::_EmissiveTextureEnabled     = true;
bool StandardMaterial::_SpecularTextureEnabled     = true;
bool StandardMaterial::_BumpTextureEnabled         = true;
bool StandardMaterial::_FresnelEnabled             = true;
bool StandardMaterial::_LightmapTextureEnabled     = true;
bool StandardMaterial::_RefractionTextureEnabled   = true;
bool StandardMaterial::_ColorGradingTextureEnabled = true;

StandardMaterial::StandardMaterial(const std::string& iName, Scene* scene)
    : PushMaterial{iName, scene}
    , diffuseTexture{this, &StandardMaterial::get_diffuseTexture,
                     &StandardMaterial::set_diffuseTexture}
    , ambientTexture{this, &StandardMaterial::get_ambientTexture,
                     &StandardMaterial::set_ambientTexture}
    , opacityTexture{this, &StandardMaterial::get_opacityTexture,
                     &StandardMaterial::set_opacityTexture}
    , reflectionTexture{this, &StandardMaterial::get_reflectionTexture,
                        &StandardMaterial::set_reflectionTexture}
    , emissiveTexture{this, &StandardMaterial::get_emissiveTexture,
                      &StandardMaterial::set_emissiveTexture}
    , specularTexture{this, &StandardMaterial::get_specularTexture,
                      &StandardMaterial::set_specularTexture}
    , bumpTexture{this, &StandardMaterial::get_bumpTexture,
                  &StandardMaterial::set_bumpTexture}
    , lightmapTexture{this, &StandardMaterial::get_lightmapTexture,
                      &StandardMaterial::set_lightmapTexture}
    , refractionTexture{this, &StandardMaterial::get_refractionTexture,
                        &StandardMaterial::set_refractionTexture}
    , ambientColor{Color3(0.f, 0.f, 0.f)}
    , diffuseColor{Color3(1.f, 1.f, 1.f)}
    , specularColor{Color3(1.f, 1.f, 1.f)}
    , emissiveColor{Color3(0.f, 0.f, 0.f)}
    , specularPower{64.f}
    , useAlphaFromDiffuseTexture{this,
                                 &StandardMaterial::
                                   get_useAlphaFromDiffuseTexture,
                                 &StandardMaterial::
                                   set_useAlphaFromDiffuseTexture}
    , useEmissiveAsIllumination{this,
                                &StandardMaterial::
                                  get_useEmissiveAsIllumination,
                                &StandardMaterial::
                                  set_useEmissiveAsIllumination}
    , linkEmissiveWithDiffuse{this,
                              &StandardMaterial::get_linkEmissiveWithDiffuse,
                              &StandardMaterial::set_linkEmissiveWithDiffuse}
    , useSpecularOverAlpha{this, &StandardMaterial::get_useSpecularOverAlpha,
                           &StandardMaterial::set_useSpecularOverAlpha}
    , useReflectionOverAlpha{this,
                             &StandardMaterial::get_useReflectionOverAlpha,
                             &StandardMaterial::set_useReflectionOverAlpha}
    , disableLighting{this, &StandardMaterial::get_disableLighting,
                      &StandardMaterial::set_disableLighting}
    , useObjectSpaceNormalMap{this,
                              &StandardMaterial::get_useObjectSpaceNormalMap,
                              &StandardMaterial::set_useObjectSpaceNormalMap}
    , useParallax{this, &StandardMaterial::get_useParallax,
                  &StandardMaterial::set_useParallax}
    , useParallaxOcclusion{this, &StandardMaterial::get_useParallaxOcclusion,
                           &StandardMaterial::set_useParallaxOcclusion}
    , parallaxScaleBias{0.05f}
    , roughness{this, &StandardMaterial::get_roughness,
                &StandardMaterial::set_roughness}
    , indexOfRefraction{0.98f}
    , invertRefractionY{true}
    , alphaCutOff{0.4f}
    , useLightmapAsShadowmap{this,
                             &StandardMaterial::get_useLightmapAsShadowmap,
                             &StandardMaterial::set_useLightmapAsShadowmap}
    , diffuseFresnelParameters{this,
                               &StandardMaterial::get_diffuseFresnelParameters,
                               &StandardMaterial::set_diffuseFresnelParameters}
    , opacityFresnelParameters{this,
                               &StandardMaterial::get_opacityFresnelParameters,
                               &StandardMaterial::set_opacityFresnelParameters}
    , reflectionFresnelParameters{this,
                                  &StandardMaterial::
                                    get_reflectionFresnelParameters,
                                  &StandardMaterial::
                                    set_reflectionFresnelParameters}
    , refractionFresnelParameters{this,
                                  &StandardMaterial::
                                    get_refractionFresnelParameters,
                                  &StandardMaterial::
                                    set_refractionFresnelParameters}
    , emissiveFresnelParameters{this,
                                &StandardMaterial::
                                  get_emissiveFresnelParameters,
                                &StandardMaterial::
                                  set_emissiveFresnelParameters}
    , useReflectionFresnelFromSpecular{this,
                                       &StandardMaterial::
                                         get_useReflectionFresnelFromSpecular,
                                       &StandardMaterial::
                                         set_useReflectionFresnelFromSpecular}
    , useGlossinessFromSpecularMapAlpha{this,
                                        &StandardMaterial::
                                          get_useGlossinessFromSpecularMapAlpha,
                                        &StandardMaterial::
                                          set_useGlossinessFromSpecularMapAlpha}
    , maxSimultaneousLights{this, &StandardMaterial::get_maxSimultaneousLights,
                            &StandardMaterial::set_maxSimultaneousLights}
    , invertNormalMapX{this, &StandardMaterial::get_invertNormalMapX,
                       &StandardMaterial::set_invertNormalMapX}
    , invertNormalMapY{this, &StandardMaterial::get_invertNormalMapY,
                       &StandardMaterial::set_invertNormalMapY}
    , twoSidedLighting{this, &StandardMaterial::get_twoSidedLighting,
                       &StandardMaterial::set_twoSidedLighting}
    , customShaderNameResolve{nullptr}
    , imageProcessingConfiguration{this,
                                   &StandardMaterial::
                                     get_imageProcessingConfiguration,
                                   &StandardMaterial::
                                     set_imageProcessingConfiguration}
    , cameraColorCurvesEnabled{this,
                               &StandardMaterial::get_cameraColorCurvesEnabled,
                               &StandardMaterial::set_cameraColorCurvesEnabled}
    , cameraColorGradingEnabled{this,
                                &StandardMaterial::
                                  get_cameraColorGradingEnabled,
                                &StandardMaterial::
                                  set_cameraColorGradingEnabled}
    , cameraToneMappingEnabled{this,
                               &StandardMaterial::get_cameraToneMappingEnabled,
                               &StandardMaterial::set_cameraToneMappingEnabled}
    , cameraExposure{this, &StandardMaterial::get_cameraExposure,
                     &StandardMaterial::set_cameraExposure}
    , cameraContrast{this, &StandardMaterial::get_cameraContrast,
                     &StandardMaterial::set_cameraContrast}
    , cameraColorGradingTexture{this,
                                &StandardMaterial::
                                  get_cameraColorGradingTexture,
                                &StandardMaterial::
                                  set_cameraColorGradingTexture}
    , cameraColorCurves{this, &StandardMaterial::get_cameraColorCurves,
                        &StandardMaterial::set_cameraColorCurves}
    , _worldViewProjectionMatrix{Matrix::Zero()}
    , _globalAmbientColor{Color3(0.f, 0.f, 0.f)}
    , _useLogarithmicDepth{false}
    , _imageProcessingConfiguration{nullptr}
    , _diffuseTexture{nullptr}
    , _ambientTexture{nullptr}
    , _opacityTexture{nullptr}
    , _reflectionTexture{nullptr}
    , _emissiveTexture{nullptr}
    , _specularTexture{nullptr}
    , _bumpTexture{nullptr}
    , _lightmapTexture{nullptr}
    , _refractionTexture{nullptr}
    , _useAlphaFromDiffuseTexture{false}
    , _useEmissiveAsIllumination{false}
    , _linkEmissiveWithDiffuse{false}
    , _useReflectionFresnelFromSpecular{false}
    , _useSpecularOverAlpha{false}
    , _useReflectionOverAlpha{false}
    , _disableLighting{false}
    , _useObjectSpaceNormalMap{false}
    , _useParallax{false}
    , _useParallaxOcclusion{false}
    , _roughness{0.f}
    , _useLightmapAsShadowmap{false}
    , _diffuseFresnelParameters{nullptr}
    , _opacityFresnelParameters{nullptr}
    , _reflectionFresnelParameters{nullptr}
    , _refractionFresnelParameters{nullptr}
    , _emissiveFresnelParameters{nullptr}
    , _useGlossinessFromSpecularMapAlpha{false}
    , _maxSimultaneousLights{4}
    , _invertNormalMapX{false}
    , _invertNormalMapY{false}
    , _twoSidedLighting{false}
    , _imageProcessingObserver{nullptr}
{
  // Setup the default processing configuration to the scene.
  _attachImageProcessingConfiguration(nullptr);

  getRenderTargetTextures = [this]() {
    _renderTargets.clear();

    if (StandardMaterial::ReflectionTextureEnabled() && _reflectionTexture
        && _reflectionTexture->isRenderTarget) {
      _renderTargets.emplace_back(
        std::static_pointer_cast<RenderTargetTexture>(_reflectionTexture));
    }

    if (StandardMaterial::RefractionTextureEnabled() && _refractionTexture
        && _refractionTexture->isRenderTarget) {
      _renderTargets.emplace_back(
        std::static_pointer_cast<RenderTargetTexture>(_refractionTexture));
    }

    return _renderTargets;
  };
}

StandardMaterial::StandardMaterial(const StandardMaterial& other)
    : PushMaterial{other.name, other.getScene()}
    , diffuseTexture{this, &StandardMaterial::get_diffuseTexture,
                     &StandardMaterial::set_diffuseTexture}
    , ambientTexture{this, &StandardMaterial::get_ambientTexture,
                     &StandardMaterial::set_ambientTexture}
    , opacityTexture{this, &StandardMaterial::get_opacityTexture,
                     &StandardMaterial::set_opacityTexture}
    , reflectionTexture{this, &StandardMaterial::get_reflectionTexture,
                        &StandardMaterial::set_reflectionTexture}
    , emissiveTexture{this, &StandardMaterial::get_emissiveTexture,
                      &StandardMaterial::set_emissiveTexture}
    , specularTexture{this, &StandardMaterial::get_specularTexture,
                      &StandardMaterial::set_specularTexture}
    , bumpTexture{this, &StandardMaterial::get_bumpTexture,
                  &StandardMaterial::set_bumpTexture}
    , lightmapTexture{this, &StandardMaterial::get_lightmapTexture,
                      &StandardMaterial::set_lightmapTexture}
    , refractionTexture{this, &StandardMaterial::get_refractionTexture,
                        &StandardMaterial::set_refractionTexture}
    , useAlphaFromDiffuseTexture{this,
                                 &StandardMaterial::
                                   get_useAlphaFromDiffuseTexture,
                                 &StandardMaterial::
                                   set_useAlphaFromDiffuseTexture}
    , useEmissiveAsIllumination{this,
                                &StandardMaterial::
                                  get_useEmissiveAsIllumination,
                                &StandardMaterial::
                                  set_useEmissiveAsIllumination}
    , linkEmissiveWithDiffuse{this,
                              &StandardMaterial::get_linkEmissiveWithDiffuse,
                              &StandardMaterial::set_linkEmissiveWithDiffuse}
    , useSpecularOverAlpha{this, &StandardMaterial::get_useSpecularOverAlpha,
                           &StandardMaterial::set_useSpecularOverAlpha}
    , useReflectionOverAlpha{this,
                             &StandardMaterial::get_useReflectionOverAlpha,
                             &StandardMaterial::set_useReflectionOverAlpha}
    , disableLighting{this, &StandardMaterial::get_disableLighting,
                      &StandardMaterial::set_disableLighting}
    , useObjectSpaceNormalMap{this,
                              &StandardMaterial::get_useObjectSpaceNormalMap,
                              &StandardMaterial::set_useObjectSpaceNormalMap}
    , useParallax{this, &StandardMaterial::get_useParallax,
                  &StandardMaterial::set_useParallax}
    , useParallaxOcclusion{this, &StandardMaterial::get_useParallaxOcclusion,
                           &StandardMaterial::set_useParallaxOcclusion}
    , roughness{this, &StandardMaterial::get_roughness,
                &StandardMaterial::set_roughness}
    , useLightmapAsShadowmap{this,
                             &StandardMaterial::get_useLightmapAsShadowmap,
                             &StandardMaterial::set_useLightmapAsShadowmap}
    , diffuseFresnelParameters{this,
                               &StandardMaterial::get_diffuseFresnelParameters,
                               &StandardMaterial::set_diffuseFresnelParameters}
    , opacityFresnelParameters{this,
                               &StandardMaterial::get_opacityFresnelParameters,
                               &StandardMaterial::set_opacityFresnelParameters}
    , reflectionFresnelParameters{this,
                                  &StandardMaterial::
                                    get_reflectionFresnelParameters,
                                  &StandardMaterial::
                                    set_reflectionFresnelParameters}
    , refractionFresnelParameters{this,
                                  &StandardMaterial::
                                    get_refractionFresnelParameters,
                                  &StandardMaterial::
                                    set_refractionFresnelParameters}
    , emissiveFresnelParameters{this,
                                &StandardMaterial::
                                  get_emissiveFresnelParameters,
                                &StandardMaterial::
                                  set_emissiveFresnelParameters}
    , useReflectionFresnelFromSpecular{this,
                                       &StandardMaterial::
                                         get_useReflectionFresnelFromSpecular,
                                       &StandardMaterial::
                                         set_useReflectionFresnelFromSpecular}
    , useGlossinessFromSpecularMapAlpha{this,
                                        &StandardMaterial::
                                          get_useGlossinessFromSpecularMapAlpha,
                                        &StandardMaterial::
                                          set_useGlossinessFromSpecularMapAlpha}
    , maxSimultaneousLights{this, &StandardMaterial::get_maxSimultaneousLights,
                            &StandardMaterial::set_maxSimultaneousLights}
    , invertNormalMapX{this, &StandardMaterial::get_invertNormalMapX,
                       &StandardMaterial::set_invertNormalMapX}
    , invertNormalMapY{this, &StandardMaterial::get_invertNormalMapY,
                       &StandardMaterial::set_invertNormalMapY}
    , twoSidedLighting{this, &StandardMaterial::get_twoSidedLighting,
                       &StandardMaterial::set_twoSidedLighting}
    , imageProcessingConfiguration{this,
                                   &StandardMaterial::
                                     get_imageProcessingConfiguration,
                                   &StandardMaterial::
                                     set_imageProcessingConfiguration}
    , cameraColorCurvesEnabled{this,
                               &StandardMaterial::get_cameraColorCurvesEnabled,
                               &StandardMaterial::set_cameraColorCurvesEnabled}
    , cameraColorGradingEnabled{this,
                                &StandardMaterial::
                                  get_cameraColorGradingEnabled,
                                &StandardMaterial::
                                  set_cameraColorGradingEnabled}
    , cameraToneMappingEnabled{this,
                               &StandardMaterial::get_cameraToneMappingEnabled,
                               &StandardMaterial::set_cameraToneMappingEnabled}
    , cameraExposure{this, &StandardMaterial::get_cameraExposure,
                     &StandardMaterial::set_cameraExposure}
    , cameraContrast{this, &StandardMaterial::get_cameraContrast,
                     &StandardMaterial::set_cameraContrast}
    , cameraColorGradingTexture{this,
                                &StandardMaterial::
                                  get_cameraColorGradingTexture,
                                &StandardMaterial::
                                  set_cameraColorGradingTexture}
    , cameraColorCurves{this, &StandardMaterial::get_cameraColorCurves,
                        &StandardMaterial::set_cameraColorCurves}
{
  // Base material
  other.copyTo(dynamic_cast<PushMaterial*>(this));

  // Standard material
  ambientColor            = other.ambientColor;
  diffuseColor            = other.diffuseColor;
  specularColor           = other.specularColor;
  emissiveColor           = other.emissiveColor;
  specularPower           = other.specularPower;
  parallaxScaleBias       = other.parallaxScaleBias;
  indexOfRefraction       = other.indexOfRefraction;
  invertRefractionY       = other.invertRefractionY;
  customShaderNameResolve = other.customShaderNameResolve;

  _renderTargets             = other._renderTargets;
  _worldViewProjectionMatrix = other._worldViewProjectionMatrix;
  _globalAmbientColor        = other._globalAmbientColor;
  _useLogarithmicDepth       = other._useLogarithmicDepth;

  _diffuseTexture                   = other._diffuseTexture;
  _ambientTexture                   = other._ambientTexture;
  _opacityTexture                   = other._ambientTexture;
  _reflectionTexture                = other._reflectionTexture;
  _emissiveTexture                  = other._emissiveTexture;
  _specularTexture                  = other._specularTexture;
  _bumpTexture                      = other._bumpTexture;
  _lightmapTexture                  = other._lightmapTexture;
  _refractionTexture                = other._refractionTexture;
  _useAlphaFromDiffuseTexture       = other._useAlphaFromDiffuseTexture;
  _useEmissiveAsIllumination        = other._useEmissiveAsIllumination;
  _linkEmissiveWithDiffuse          = other._linkEmissiveWithDiffuse;
  _useReflectionFresnelFromSpecular = other._useReflectionFresnelFromSpecular;
  _useSpecularOverAlpha             = other._useSpecularOverAlpha;
  _useReflectionOverAlpha           = other._useReflectionOverAlpha;
  _disableLighting                  = other._disableLighting;
  _useParallax                      = other._useParallax;
  _useParallaxOcclusion             = other._useParallaxOcclusion;
  _roughness                        = other._roughness;
  _useLightmapAsShadowmap           = other._useLightmapAsShadowmap;

  if (other._diffuseFresnelParameters) {
    _diffuseFresnelParameters = other._diffuseFresnelParameters->clone();
  }
  if (other._opacityFresnelParameters) {
    _opacityFresnelParameters = other._opacityFresnelParameters->clone();
  }
  if (other._reflectionFresnelParameters) {
    _reflectionFresnelParameters = other._reflectionFresnelParameters->clone();
  }
  if (other._refractionFresnelParameters) {
    _refractionFresnelParameters = other._refractionFresnelParameters->clone();
  }
  if (other._emissiveFresnelParameters) {
    _emissiveFresnelParameters = other._emissiveFresnelParameters->clone();
  }

  _useGlossinessFromSpecularMapAlpha = other._useGlossinessFromSpecularMapAlpha;
  _maxSimultaneousLights             = other._maxSimultaneousLights;
  _invertNormalMapX                  = other._invertNormalMapX;
  _invertNormalMapY                  = other._invertNormalMapY;
  _twoSidedLighting                  = other._twoSidedLighting;
}

StandardMaterial::~StandardMaterial()
{
}

const std::string StandardMaterial::getClassName() const
{
  return "StandardMaterial";
}

IReflect::Type StandardMaterial::type() const
{
  return IReflect::Type::STANDARDMATERIAL;
}

bool StandardMaterial::get_hasRenderTargetTextures() const
{
  if (StandardMaterial::ReflectionTextureEnabled() && _reflectionTexture
      && _reflectionTexture->isRenderTarget) {
    return true;
  }

  if (StandardMaterial::RefractionTextureEnabled() && _refractionTexture
      && _refractionTexture->isRenderTarget) {
    return true;
  }

  return false;
}

bool StandardMaterial::get_useLogarithmicDepth() const
{
  return _useLogarithmicDepth;
}

void StandardMaterial::set_useLogarithmicDepth(bool value)
{
  _useLogarithmicDepth
    = value && getScene()->getEngine()->getCaps().fragmentDepthSupported;
  _markAllSubMeshesAsMiscDirty();
}

bool StandardMaterial::needAlphaBlending() const
{
  return (alpha() < 1.f) || (_opacityTexture != nullptr)
         || _shouldUseAlphaFromDiffuseTexture()
         || (_opacityFresnelParameters
             && _opacityFresnelParameters->isEnabled());
}

bool StandardMaterial::needAlphaTesting() const
{
  return _diffuseTexture != nullptr && _diffuseTexture->hasAlpha();
}

bool StandardMaterial::_shouldUseAlphaFromDiffuseTexture() const
{
  return _diffuseTexture != nullptr && _diffuseTexture->hasAlpha()
         && _useAlphaFromDiffuseTexture;
}

BaseTexturePtr StandardMaterial::getAlphaTestTexture()
{
  return _diffuseTexture;
}

bool StandardMaterial::isReadyForSubMesh(AbstractMesh* mesh,
                                         BaseSubMesh* subMesh,
                                         bool useInstances)
{
  if (subMesh->effect() && isFrozen()) {
    if (_wasPreviouslyReady) {
      return true;
    }
  }

  if (!subMesh->_materialDefines) {
    subMesh->_materialDefines = std::make_unique<StandardMaterialDefines>();
  }

  auto scene = getScene();
  auto defines
    = *(static_cast<StandardMaterialDefines*>(subMesh->_materialDefines.get()));
  if (!checkReadyOnEveryCall && subMesh->effect()) {
    if (defines._renderId == scene->getRenderId()) {
      return true;
    }
  }

  auto engine = scene->getEngine();

  // Lights
  defines._needNormals = MaterialHelper::PrepareDefinesForLights(
    scene, mesh, defines, true, _maxSimultaneousLights, _disableLighting);

  // Textures
  if (defines._areTexturesDirty) {
    defines._needUVs           = false;
    defines.boolDef["MAINUV1"] = false;
    defines.boolDef["MAINUV2"] = false;
    if (scene->texturesEnabled()) {
      if (_diffuseTexture && StandardMaterial::DiffuseTextureEnabled()) {
        if (!_diffuseTexture->isReadyOrNotBlocking()) {
          return false;
        }
        else {
          MaterialHelper::PrepareDefinesForMergedUV(_diffuseTexture, defines,
                                                    "DIFFUSE");
        }
      }
      else {
        defines.boolDef["DIFFUSE"] = false;
      }

      if (_ambientTexture && StandardMaterial::AmbientTextureEnabled()) {
        if (!_ambientTexture->isReadyOrNotBlocking()) {
          return false;
        }
        else {
          MaterialHelper::PrepareDefinesForMergedUV(_ambientTexture, defines,
                                                    "AMBIENT");
        }
      }
      else {
        defines.boolDef["AMBIENT"] = false;
      }

      if (_opacityTexture && StandardMaterial::OpacityTextureEnabled()) {
        if (!_opacityTexture->isReadyOrNotBlocking()) {
          return false;
        }
        else {
          MaterialHelper::PrepareDefinesForMergedUV(_opacityTexture, defines,
                                                    "OPACITY");
          defines.boolDef["OPACITYRGB"] = _opacityTexture->getAlphaFromRGB;
        }
      }
      else {
        defines.boolDef["OPACITY"] = false;
      }

      if (_reflectionTexture && StandardMaterial::ReflectionTextureEnabled()) {
        if (!_reflectionTexture->isReadyOrNotBlocking()) {
          return false;
        }
        else {
          defines._needNormals          = true;
          defines.boolDef["REFLECTION"] = true;

          defines.boolDef["ROUGHNESS"]           = (_roughness > 0);
          defines.boolDef["REFLECTIONOVERALPHA"] = _useReflectionOverAlpha;
          defines.boolDef["INVERTCUBICMAP"]
            = (_reflectionTexture->coordinatesMode()
               == TextureConstants::INVCUBIC_MODE);
          defines.boolDef["REFLECTIONMAP_3D"] = _reflectionTexture->isCube;

          switch (_reflectionTexture->coordinatesMode()) {
            case TextureConstants::EXPLICIT_MODE:
              defines.setReflectionMode("REFLECTIONMAP_EXPLICIT");
              break;
            case TextureConstants::PLANAR_MODE:
              defines.setReflectionMode("REFLECTIONMAP_PLANAR");
              break;
            case TextureConstants::PROJECTION_MODE:
              defines.setReflectionMode("REFLECTIONMAP_PROJECTION");
              break;
            case TextureConstants::SKYBOX_MODE:
              defines.setReflectionMode("REFLECTIONMAP_SKYBOX");
              defines.boolDef["REFLECTIONMAP_SKYBOX_TRANSFORMED"]
                = !_reflectionTexture->getReflectionTextureMatrix()
                     ->isIdentity();
              break;
            case TextureConstants::SPHERICAL_MODE:
              defines.setReflectionMode("REFLECTIONMAP_SPHERICAL");
              break;
            case TextureConstants::EQUIRECTANGULAR_MODE:
              defines.setReflectionMode("REFLECTIONMAP_EQUIRECTANGULAR");
              break;
            case TextureConstants::FIXED_EQUIRECTANGULAR_MODE:
              defines.setReflectionMode("REFLECTIONMAP_EQUIRECTANGULAR_FIXED");
              break;
            case TextureConstants::FIXED_EQUIRECTANGULAR_MIRRORED_MODE:
              defines.setReflectionMode(
                "REFLECTIONMAP_MIRROREDEQUIRECTANGULAR_FIXED");
              break;
            case TextureConstants::CUBIC_MODE:
            case TextureConstants::INVCUBIC_MODE:
              defines.setReflectionMode("REFLECTIONMAP_CUBIC");
              break;
          }

          defines.boolDef["USE_LOCAL_REFLECTIONMAP_CUBIC"]
            = _reflectionTexture->boundingBoxSize() ? true : false;
        }
      }
      else {
        defines.boolDef["REFLECTION"] = false;
      }

      if (_emissiveTexture && StandardMaterial::EmissiveTextureEnabled()) {
        if (!_emissiveTexture->isReadyOrNotBlocking()) {
          return false;
        }
        else {
          MaterialHelper::PrepareDefinesForMergedUV(_emissiveTexture, defines,
                                                    "EMISSIVE");
        }
      }
      else {
        defines.boolDef["EMISSIVE"] = false;
      }

      if (_lightmapTexture && StandardMaterial::LightmapTextureEnabled()) {
        if (!_lightmapTexture->isReadyOrNotBlocking()) {
          return false;
        }
        else {
          MaterialHelper::PrepareDefinesForMergedUV(_lightmapTexture, defines,
                                                    "LIGHTMAP");
          defines.boolDef["USELIGHTMAPASSHADOWMAP"] = _useLightmapAsShadowmap;
        }
      }
      else {
        defines.boolDef["LIGHTMAP"] = false;
      }

      if (_specularTexture && StandardMaterial::SpecularTextureEnabled()) {
        if (!_specularTexture->isReadyOrNotBlocking()) {
          return false;
        }
        else {
          MaterialHelper::PrepareDefinesForMergedUV(_specularTexture, defines,
                                                    "SPECULAR");
          defines.boolDef["GLOSSINESS"] = _useGlossinessFromSpecularMapAlpha;
        }
      }
      else {
        defines.boolDef["SPECULAR"] = false;
      }

      if (scene->getEngine()->getCaps().standardDerivatives && _bumpTexture
          && StandardMaterial::BumpTextureEnabled()) {
        // Bump texure can not be not blocking.
        if (!_bumpTexture->isReady()) {
          return false;
        }
        else {
          MaterialHelper::PrepareDefinesForMergedUV(_bumpTexture, defines,
                                                    "BUMP");

          defines.boolDef["PARALLAX"]          = _useParallax;
          defines.boolDef["PARALLAXOCCLUSION"] = _useParallaxOcclusion;
        }

        defines.boolDef["OBJECTSPACE_NORMALMAP"] = _useObjectSpaceNormalMap;
      }
      else {
        defines.boolDef["BUMP"] = false;
      }

      if (_refractionTexture && StandardMaterial::RefractionTextureEnabled()) {
        if (!_refractionTexture->isReadyOrNotBlocking()) {
          return false;
        }
        else {
          defines._needUVs              = true;
          defines.boolDef["REFRACTION"] = true;

          defines.boolDef["REFRACTIONMAP_3D"] = _refractionTexture->isCube;
        }
      }
      else {
        defines.boolDef["REFRACTION"] = false;
      }

      defines.boolDef["TWOSIDEDLIGHTING"]
        = !_backFaceCulling && _twoSidedLighting;
    }
    else {
      defines.boolDef["DIFFUSE"]    = false;
      defines.boolDef["AMBIENT"]    = false;
      defines.boolDef["OPACITY"]    = false;
      defines.boolDef["REFLECTION"] = false;
      defines.boolDef["EMISSIVE"]   = false;
      defines.boolDef["LIGHTMAP"]   = false;
      defines.boolDef["BUMP"]       = false;
      defines.boolDef["REFRACTION"] = false;
    }

    defines.boolDef["ALPHAFROMDIFFUSE"] = _shouldUseAlphaFromDiffuseTexture();

    defines.boolDef["EMISSIVEASILLUMINATION"] = _useEmissiveAsIllumination;

    defines.boolDef["LINKEMISSIVEWITHDIFFUSE"] = _linkEmissiveWithDiffuse;

    defines.boolDef["SPECULAROVERALPHA"] = _useSpecularOverAlpha;

    defines.boolDef["PREMULTIPLYALPHA"]
      = (alphaMode() == EngineConstants::ALPHA_PREMULTIPLIED
         || alphaMode() == EngineConstants::ALPHA_PREMULTIPLIED_PORTERDUFF);
  }

  if (defines._areImageProcessingDirty && _imageProcessingConfiguration) {
    if (!_imageProcessingConfiguration->isReady()) {
      return false;
    }

    _imageProcessingConfiguration->prepareDefines(defines);

    defines.boolDef["IS_REFLECTION_LINEAR"]
      = (reflectionTexture() != nullptr && !reflectionTexture()->gammaSpace);
    defines.boolDef["IS_REFRACTION_LINEAR"]
      = (refractionTexture() != nullptr && !refractionTexture()->gammaSpace);
  }

  if (defines._areFresnelDirty) {
    if (StandardMaterial::FresnelEnabled()) {
      // Fresnel
      if (_diffuseFresnelParameters || _opacityFresnelParameters
          || _emissiveFresnelParameters || _refractionFresnelParameters
          || _reflectionFresnelParameters) {

        defines.boolDef["DIFFUSEFRESNEL"]
          = (_diffuseFresnelParameters
             && _diffuseFresnelParameters->isEnabled());

        defines.boolDef["OPACITYFRESNEL"]
          = (_opacityFresnelParameters
             && _opacityFresnelParameters->isEnabled());

        defines.boolDef["REFLECTIONFRESNEL"]
          = (_reflectionFresnelParameters
             && _reflectionFresnelParameters->isEnabled());

        defines.boolDef["REFLECTIONFRESNELFROMSPECULAR"]
          = _useReflectionFresnelFromSpecular;

        defines.boolDef["REFRACTIONFRESNEL"]
          = (_refractionFresnelParameters
             && _refractionFresnelParameters->isEnabled());

        defines.boolDef["EMISSIVEFRESNEL"]
          = (_emissiveFresnelParameters
             && _emissiveFresnelParameters->isEnabled());

        defines._needNormals       = true;
        defines.boolDef["FRESNEL"] = true;
      }
    }
    else {
      defines.boolDef["FRESNEL"] = false;
    }
  }

  // Misc.
  MaterialHelper::PrepareDefinesForMisc(mesh, scene, _useLogarithmicDepth,
                                        pointsCloud(), fogEnabled(),
                                        _shouldTurnAlphaTestOn(mesh), defines);

  // Attribs
  MaterialHelper::PrepareDefinesForAttributes(mesh, defines, true, true, true,
                                              true);

  // Values that need to be evaluated on every frame
  MaterialHelper::PrepareDefinesForFrameBoundValues(scene, engine, defines,
                                                    useInstances);

  // Get correct effect
  if (defines.isDirty()) {
    defines.markAsProcessed();
    scene->resetCachedMaterial();

    // Fallbacks
    auto fallbacks = std::make_unique<EffectFallbacks>();
    if (defines["REFLECTION"]) {
      fallbacks->addFallback(0, "REFLECTION");
    }

    if (defines["SPECULAR"]) {
      fallbacks->addFallback(0, "SPECULAR");
    }

    if (defines["BUMP"]) {
      fallbacks->addFallback(0, "BUMP");
    }

    if (defines["PARALLAX"]) {
      fallbacks->addFallback(1, "PARALLAX");
    }

    if (defines["PARALLAXOCCLUSION"]) {
      fallbacks->addFallback(0, "PARALLAXOCCLUSION");
    }

    if (defines["SPECULAROVERALPHA"]) {
      fallbacks->addFallback(0, "SPECULAROVERALPHA");
    }

    if (defines["FOG"]) {
      fallbacks->addFallback(1, "FOG");
    }

    if (defines["POINTSIZE"]) {
      fallbacks->addFallback(0, "POINTSIZE");
    }

    if (defines["LOGARITHMICDEPTH"]) {
      fallbacks->addFallback(0, "LOGARITHMICDEPTH");
    }

    MaterialHelper::HandleFallbacksForShadows(defines, *fallbacks,
                                              _maxSimultaneousLights);

    if (defines["SPECULARTERM"]) {
      fallbacks->addFallback(0, "SPECULARTERM");
    }

    if (defines["DIFFUSEFRESNEL"]) {
      fallbacks->addFallback(1, "DIFFUSEFRESNEL");
    }

    if (defines["OPACITYFRESNEL"]) {
      fallbacks->addFallback(2, "OPACITYFRESNEL");
    }

    if (defines["REFLECTIONFRESNEL"]) {
      fallbacks->addFallback(3, "REFLECTIONFRESNEL");
    }

    if (defines["EMISSIVEFRESNEL"]) {
      fallbacks->addFallback(4, "EMISSIVEFRESNEL");
    }

    if (defines["FRESNEL"]) {
      fallbacks->addFallback(4, "FRESNEL");
    }

    // Attributes
    std::vector<std::string> attribs{VertexBuffer::PositionKindChars};

    if (defines["NORMAL"]) {
      attribs.emplace_back(VertexBuffer::NormalKindChars);
    }

    if (defines["UV1"]) {
      attribs.emplace_back(VertexBuffer::UVKindChars);
    }

    if (defines["UV2"]) {
      attribs.emplace_back(VertexBuffer::UV2KindChars);
    }

    if (defines["VERTEXCOLOR"]) {
      attribs.emplace_back(VertexBuffer::ColorKindChars);
    }

    MaterialHelper::PrepareAttributesForBones(attribs, mesh, defines,
                                              *fallbacks);
    MaterialHelper::PrepareAttributesForInstances(attribs, defines);
    MaterialHelper::PrepareAttributesForMorphTargets(attribs, mesh, defines);

    std::string shaderName{"default"};
    const auto join = defines.toString();
    std::vector<std::string> uniforms{"world",
                                      "view",
                                      "viewProjection",
                                      "vEyePosition",
                                      "vLightsType",
                                      "vAmbientColor",
                                      "vDiffuseColor",
                                      "vSpecularColor",
                                      "vEmissiveColor",
                                      "vFogInfos",
                                      "vFogColor",
                                      "pointSize",
                                      "vDiffuseInfos",
                                      "vAmbientInfos",
                                      "vOpacityInfos",
                                      "vReflectionInfos",
                                      "vEmissiveInfos",
                                      "vSpecularInfos",
                                      "vBumpInfos",
                                      "vLightmapInfos",
                                      "vRefractionInfos",
                                      "mBones",
                                      "vClipPlane",
                                      "vClipPlane2",
                                      "vClipPlane3",
                                      "vClipPlane4",
                                      "diffuseMatrix",
                                      "ambientMatrix",
                                      "opacityMatrix",
                                      "reflectionMatrix",
                                      "emissiveMatrix",
                                      "specularMatrix",
                                      "bumpMatrix",
                                      "normalMatrix",
                                      "lightmapMatrix",
                                      "refractionMatrix",
                                      "diffuseLeftColor",
                                      "diffuseRightColor",
                                      "opacityParts",
                                      "reflectionLeftColor",
                                      "reflectionRightColor",
                                      "emissiveLeftColor",
                                      "emissiveRightColor",
                                      "refractionLeftColor",
                                      "refractionRightColor",
                                      "vReflectionPosition",
                                      "vReflectionSize",
                                      "logarithmicDepthConstant",
                                      "vTangentSpaceParams",
                                      "alphaCutOff"};
    std::vector<std::string> samplers{
      "diffuseSampler",        "ambientSampler",      "opacitySampler",
      "reflectionCubeSampler", "reflection2DSampler", "emissiveSampler",
      "specularSampler",       "bumpSampler",         "lightmapSampler",
      "refractionCubeSampler", "refraction2DSampler"};
    std::vector<std::string> uniformBuffers{"Material", "Scene"};

    ImageProcessingConfiguration::PrepareUniforms(uniforms, defines);
    ImageProcessingConfiguration::PrepareSamplers(samplers, defines);

    std::unordered_map<std::string, unsigned int> indexParameters{
      {"maxSimultaneousLights", _maxSimultaneousLights},
      {"maxSimultaneousMorphTargets", defines.intDef["NUM_MORPH_INFLUENCERS"]}};

    EffectCreationOptions options;
    options.attributes            = std::move(attribs);
    options.uniformsNames         = std::move(uniforms);
    options.uniformBuffersNames   = std::move(uniformBuffers);
    options.samplers              = std::move(samplers);
    options.materialDefines       = &defines;
    options.defines               = std::move(join);
    options.fallbacks             = std::move(fallbacks);
    options.onCompiled            = onCompiled;
    options.onError               = onError;
    options.indexParameters       = std::move(indexParameters);
    options.maxSimultaneousLights = _maxSimultaneousLights;

    MaterialHelper::PrepareUniformsAndSamplersList(options);

    if (customShaderNameResolve) {
      shaderName = customShaderNameResolve(shaderName, uniforms, uniformBuffers,
                                           samplers, defines);
    }

    subMesh->setEffect(
      scene->getEngine()->createEffect(shaderName, options, engine), defines);

    buildUniformLayout();
  }

  if (!subMesh->effect() || !subMesh->effect()->isReady()) {
    return false;
  }

  defines._renderId   = scene->getRenderId();
  _wasPreviouslyReady = true;

  return true;
}

void StandardMaterial::buildUniformLayout()
{
  // Order is important !
  _uniformBuffer->addUniform("diffuseLeftColor", 4);
  _uniformBuffer->addUniform("diffuseRightColor", 4);
  _uniformBuffer->addUniform("opacityParts", 4);
  _uniformBuffer->addUniform("reflectionLeftColor", 4);
  _uniformBuffer->addUniform("reflectionRightColor", 4);
  _uniformBuffer->addUniform("refractionLeftColor", 4);
  _uniformBuffer->addUniform("refractionRightColor", 4);
  _uniformBuffer->addUniform("emissiveLeftColor", 4);
  _uniformBuffer->addUniform("emissiveRightColor", 4);

  _uniformBuffer->addUniform("vDiffuseInfos", 2);
  _uniformBuffer->addUniform("vAmbientInfos", 2);
  _uniformBuffer->addUniform("vOpacityInfos", 2);
  _uniformBuffer->addUniform("vReflectionInfos", 2);
  _uniformBuffer->addUniform("vReflectionPosition", 3);
  _uniformBuffer->addUniform("vReflectionSize", 3);
  _uniformBuffer->addUniform("vEmissiveInfos", 2);
  _uniformBuffer->addUniform("vLightmapInfos", 2);
  _uniformBuffer->addUniform("vSpecularInfos", 2);
  _uniformBuffer->addUniform("vBumpInfos", 3);

  _uniformBuffer->addUniform("diffuseMatrix", 16);
  _uniformBuffer->addUniform("ambientMatrix", 16);
  _uniformBuffer->addUniform("opacityMatrix", 16);
  _uniformBuffer->addUniform("reflectionMatrix", 16);
  _uniformBuffer->addUniform("emissiveMatrix", 16);
  _uniformBuffer->addUniform("lightmapMatrix", 16);
  _uniformBuffer->addUniform("specularMatrix", 16);
  _uniformBuffer->addUniform("bumpMatrix", 16);
  _uniformBuffer->addUniform("vTangentSpaceParams", 2);
  _uniformBuffer->addUniform("refractionMatrix", 16);
  _uniformBuffer->addUniform("vRefractionInfos", 4);
  _uniformBuffer->addUniform("vSpecularColor", 4);
  _uniformBuffer->addUniform("vEmissiveColor", 3);
  _uniformBuffer->addUniform("vDiffuseColor", 4);
  _uniformBuffer->addUniform("pointSize", 1);

  _uniformBuffer->create();
}

void StandardMaterial::unbind()
{
  if (_activeEffect) {
    auto needFlag = false;
    if (_reflectionTexture && _reflectionTexture->isRenderTarget) {
      _activeEffect->setTexture("reflection2DSampler", nullptr);
      needFlag = true;
    }

    if (_refractionTexture && _refractionTexture->isRenderTarget) {
      _activeEffect->setTexture("refraction2DSampler", nullptr);
      needFlag = true;
    }

    if (needFlag) {
      _markAllSubMeshesAsTexturesDirty();
    }
  }

  PushMaterial::unbind();
}

void StandardMaterial::bindForSubMesh(Matrix& world, Mesh* mesh,
                                      SubMesh* subMesh)
{
  auto scene = getScene();

  auto definesTmp
    = static_cast<StandardMaterialDefines*>(subMesh->_materialDefines.get());
  if (!definesTmp) {
    return;
  }
  auto defines = *definesTmp;

  auto effect = subMesh->effect();
  if (!effect) {
    return;
  }
  _activeEffect = effect;

  // Matrices
  bindOnlyWorldMatrix(world);

  // Normal Matrix
  if (defines["OBJECTSPACE_NORMALMAP"]) {
    world.toNormalMatrix(_normalMatrix);
    bindOnlyNormalMatrix(_normalMatrix);
  }

  const auto mustRebind = _mustRebind(scene, effect, mesh->visibility());

  // Bones
  MaterialHelper::BindBonesParameters(mesh, effect);
  if (mustRebind) {
    _uniformBuffer->bindToEffect(effect.get(), "Material");

    bindViewProjection(effect);
    if (!_uniformBuffer->useUbo() || !isFrozen() || !_uniformBuffer->isSync()) {

      if (StandardMaterial::FresnelEnabled() && defines["FRESNEL"]) {
        // Fresnel
        if (_diffuseFresnelParameters
            && _diffuseFresnelParameters->isEnabled()) {
          _uniformBuffer->updateColor4("diffuseLeftColor",
                                       _diffuseFresnelParameters->leftColor,
                                       _diffuseFresnelParameters->power, "");
          _uniformBuffer->updateColor4("diffuseRightColor",
                                       _diffuseFresnelParameters->rightColor,
                                       _diffuseFresnelParameters->bias, "");
        }

        if (_opacityFresnelParameters
            && _opacityFresnelParameters->isEnabled()) {
          _uniformBuffer->updateColor4(
            "opacityParts",
            Color3(_opacityFresnelParameters->leftColor.toLuminance(),
                   _opacityFresnelParameters->rightColor.toLuminance(),
                   _opacityFresnelParameters->bias),
            _opacityFresnelParameters->power, "");
        }

        if (_reflectionFresnelParameters
            && _reflectionFresnelParameters->isEnabled()) {
          _uniformBuffer->updateColor4("reflectionLeftColor",
                                       _reflectionFresnelParameters->leftColor,
                                       _reflectionFresnelParameters->power, "");
          _uniformBuffer->updateColor4("reflectionRightColor",
                                       _reflectionFresnelParameters->rightColor,
                                       _reflectionFresnelParameters->bias, "");
        }

        if (_refractionFresnelParameters
            && _refractionFresnelParameters->isEnabled()) {
          _uniformBuffer->updateColor4("refractionLeftColor",
                                       _refractionFresnelParameters->leftColor,
                                       _refractionFresnelParameters->power, "");
          _uniformBuffer->updateColor4("refractionRightColor",
                                       _refractionFresnelParameters->rightColor,
                                       _refractionFresnelParameters->bias, "");
        }

        if (_emissiveFresnelParameters
            && _emissiveFresnelParameters->isEnabled()) {
          _uniformBuffer->updateColor4("emissiveLeftColor",
                                       _emissiveFresnelParameters->leftColor,
                                       _emissiveFresnelParameters->power, "");
          _uniformBuffer->updateColor4("emissiveRightColor",
                                       _emissiveFresnelParameters->rightColor,
                                       _emissiveFresnelParameters->bias, "");
        }
      }

      // Textures
      if (scene->texturesEnabled()) {
        if (_diffuseTexture && StandardMaterial::DiffuseTextureEnabled()) {
          _uniformBuffer->updateFloat2(
            "vDiffuseInfos",
            static_cast<float>(_diffuseTexture->coordinatesIndex),
            static_cast<float>(_diffuseTexture->level), "");
          MaterialHelper::BindTextureMatrix(*_diffuseTexture, *_uniformBuffer,
                                            "diffuse");

          if (_diffuseTexture->hasAlpha()) {
            effect->setFloat("alphaCutOff", alphaCutOff);
          }
        }

        if (_ambientTexture && StandardMaterial::AmbientTextureEnabled()) {
          _uniformBuffer->updateFloat2(
            "vAmbientInfos",
            static_cast<float>(_ambientTexture->coordinatesIndex),
            static_cast<float>(_ambientTexture->level), "");
          MaterialHelper::BindTextureMatrix(*_ambientTexture, *_uniformBuffer,
                                            "ambient");
        }

        if (_opacityTexture && StandardMaterial::OpacityTextureEnabled()) {
          _uniformBuffer->updateFloat2(
            "vOpacityInfos",
            static_cast<float>(_opacityTexture->coordinatesIndex),
            static_cast<float>(_opacityTexture->level), "");
          MaterialHelper::BindTextureMatrix(*_opacityTexture, *_uniformBuffer,
                                            "opacity");
        }

        if (_reflectionTexture
            && StandardMaterial::ReflectionTextureEnabled()) {
          _uniformBuffer->updateFloat2(
            "vReflectionInfos", _reflectionTexture->level, _roughness, "");
          _uniformBuffer->updateMatrix(
            "reflectionMatrix",
            *_reflectionTexture->getReflectionTextureMatrix());

          if (_reflectionTexture->boundingBoxSize()) {
            if (auto cubeTexture
                = std::static_pointer_cast<CubeTexture>(_reflectionTexture)) {
              _uniformBuffer->updateVector3("vReflectionPosition",
                                            cubeTexture->boundingBoxPosition);
              _uniformBuffer->updateVector3("vReflectionSize",
                                            *cubeTexture->boundingBoxSize());
            }
          }
        }

        if (_emissiveTexture && StandardMaterial::EmissiveTextureEnabled()) {
          _uniformBuffer->updateFloat2(
            "vEmissiveInfos",
            static_cast<float>(_emissiveTexture->coordinatesIndex),
            static_cast<float>(_emissiveTexture->level), "");
          MaterialHelper::BindTextureMatrix(*_emissiveTexture, *_uniformBuffer,
                                            "emissive");
        }

        if (_lightmapTexture && StandardMaterial::LightmapTextureEnabled()) {
          _uniformBuffer->updateFloat2(
            "vLightmapInfos",
            static_cast<float>(_lightmapTexture->coordinatesIndex),
            static_cast<float>(_lightmapTexture->level), "");
          MaterialHelper::BindTextureMatrix(*_lightmapTexture, *_uniformBuffer,
                                            "lightmap");
        }

        if (_specularTexture && StandardMaterial::SpecularTextureEnabled()) {
          _uniformBuffer->updateFloat2(
            "vSpecularInfos",
            static_cast<float>(_specularTexture->coordinatesIndex),
            static_cast<float>(_specularTexture->level), "");
          MaterialHelper::BindTextureMatrix(*_specularTexture, *_uniformBuffer,
                                            "specular");
        }

        if (_bumpTexture && scene->getEngine()->getCaps().standardDerivatives
            && StandardMaterial::BumpTextureEnabled()) {
          _uniformBuffer->updateFloat3(
            "vBumpInfos", static_cast<float>(_bumpTexture->coordinatesIndex),
            1.f / _bumpTexture->level, parallaxScaleBias, "");
          MaterialHelper::BindTextureMatrix(*_bumpTexture, *_uniformBuffer,
                                            "bump");
          if (scene->_mirroredCameraPosition) {
            _uniformBuffer->updateFloat2("vTangentSpaceParams",
                                         _invertNormalMapX ? 1.f : -1.f,
                                         _invertNormalMapY ? 1.f : -1.f, "");
          }
          else {
            _uniformBuffer->updateFloat2("vTangentSpaceParams",
                                         _invertNormalMapX ? -1.f : 1.f,
                                         _invertNormalMapY ? -1.f : 1.f, "");
          }
        }

        if (_refractionTexture
            && StandardMaterial::RefractionTextureEnabled()) {
          float depth = 1.f;
          if (!_refractionTexture->isCube) {
            _uniformBuffer->updateMatrix(
              "refractionMatrix",
              *_refractionTexture->getReflectionTextureMatrix());
            auto refractionTextureTmp
              = std::static_pointer_cast<RefractionTexture>(_refractionTexture);
            if (refractionTextureTmp) {
              depth = refractionTextureTmp->depth;
            }
          }
          _uniformBuffer->updateFloat4(
            "vRefractionInfos", _refractionTexture->level, indexOfRefraction,
            depth, invertRefractionY ? -1.f : 1.f, "");
        }
      }

      // Point size
      if (pointsCloud()) {
        _uniformBuffer->updateFloat("pointSize", pointSize);
      }

      if (defines["SPECULARTERM"]) {
        _uniformBuffer->updateColor4("vSpecularColor", specularColor,
                                     specularPower, "");
      }
      _uniformBuffer->updateColor3("vEmissiveColor", emissiveColor, "");
      // Diffuse
      _uniformBuffer->updateColor4("vDiffuseColor", diffuseColor,
                                   alpha() * mesh->visibility(), "");
    }

    // Textures
    if (scene->texturesEnabled()) {
      if (_diffuseTexture && StandardMaterial::DiffuseTextureEnabled()) {
        effect->setTexture("diffuseSampler", _diffuseTexture);
      }

      if (_ambientTexture && StandardMaterial::AmbientTextureEnabled()) {
        effect->setTexture("ambientSampler", _ambientTexture);
      }

      if (_opacityTexture && StandardMaterial::OpacityTextureEnabled()) {
        effect->setTexture("opacitySampler", _opacityTexture);
      }

      if (_reflectionTexture && StandardMaterial::ReflectionTextureEnabled()) {
        if (_reflectionTexture->isCube) {
          effect->setTexture("reflectionCubeSampler", _reflectionTexture);
        }
        else {
          effect->setTexture("reflection2DSampler", _reflectionTexture);
        }
      }

      if (_emissiveTexture && StandardMaterial::EmissiveTextureEnabled()) {
        effect->setTexture("emissiveSampler", _emissiveTexture);
      }

      if (_lightmapTexture && StandardMaterial::LightmapTextureEnabled()) {
        effect->setTexture("lightmapSampler", _lightmapTexture);
      }

      if (_specularTexture && StandardMaterial::SpecularTextureEnabled()) {
        effect->setTexture("specularSampler", _specularTexture);
      }

      if (_bumpTexture && scene->getEngine()->getCaps().standardDerivatives
          && StandardMaterial::BumpTextureEnabled()) {
        effect->setTexture("bumpSampler", _bumpTexture);
      }

      if (_refractionTexture && StandardMaterial::RefractionTextureEnabled()) {
        if (_refractionTexture->isCube) {
          effect->setTexture("refractionCubeSampler", _refractionTexture);
        }
        else {
          effect->setTexture("refraction2DSampler", _refractionTexture);
        }
      }
    }

    // Clip plane
    MaterialHelper::BindClipPlane(effect, scene);

    // Colors
    scene->ambientColor.multiplyToRef(ambientColor, _globalAmbientColor);

    MaterialHelper::BindEyePosition(effect, scene);
    effect->setColor3("vAmbientColor", _globalAmbientColor);
  }

  if (mustRebind || !isFrozen()) {
    // Lights
    if (scene->lightsEnabled() && !_disableLighting) {
      MaterialHelper::BindLights(scene, mesh, effect, defines,
                                 _maxSimultaneousLights);
    }

    // View
    if ((scene->fogEnabled() && mesh->applyFog()
         && (scene->fogMode() != Scene::FOGMODE_NONE))
        || _reflectionTexture || _refractionTexture) {
      bindView(effect.get());
    }

    // Fog
    MaterialHelper::BindFogParameters(scene, mesh, effect);

    // Morph targets
    if (defines.intDef["NUM_MORPH_INFLUENCERS"]) {
      MaterialHelper::BindMorphTargetParameters(mesh, effect);
    }

    // Log. depth
    MaterialHelper::BindLogDepth(defines, effect, scene);

    // Image processing
    if (_imageProcessingConfiguration
        && !_imageProcessingConfiguration->applyByPostProcess()) {
      _imageProcessingConfiguration->bind(_activeEffect.get());
    }
  }

  _uniformBuffer->update();
  _afterBind(mesh, _activeEffect);
}

std::vector<IAnimatablePtr> StandardMaterial::getAnimatables()
{
  std::vector<IAnimatablePtr> results;

  if (_diffuseTexture && _diffuseTexture->animations.size() > 0) {
    results.emplace_back(_diffuseTexture);
  }

  if (_ambientTexture && _ambientTexture->animations.size() > 0) {
    results.emplace_back(_ambientTexture);
  }

  if (_opacityTexture && _opacityTexture->animations.size() > 0) {
    results.emplace_back(_opacityTexture);
  }

  if (_reflectionTexture && _reflectionTexture->animations.size() > 0) {
    results.emplace_back(_reflectionTexture);
  }

  if (_emissiveTexture && _emissiveTexture->animations.size() > 0) {
    results.emplace_back(_emissiveTexture);
  }

  if (_specularTexture && _specularTexture->animations.size() > 0) {
    results.emplace_back(_specularTexture);
  }

  if (_bumpTexture && _bumpTexture->animations.size() > 0) {
    results.emplace_back(_bumpTexture);
  }

  if (_lightmapTexture && _lightmapTexture->animations.size() > 0) {
    results.emplace_back(_lightmapTexture);
  }

  if (_refractionTexture && _refractionTexture->animations.size() > 0) {
    results.emplace_back(_refractionTexture);
  }

  return results;
}

std::vector<BaseTexturePtr> StandardMaterial::getActiveTextures() const
{
  auto activeTextures = Material::getActiveTextures();

  if (_diffuseTexture) {
    activeTextures.emplace_back(_diffuseTexture);
  }

  if (_ambientTexture) {
    activeTextures.emplace_back(_ambientTexture);
  }

  if (_opacityTexture) {
    activeTextures.emplace_back(_opacityTexture);
  }

  if (_reflectionTexture) {
    activeTextures.emplace_back(_reflectionTexture);
  }

  if (_emissiveTexture) {
    activeTextures.emplace_back(_emissiveTexture);
  }

  if (_specularTexture) {
    activeTextures.emplace_back(_specularTexture);
  }

  if (_bumpTexture) {
    activeTextures.emplace_back(_bumpTexture);
  }

  if (_lightmapTexture) {
    activeTextures.emplace_back(_lightmapTexture);
  }

  if (_refractionTexture) {
    activeTextures.emplace_back(_refractionTexture);
  }

  return activeTextures;
}

bool StandardMaterial::hasTexture(const BaseTexturePtr& texture) const
{
  if (PushMaterial::hasTexture(texture)) {
    return true;
  }

  if (_diffuseTexture == texture) {
    return true;
  }

  if (_ambientTexture == texture) {
    return true;
  }

  if (_opacityTexture == texture) {
    return true;
  }

  if (_reflectionTexture == texture) {
    return true;
  }

  if (_emissiveTexture == texture) {
    return true;
  }

  if (_specularTexture == texture) {
    return true;
  }

  if (_bumpTexture == texture) {
    return true;
  }

  if (_lightmapTexture == texture) {
    return true;
  }

  if (_refractionTexture == texture) {
    return true;
  }

  return false;
}

void StandardMaterial::dispose(bool forceDisposeEffect,
                               bool forceDisposeTextures)
{
  if (forceDisposeTextures) {
    if (_diffuseTexture) {
      _diffuseTexture->dispose();
    }

    if (_ambientTexture) {
      _ambientTexture->dispose();
    }

    if (_opacityTexture) {
      _opacityTexture->dispose();
    }

    if (_reflectionTexture) {
      _reflectionTexture->dispose();
    }

    if (_emissiveTexture) {
      _emissiveTexture->dispose();
    }

    if (_specularTexture) {
      _specularTexture->dispose();
    }

    if (_bumpTexture) {
      _bumpTexture->dispose();
    }

    if (_lightmapTexture) {
      _lightmapTexture->dispose();
    }

    if (_refractionTexture) {
      _refractionTexture->dispose();
    }
  }

  if (_imageProcessingConfiguration && _imageProcessingObserver) {
    _imageProcessingConfiguration->onUpdateParameters.remove(
      _imageProcessingObserver);
  }

  Material::dispose(forceDisposeEffect, forceDisposeTextures);
}

MaterialPtr StandardMaterial::clone(const std::string& _name,
                                    bool /*cloneChildren*/) const
{
  auto standardMaterial  = StandardMaterial::New(*this);
  standardMaterial->name = _name;
  standardMaterial->id   = _name;
  return standardMaterial;
}

json StandardMaterial::serialize() const
{
  return nullptr;
}

BaseTexturePtr& StandardMaterial::get_diffuseTexture()
{
  return _diffuseTexture;
}

void StandardMaterial::set_diffuseTexture(const BaseTexturePtr& value)
{
  if (_diffuseTexture == value) {
    return;
  }

  _diffuseTexture = value;
  _markAllSubMeshesAsTexturesAndMiscDirty();
}

BaseTexturePtr& StandardMaterial::get_ambientTexture()
{
  return _ambientTexture;
}

void StandardMaterial::set_ambientTexture(const BaseTexturePtr& value)
{
  if (_ambientTexture == value) {
    return;
  }

  _ambientTexture = value;
  _markAllSubMeshesAsTexturesDirty();
}

BaseTexturePtr& StandardMaterial::get_opacityTexture()
{
  return _opacityTexture;
}

void StandardMaterial::set_opacityTexture(const BaseTexturePtr& value)
{
  if (_opacityTexture == value) {
    return;
  }

  _opacityTexture = value;
  _markAllSubMeshesAsTexturesAndMiscDirty();
}

BaseTexturePtr& StandardMaterial::get_reflectionTexture()
{
  return _reflectionTexture;
}

void StandardMaterial::set_reflectionTexture(const BaseTexturePtr& value)
{
  if (_reflectionTexture == value) {
    return;
  }

  _reflectionTexture = value;
  _markAllSubMeshesAsTexturesDirty();
}

BaseTexturePtr& StandardMaterial::get_emissiveTexture()
{
  return _emissiveTexture;
}

void StandardMaterial::set_emissiveTexture(const BaseTexturePtr& value)
{
  if (_emissiveTexture == value) {
    return;
  }

  _emissiveTexture = value;
  _markAllSubMeshesAsTexturesDirty();
}

BaseTexturePtr& StandardMaterial::get_specularTexture()
{
  return _specularTexture;
}

void StandardMaterial::set_specularTexture(const BaseTexturePtr& value)
{
  if (_specularTexture == value) {
    return;
  }

  _specularTexture = value;
  _markAllSubMeshesAsTexturesDirty();
}

BaseTexturePtr& StandardMaterial::get_bumpTexture()
{
  return _bumpTexture;
}

void StandardMaterial::set_bumpTexture(const BaseTexturePtr& value)
{
  if (_bumpTexture == value) {
    return;
  }

  _bumpTexture = value;
  _markAllSubMeshesAsTexturesDirty();
}

BaseTexturePtr& StandardMaterial::get_lightmapTexture()
{
  return _lightmapTexture;
}

void StandardMaterial::set_lightmapTexture(const BaseTexturePtr& value)
{
  if (_lightmapTexture == value) {
    return;
  }

  _lightmapTexture = value;
  _markAllSubMeshesAsTexturesDirty();
}

BaseTexturePtr& StandardMaterial::get_refractionTexture()
{
  return _refractionTexture;
}

void StandardMaterial::set_refractionTexture(const BaseTexturePtr& value)
{
  if (_refractionTexture == value) {
    return;
  }

  _refractionTexture = value;
  _markAllSubMeshesAsTexturesDirty();
}

bool StandardMaterial::get_useAlphaFromDiffuseTexture() const
{
  return _useAlphaFromDiffuseTexture;
}

void StandardMaterial::set_useAlphaFromDiffuseTexture(bool value)
{
  if (_useAlphaFromDiffuseTexture == value) {
    return;
  }

  _useAlphaFromDiffuseTexture = value;
  _markAllSubMeshesAsTexturesDirty();
}

bool StandardMaterial::get_useEmissiveAsIllumination() const
{
  return _useEmissiveAsIllumination;
}

void StandardMaterial::set_useEmissiveAsIllumination(bool value)
{
  if (_useEmissiveAsIllumination == value) {
    return;
  }

  _useEmissiveAsIllumination = value;
  _markAllSubMeshesAsTexturesDirty();
}

bool StandardMaterial::get_linkEmissiveWithDiffuse() const
{
  return _linkEmissiveWithDiffuse;
}

void StandardMaterial::set_linkEmissiveWithDiffuse(bool value)
{
  if (_linkEmissiveWithDiffuse == value) {
    return;
  }

  _linkEmissiveWithDiffuse = value;
  _markAllSubMeshesAsTexturesDirty();
}

bool StandardMaterial::get_useSpecularOverAlpha() const
{
  return _useSpecularOverAlpha;
}

void StandardMaterial::set_useSpecularOverAlpha(bool value)
{
  if (_useSpecularOverAlpha == value) {
    return;
  }

  _useSpecularOverAlpha = value;
  _markAllSubMeshesAsTexturesDirty();
}

bool StandardMaterial::get_useReflectionOverAlpha() const
{
  return _useReflectionOverAlpha;
}

void StandardMaterial::set_useReflectionOverAlpha(bool value)
{
  if (_useReflectionOverAlpha == value) {
    return;
  }

  _useReflectionOverAlpha = value;
  _markAllSubMeshesAsTexturesDirty();
}

bool StandardMaterial::get_disableLighting() const
{
  return _disableLighting;
}

void StandardMaterial::set_disableLighting(bool value)
{
  if (_disableLighting == value) {
    return;
  }

  _disableLighting = value;
  _markAllSubMeshesAsLightsDirty();
}

bool StandardMaterial::get_useObjectSpaceNormalMap() const
{
  return _useObjectSpaceNormalMap;
}

void StandardMaterial::set_useObjectSpaceNormalMap(bool value)
{
  if (_useObjectSpaceNormalMap == value) {
    return;
  }

  _useObjectSpaceNormalMap = value;
  _markAllSubMeshesAsTexturesDirty();
}

bool StandardMaterial::get_useParallax() const
{
  return _useParallax;
}

void StandardMaterial::set_useParallax(bool value)
{
  if (_useParallax == value) {
    return;
  }

  _useParallax = value;
  _markAllSubMeshesAsTexturesDirty();
}

bool StandardMaterial::get_useParallaxOcclusion() const
{
  return _useParallaxOcclusion;
}

void StandardMaterial::set_useParallaxOcclusion(bool value)
{
  if (_useParallaxOcclusion == value) {
    return;
  }

  _useParallaxOcclusion = value;
  _markAllSubMeshesAsTexturesDirty();
}

float StandardMaterial::get_roughness() const
{
  return _roughness;
}

void StandardMaterial::set_roughness(float value)
{
  if (stl_util::almost_equal(_roughness, value)) {
    return;
  }

  _roughness = value;
  _markAllSubMeshesAsTexturesDirty();
}

bool StandardMaterial::get_useLightmapAsShadowmap() const
{
  return _useLightmapAsShadowmap;
}

void StandardMaterial::set_useLightmapAsShadowmap(bool value)
{
  if (_useLightmapAsShadowmap == value) {
    return;
  }

  _useLightmapAsShadowmap = value;
  _markAllSubMeshesAsTexturesDirty();
}

FresnelParametersPtr& StandardMaterial::get_diffuseFresnelParameters()
{
  if (!_diffuseFresnelParameters) {
    _diffuseFresnelParameters = std::make_shared<FresnelParameters>();
  }

  return _diffuseFresnelParameters;
}

void StandardMaterial::set_diffuseFresnelParameters(
  const FresnelParametersPtr& value)
{
  if (_diffuseFresnelParameters == value) {
    return;
  }

  _diffuseFresnelParameters = value;
  _markAllSubMeshesAsFresnelDirty();
}

FresnelParametersPtr& StandardMaterial::get_opacityFresnelParameters()
{
  if (!_opacityFresnelParameters) {
    _opacityFresnelParameters = std::make_shared<FresnelParameters>();
  }

  return _opacityFresnelParameters;
}

void StandardMaterial::set_opacityFresnelParameters(
  const FresnelParametersPtr& value)
{
  if (_opacityFresnelParameters == value) {
    return;
  }

  _opacityFresnelParameters = value;
  _markAllSubMeshesAsFresnelAndMiscDirty();
}

FresnelParametersPtr& StandardMaterial::get_reflectionFresnelParameters()
{
  if (!_reflectionFresnelParameters) {
    _reflectionFresnelParameters = std::make_shared<FresnelParameters>();
  }

  return _reflectionFresnelParameters;
}

void StandardMaterial::set_reflectionFresnelParameters(
  const FresnelParametersPtr& value)
{
  if (_reflectionFresnelParameters == value) {
    return;
  }

  _reflectionFresnelParameters = value;
  _markAllSubMeshesAsFresnelDirty();
}

FresnelParametersPtr& StandardMaterial::get_refractionFresnelParameters()
{
  if (!_refractionFresnelParameters) {
    _refractionFresnelParameters = std::make_shared<FresnelParameters>();
  }

  return _refractionFresnelParameters;
}

void StandardMaterial::set_refractionFresnelParameters(
  const FresnelParametersPtr& value)
{
  if (_refractionFresnelParameters == value) {
    return;
  }

  _refractionFresnelParameters = value;
  _markAllSubMeshesAsFresnelDirty();
}

FresnelParametersPtr& StandardMaterial::get_emissiveFresnelParameters()
{
  if (!_emissiveFresnelParameters) {
    _emissiveFresnelParameters = std::make_shared<FresnelParameters>();
  }

  return _emissiveFresnelParameters;
}

void StandardMaterial::set_emissiveFresnelParameters(
  const FresnelParametersPtr& value)
{
  if (_emissiveFresnelParameters == value) {
    return;
  }

  _emissiveFresnelParameters = value;
  _markAllSubMeshesAsFresnelDirty();
}

bool StandardMaterial::get_useReflectionFresnelFromSpecular() const
{
  return _useReflectionFresnelFromSpecular;
}

void StandardMaterial::set_useReflectionFresnelFromSpecular(bool value)
{
  if (_useReflectionFresnelFromSpecular == value) {
    return;
  }

  _useReflectionFresnelFromSpecular = value;
  _markAllSubMeshesAsFresnelDirty();
}

bool StandardMaterial::get_useGlossinessFromSpecularMapAlpha() const
{
  return _useGlossinessFromSpecularMapAlpha;
}

void StandardMaterial::set_useGlossinessFromSpecularMapAlpha(bool value)
{
  if (_useGlossinessFromSpecularMapAlpha == value) {
    return;
  }

  _useGlossinessFromSpecularMapAlpha = value;
  _markAllSubMeshesAsTexturesDirty();
}

unsigned int StandardMaterial::get_maxSimultaneousLights() const
{
  return _maxSimultaneousLights;
}

void StandardMaterial::set_maxSimultaneousLights(unsigned int value)
{
  if (_maxSimultaneousLights == value) {
    return;
  }

  _maxSimultaneousLights = value;
  _markAllSubMeshesAsLightsDirty();
}

bool StandardMaterial::get_invertNormalMapX() const
{
  return _invertNormalMapX;
}

void StandardMaterial::set_invertNormalMapX(bool value)
{
  if (_invertNormalMapX == value) {
    return;
  }

  _invertNormalMapX = value;
  _markAllSubMeshesAsTexturesDirty();
}

bool StandardMaterial::get_invertNormalMapY() const
{
  return _invertNormalMapY;
}

void StandardMaterial::set_invertNormalMapY(bool value)
{
  if (_invertNormalMapY == value) {
    return;
  }

  _invertNormalMapY = value;
  _markAllSubMeshesAsTexturesDirty();
}

bool StandardMaterial::get_twoSidedLighting() const
{
  return _twoSidedLighting;
}

void StandardMaterial::set_twoSidedLighting(bool value)
{
  if (_twoSidedLighting == value) {
    return;
  }

  _twoSidedLighting = value;
  _markAllSubMeshesAsTexturesDirty();
}

ImageProcessingConfiguration*&
StandardMaterial::get_imageProcessingConfiguration()
{
  return _imageProcessingConfiguration;
}

void StandardMaterial::set_imageProcessingConfiguration(
  ImageProcessingConfiguration* const& value)
{
  _attachImageProcessingConfiguration(value);

  // Ensure the effect will be rebuilt.
  _markAllSubMeshesAsTexturesDirty();
}

void StandardMaterial::_attachImageProcessingConfiguration(
  ImageProcessingConfiguration* configuration)
{
  if (configuration && (configuration == _imageProcessingConfiguration)) {
    return;
  }

  // Detaches observer.
  if (_imageProcessingConfiguration && _imageProcessingObserver) {
    _imageProcessingConfiguration->onUpdateParameters.remove(
      _imageProcessingObserver);
  }

  // Pick the scene configuration if needed.
  if (!configuration) {
    _imageProcessingConfiguration
      = getScene()->imageProcessingConfiguration().get();
  }
  else {
    _imageProcessingConfiguration = configuration;
  }

  // Attaches observer.
  if (_imageProcessingConfiguration) {
    _imageProcessingObserver
      = _imageProcessingConfiguration->onUpdateParameters.add(
        [this](ImageProcessingConfiguration*, EventState&) {
          _markAllSubMeshesAsImageProcessingDirty();
        });
  }
}

bool StandardMaterial::get_cameraColorCurvesEnabled() const
{
  return imageProcessingConfiguration()->colorCurvesEnabled();
}

void StandardMaterial::set_cameraColorCurvesEnabled(bool value)
{
  imageProcessingConfiguration()->colorCurvesEnabled = value;
}

bool StandardMaterial::get_cameraColorGradingEnabled() const
{
  return imageProcessingConfiguration()->colorGradingEnabled();
}

void StandardMaterial::set_cameraColorGradingEnabled(bool value)
{
  imageProcessingConfiguration()->colorGradingEnabled = value;
}

bool StandardMaterial::get_cameraToneMappingEnabled() const
{
  return _imageProcessingConfiguration->toneMappingEnabled();
}

void StandardMaterial::set_cameraToneMappingEnabled(bool value)
{
  _imageProcessingConfiguration->toneMappingEnabled = value;
}

float StandardMaterial::get_cameraExposure() const
{
  return _imageProcessingConfiguration->exposure();
}

void StandardMaterial::set_cameraExposure(float value)
{
  _imageProcessingConfiguration->exposure = value;
}

float StandardMaterial::get_cameraContrast() const
{
  return _imageProcessingConfiguration->contrast();
}

void StandardMaterial::set_cameraContrast(float value)
{
  _imageProcessingConfiguration->contrast = value;
}

BaseTexturePtr& StandardMaterial::get_cameraColorGradingTexture()
{
  return _imageProcessingConfiguration->colorGradingTexture;
}

void StandardMaterial::set_cameraColorGradingTexture(
  const BaseTexturePtr& value)
{
  _imageProcessingConfiguration->colorGradingTexture = value;
}

std::shared_ptr<ColorCurves>& StandardMaterial::get_cameraColorCurves()
{
  return _imageProcessingConfiguration->colorCurves;
}

void StandardMaterial::set_cameraColorCurves(
  const std::shared_ptr<ColorCurves>& value)
{
  _imageProcessingConfiguration->colorCurves = value;
}

StandardMaterialPtr StandardMaterial::Parse(const json& source, Scene* scene,
                                            const std::string& rootUrl)
{
  return SerializationHelper::Parse(
    [source, scene, rootUrl]() {
      return StandardMaterial::New(json_util::get_string(source, "name"),
                                   scene);
    },
    source, scene, rootUrl);
}

bool StandardMaterial::DiffuseTextureEnabled()
{
  return StandardMaterial::_DiffuseTextureEnabled;
}

void StandardMaterial::SetDiffuseTextureEnabled(bool value)
{
  if (StandardMaterial::_DiffuseTextureEnabled == value) {
    return;
  }

  StandardMaterial::_DiffuseTextureEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::TextureDirtyFlag);
}

bool StandardMaterial::AmbientTextureEnabled()
{
  return StandardMaterial::_AmbientTextureEnabled;
}

void StandardMaterial::SetAmbientTextureEnabled(bool value)
{
  if (StandardMaterial::_AmbientTextureEnabled == value) {
    return;
  }

  StandardMaterial::_AmbientTextureEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::TextureDirtyFlag);
}

bool StandardMaterial::OpacityTextureEnabled()
{
  return StandardMaterial::_OpacityTextureEnabled;
}

void StandardMaterial::SetOpacityTextureEnabled(bool value)
{
  if (StandardMaterial::_OpacityTextureEnabled == value) {
    return;
  }

  StandardMaterial::_OpacityTextureEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::TextureDirtyFlag);
}

bool StandardMaterial::ReflectionTextureEnabled()
{
  return StandardMaterial::_ReflectionTextureEnabled;
}

void StandardMaterial::SetReflectionTextureEnabled(bool value)
{
  if (StandardMaterial::_ReflectionTextureEnabled == value) {
    return;
  }

  StandardMaterial::_ReflectionTextureEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::TextureDirtyFlag);
}

bool StandardMaterial::EmissiveTextureEnabled()
{
  return StandardMaterial::_EmissiveTextureEnabled;
}

void StandardMaterial::SetEmissiveTextureEnabled(bool value)
{
  if (StandardMaterial::_EmissiveTextureEnabled == value) {
    return;
  }

  StandardMaterial::_EmissiveTextureEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::TextureDirtyFlag);
}

bool StandardMaterial::SpecularTextureEnabled()
{
  return StandardMaterial::_SpecularTextureEnabled;
}

void StandardMaterial::SetSpecularTextureEnabled(bool value)
{
  if (StandardMaterial::_SpecularTextureEnabled == value) {
    return;
  }

  StandardMaterial::_SpecularTextureEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::TextureDirtyFlag);
}

bool StandardMaterial::BumpTextureEnabled()
{
  return StandardMaterial::_BumpTextureEnabled;
}

void StandardMaterial::SetBumpTextureEnabled(bool value)
{
  if (StandardMaterial::_BumpTextureEnabled == value) {
    return;
  }

  StandardMaterial::_BumpTextureEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::TextureDirtyFlag);
}

bool StandardMaterial::LightmapTextureEnabled()
{
  return StandardMaterial::_LightmapTextureEnabled;
}

void StandardMaterial::SetLightmapTextureEnabled(bool value)
{
  if (StandardMaterial::_LightmapTextureEnabled == value) {
    return;
  }

  StandardMaterial::_LightmapTextureEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::TextureDirtyFlag);
}

bool StandardMaterial::RefractionTextureEnabled()
{
  return StandardMaterial::_RefractionTextureEnabled;
}

void StandardMaterial::SetRefractionTextureEnabled(bool value)
{
  if (StandardMaterial::_RefractionTextureEnabled == value) {
    return;
  }

  StandardMaterial::_RefractionTextureEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::TextureDirtyFlag);
}

bool StandardMaterial::ColorGradingTextureEnabled()
{
  return StandardMaterial::_ColorGradingTextureEnabled;
}

void StandardMaterial::SetColorGradingTextureEnabled(bool value)
{
  if (StandardMaterial::_ColorGradingTextureEnabled == value) {
    return;
  }

  StandardMaterial::_ColorGradingTextureEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::TextureDirtyFlag);
}

bool StandardMaterial::FresnelEnabled()
{
  return StandardMaterial::_FresnelEnabled;
}

void StandardMaterial::SetFresnelEnabled(bool value)
{
  if (StandardMaterial::_FresnelEnabled == value) {
    return;
  }

  StandardMaterial::_FresnelEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::FresnelDirtyFlag);
}

} // end of namespace BABYLON
