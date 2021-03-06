#include <babylon/materials/pbr/imaterial_sheen_defines.h>

#include <sstream>

namespace BABYLON {

IMaterialSheenDefines::IMaterialSheenDefines()
{
  boolDef = {
    {"SHEEN", false},                 //
    {"SHEEN_TEXTURE", false},         //
    {"SHEEN_TEXTUREDIRECTUV", false}, //
    {"SHEEN_LINKWITHALBEDO", false},  //
    {"SHEEN_ROUGHNESS", false},       //
    {"SHEEN_ALBEDOSCALING", false},   //
  };
}

IMaterialSheenDefines::~IMaterialSheenDefines() = default;

std::string IMaterialSheenDefines::toString() const
{
  std::ostringstream oss;
  oss << MaterialDefines::toString();

  return oss.str();
}

} // end of namespace BABYLON
