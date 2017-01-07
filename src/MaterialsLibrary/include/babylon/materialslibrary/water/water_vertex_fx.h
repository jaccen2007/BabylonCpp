﻿#ifndef BABYLON_MATERIALS_LIBRARY_WATER_WATER_VERTEX_FX_H
#define BABYLON_MATERIALS_LIBRARY_WATER_WATER_VERTEX_FX_H

namespace BABYLON {

extern const char* waterVertexShader;

const char* waterVertexShader
  = "#ifdef GL_ES\n"
    "precision highp float;\n"
    "#endif\n"
    "\n"
    "// Attributes\n"
    "attribute vec3 position;\n"
    "#ifdef NORMAL\n"
    "attribute vec3 normal;\n"
    "#endif\n"
    "#ifdef UV1\n"
    "attribute vec2 uv;\n"
    "#endif\n"
    "#ifdef UV2\n"
    "attribute vec2 uv2;\n"
    "#endif\n"
    "#ifdef VERTEXCOLOR\n"
    "attribute vec4 color;\n"
    "#endif\n"
    "\n"
    "#include<bonesDeclaration>\n"
    "\n"
    "// Uniforms\n"
    "#include<instancesDeclaration>\n"
    "\n"
    "uniform mat4 view;\n"
    "uniform mat4 viewProjection;\n"
    "\n"
    "#ifdef BUMP\n"
    "varying vec2 vNormalUV;\n"
    "#ifdef BUMPSUPERIMPOSE\n"
    "  varying vec2 vNormalUV2;\n"
    "#endif\n"
    "uniform mat4 normalMatrix;\n"
    "uniform vec2 vNormalInfos;\n"
    "#endif\n"
    "\n"
    "#ifdef POINTSIZE\n"
    "uniform float pointSize;\n"
    "#endif\n"
    "\n"
    "// Output\n"
    "varying vec3 vPositionW;\n"
    "#ifdef NORMAL\n"
    "varying vec3 vNormalW;\n"
    "#endif\n"
    "\n"
    "#ifdef VERTEXCOLOR\n"
    "varying vec4 vColor;\n"
    "#endif\n"
    "\n"
    "#include<clipPlaneVertexDeclaration>\n"
    "\n"
    "#include<fogVertexDeclaration>\n"
    "#include<shadowsVertexDeclaration>[0..maxSimultaneousLights]\n"
    "\n"
    "#include<logDepthDeclaration>\n"
    "\n"
    "// Water uniforms\n"
    "uniform mat4 worldReflectionViewProjection;\n"
    "uniform vec2 windDirection;\n"
    "uniform float waveLength;\n"
    "uniform float time;\n"
    "uniform float windForce;\n"
    "uniform float waveHeight;\n"
    "uniform float waveSpeed;\n"
    "\n"
    "// Water varyings\n"
    "varying vec3 vPosition;\n"
    "varying vec3 vRefractionMapTexCoord;\n"
    "varying vec3 vReflectionMapTexCoord;\n"
    "\n"
    "\n"
    "\n"
    "void main(void) {\n"
    "\n"
    "  #include<instancesVertex>\n"
    "  #include<bonesVertex>\n"
    "\n"
    "  vec4 worldPos = finalWorld * vec4(position, 1.0);\n"
    "  vPositionW = vec3(worldPos);\n"
    "\n"
    "#ifdef NORMAL\n"
    "  vNormalW = normalize(vec3(finalWorld * vec4(normal, 0.0)));\n"
    "#endif\n"
    "\n"
    "  // Texture coordinates\n"
    "#ifndef UV1\n"
    "  vec2 uv = vec2(0., 0.);\n"
    "#endif\n"
    "#ifndef UV2\n"
    "  vec2 uv2 = vec2(0., 0.);\n"
    "#endif\n"
    "\n"
    "#ifdef BUMP\n"
    "  if (vNormalInfos.x == 0.)\n"
    "  {\n"
    "  vNormalUV = vec2(normalMatrix * vec4((uv * 1.0) / waveLength + time * windForce * windDirection, 1.0, 0.0));\n"
    "  #ifdef BUMPSUPERIMPOSE\n"
    "  vNormalUV2 = vec2(normalMatrix * vec4((uv * 0.721) / waveLength + time * 1.2 * windForce * windDirection, 1.0, 0.0));\n"
    "  #endif\n"
    "  }\n"
    "  else\n"
    "  {\n"
    "  vNormalUV = vec2(normalMatrix * vec4((uv2 * 1.0) / waveLength + time * windForce * windDirection , 1.0, 0.0));\n"
    "  #ifdef BUMPSUPERIMPOSE\n"
    "  vNormalUV2 = vec2(normalMatrix * vec4((uv2 * 0.721) / waveLength + time * 1.2 * windForce * windDirection , 1.0, 0.0));\n"
    "  #endif\n"
    "  }\n"
    "#endif\n"
    "\n"
    "  // Clip plane\n"
    "  #include<clipPlaneVertex>\n"
    "\n"
    "  // Fog\n"
    "  #include<fogVertex>\n"
    "  \n"
    "  // Shadows\n"
    "  #include<shadowsVertex>[0..maxSimultaneousLights]\n"
    "  \n"
    "  // Vertex color\n"
    "#ifdef VERTEXCOLOR\n"
    "  vColor = color;\n"
    "#endif\n"
    "\n"
    "  // Point size\n"
    "#ifdef POINTSIZE\n"
    "  gl_PointSize = pointSize;\n"
    "#endif\n"
    "\n"
    "  vec3 p = position;\n"
    "  float newY = (sin(((p.x / 0.05) + time * waveSpeed)) * waveHeight * windDirection.x * 5.0)\n"
    "   + (cos(((p.z / 0.05) +  time * waveSpeed)) * waveHeight * windDirection.y * 5.0);\n"
    "  p.y += abs(newY);\n"
    "  \n"
    "  gl_Position = viewProjection * finalWorld * vec4(p, 1.0);\n"
    "\n"
    "#ifdef REFLECTION\n"
    "  worldPos = viewProjection * finalWorld * vec4(p, 1.0);\n"
    "  \n"
    "  // Water\n"
    "  vPosition = position;\n"
    "  \n"
    "  vRefractionMapTexCoord.x = 0.5 * (worldPos.w + worldPos.x);\n"
    "  vRefractionMapTexCoord.y = 0.5 * (worldPos.w + worldPos.y);\n"
    "  vRefractionMapTexCoord.z = worldPos.w;\n"
    "  \n"
    "  worldPos = worldReflectionViewProjection * vec4(position, 1.0);\n"
    "  vReflectionMapTexCoord.x = 0.5 * (worldPos.w + worldPos.x);\n"
    "  vReflectionMapTexCoord.y = 0.5 * (worldPos.w + worldPos.y);\n"
    "  vReflectionMapTexCoord.z = worldPos.w;\n"
    "#endif\n"
    "\n"
    "#include<logDepthVertex>\n"
    "\n"
    "}\n";

} // end of namespace BABYLON

#endif // end of BABYLON_MATERIALS_LIBRARY_WATER_WATER_VERTEX_FX_H
