#include <babylon/materials/pbr/pbr_sheen_configuration.h>

#include <babylon/babylon_stl_util.h>
#include <babylon/core/json_util.h>
#include <babylon/engines/scene.h>
#include <babylon/materials/effect_fallbacks.h>
#include <babylon/materials/material_flags.h>
#include <babylon/materials/material_helper.h>
#include <babylon/materials/pbr/imaterial_sheen_defines.h>
#include <babylon/materials/textures/base_texture.h>
#include <babylon/materials/uniform_buffer.h>

namespace BABYLON {

PBRSheenConfiguration::PBRSheenConfiguration(
  const std::function<void()>& markAllSubMeshesAsTexturesDirty)
    : isEnabled{this, &PBRSheenConfiguration::get_isEnabled, &PBRSheenConfiguration::set_isEnabled}
    , linkSheenWithAlbedo{this, &PBRSheenConfiguration::get_linkSheenWithAlbedo,
                          &PBRSheenConfiguration::set_linkSheenWithAlbedo}
    , intensity{1.f}
    , color{Color3::White()}
    , texture{this, &PBRSheenConfiguration::get_texture, &PBRSheenConfiguration::set_texture}
    , roughness{this, &PBRSheenConfiguration::get_roughness, &PBRSheenConfiguration::set_roughness}
    , albedoScaling{this, &PBRSheenConfiguration::get_albedoScaling,
                    &PBRSheenConfiguration::set_albedoScaling}
    , _isEnabled{false}
    , _linkSheenWithAlbedo{false}
    , _texture{nullptr}
    , _roughness{std::nullopt}
    , _albedoScaling{false}
    , _internalMarkAllSubMeshesAsTexturesDirty{markAllSubMeshesAsTexturesDirty}
{
  isEnabled = false;
}

PBRSheenConfiguration::~PBRSheenConfiguration() = default;

bool PBRSheenConfiguration::get_isEnabled() const
{
  return _isEnabled;
}

void PBRSheenConfiguration::set_isEnabled(bool value)
{
  if (_isEnabled == value) {
    return;
  }

  _isEnabled = value;
  _markAllSubMeshesAsTexturesDirty();
}

bool PBRSheenConfiguration::get_linkSheenWithAlbedo() const
{
  return _linkSheenWithAlbedo;
}

void PBRSheenConfiguration::set_linkSheenWithAlbedo(bool value)
{
  if (_linkSheenWithAlbedo == value) {
    return;
  }

  _linkSheenWithAlbedo = value;
  _markAllSubMeshesAsTexturesDirty();
}

BaseTexturePtr& PBRSheenConfiguration::get_texture()
{
  return _texture;
}

void PBRSheenConfiguration::set_texture(const BaseTexturePtr& value)
{
  if (_texture == value) {
    return;
  }

  _texture = value;
  _markAllSubMeshesAsTexturesDirty();
}

std::optional<float>& PBRSheenConfiguration::get_roughness()
{
  return _roughness;
}

void PBRSheenConfiguration::set_roughness(const std::optional<float>& value)
{
  if (_roughness == value) {
    return;
  }

  _roughness = value;
  _markAllSubMeshesAsTexturesDirty();
}

bool PBRSheenConfiguration::get_albedoScaling() const
{
  return _albedoScaling;
}

void PBRSheenConfiguration::set_albedoScaling(bool value)
{
  if (_albedoScaling == value) {
    return;
  }

  _albedoScaling = value;
  _markAllSubMeshesAsTexturesDirty();
}

void PBRSheenConfiguration::_markAllSubMeshesAsTexturesDirty()
{
  _internalMarkAllSubMeshesAsTexturesDirty();
}

bool PBRSheenConfiguration::isReadyForSubMesh(const MaterialDefines& defines, Scene* scene) const
{
  if (defines._areTexturesDirty) {
    if (scene->texturesEnabled()) {
      if (_texture && MaterialFlags::AnisotropicTextureEnabled()) {
        if (!_texture->isReadyOrNotBlocking()) {
          return false;
        }
      }
    }
  }

  return true;
}

void PBRSheenConfiguration::prepareDefines(MaterialDefines& defines, Scene* scene)
{
  if (_isEnabled) {
    defines.boolDef["SHEEN"]                = _isEnabled;
    defines.boolDef["SHEEN_LINKWITHALBEDO"] = _linkSheenWithAlbedo;
    defines.boolDef["SHEEN_ROUGHNESS"]      = _roughness.has_value();
    defines.boolDef["SHEEN_ALBEDOSCALING"]  = _albedoScaling;

    if (defines._areTexturesDirty) {
      if (scene->texturesEnabled()) {
        if (_texture && MaterialFlags::SheenTextureEnabled()) {
          MaterialHelper::PrepareDefinesForMergedUV(_texture, defines, "SHEEN_TEXTURE");
        }
        else {
          defines.boolDef["SHEEN_TEXTURE"] = false;
        }
      }
    }
  }
  else {
    defines.boolDef["SHEEN"]                = false;
    defines.boolDef["SHEEN_TEXTURE"]        = false;
    defines.boolDef["SHEEN_LINKWITHALBEDO"] = false;
    defines.boolDef["SHEEN_ROUGHNESS"]      = false;
    defines.boolDef["SHEEN_ALBEDOSCALING"]  = false;
  }
}

void PBRSheenConfiguration::bindForSubMesh(UniformBuffer& uniformBuffer, Scene* scene,
                                           bool isFrozen)
{
  if (!uniformBuffer.useUbo() || !isFrozen || !uniformBuffer.isSync()) {
    if (_texture && MaterialFlags::SheenTextureEnabled()) {
      uniformBuffer.updateFloat2("vSheenInfos", static_cast<float>(_texture->coordinatesIndex),
                                 _texture->level, "");
      MaterialHelper::BindTextureMatrix(*_texture, uniformBuffer, "sheen");
    }

    // Sheen
    uniformBuffer.updateFloat4("vSheenColor", color.r, color.g, color.b, intensity, "");

    if (_roughness.has_value()) {
      uniformBuffer.updateFloat("vSheenRoughness", *_roughness);
    }
  }

  // Textures
  if (scene->texturesEnabled()) {
    if (_texture && MaterialFlags::SheenTextureEnabled()) {
      uniformBuffer.setTexture("sheenSampler", _texture);
    }
  }
}

bool PBRSheenConfiguration::hasTexture(const BaseTexturePtr& iTexture) const
{
  return _texture == iTexture;
}

void PBRSheenConfiguration::getActiveTextures(std::vector<BaseTexturePtr>& activeTextures)
{
  if (_texture) {
    activeTextures.emplace_back(_texture);
  }
}

void PBRSheenConfiguration::getAnimatables(std::vector<IAnimatablePtr>& animatables)
{
  if (_texture && !_texture->animations.empty()) {
    animatables.emplace_back(_texture);
  }
}

void PBRSheenConfiguration::dispose(bool forceDisposeTextures)
{
  if (forceDisposeTextures) {
    if (_texture) {
      _texture->dispose();
    }
  }
}

std::string PBRSheenConfiguration::getClassName() const
{
  return "PBRSheenConfiguration";
}

unsigned int PBRSheenConfiguration::AddFallbacks(const MaterialDefines& defines,
                                                 EffectFallbacks& fallbacks,
                                                 unsigned int currentRank)
{
  if (defines["SHEEN"]) {
    fallbacks.addFallback(currentRank++, "SHEEN");
  }
  return currentRank;
}

void PBRSheenConfiguration::AddUniforms(std::vector<std::string>& uniforms)
{
  stl_util::concat(uniforms, {"vSheenColor", "vSheenRoughness", "vSheenInfos", "sheenMatrix"});
}

void PBRSheenConfiguration::PrepareUniformBuffer(UniformBuffer& uniformBuffer)
{
  uniformBuffer.addUniform("vSheenColor", 4);
  uniformBuffer.addUniform("vSheenRoughness", 1);
  uniformBuffer.addUniform("vSheenInfos", 2);
  uniformBuffer.addUniform("sheenMatrix", 16);
}

void PBRSheenConfiguration::AddSamplers(std::vector<std::string>& samplers)
{
  samplers.emplace_back("sheenSampler");
}

void PBRSheenConfiguration::copyTo(PBRSheenConfiguration& /*anisotropicConfiguration*/)
{
}

json PBRSheenConfiguration::serialize() const
{
  return nullptr;
}

void PBRSheenConfiguration::parse(const json& /*source*/, Scene* /*scene*/,
                                  const std::string& /*rootUrl*/)
{
}

} // end of namespace BABYLON
