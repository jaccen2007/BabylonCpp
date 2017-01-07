﻿#ifndef BABYLON_MATERIALS_LIBRARY_FIRE_FIRE_VERTEX_FX_H
#define BABYLON_MATERIALS_LIBRARY_FIRE_FIRE_VERTEX_FX_H

namespace BABYLON {

extern const char* fireVertexShader;

const char* fireVertexShader
  = "#ifdef GL_ES\n"
    "precision highp float;\n"
    "#endif\n"
    "\n"
    "// Attributes\n"
    "attribute vec3 position;\n"
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
    "#ifdef DIFFUSE\n"
    "varying vec2 vDiffuseUV;\n"
    "#endif\n"
    "\n"
    "#ifdef POINTSIZE\n"
    "uniform float pointSize;\n"
    "#endif\n"
    "\n"
    "// Output\n"
    "varying vec3 vPositionW;\n"
    "\n"
    "#ifdef VERTEXCOLOR\n"
    "varying vec4 vColor;\n"
    "#endif\n"
    "\n"
    "#include<clipPlaneVertexDeclaration>\n"
    "\n"
    "#include<fogVertexDeclaration>\n"
    "\n"
    "// Fire\n"
    "uniform float time;\n"
    "uniform float speed;\n"
    "\n"
    "#ifdef DIFFUSE\n"
    "varying vec2 vDistortionCoords1;\n"
    "varying vec2 vDistortionCoords2;\n"
    "varying vec2 vDistortionCoords3;\n"
    "#endif\n"
    "\n"
    "void main(void) {\n"
    "\n"
    "#include<instancesVertex>\n"
    "#include<bonesVertex>\n"
    "\n"
    "  gl_Position = viewProjection * finalWorld * vec4(position, 1.0);\n"
    "\n"
    "  vec4 worldPos = finalWorld * vec4(position, 1.0);\n"
    "  vPositionW = vec3(worldPos);\n"
    "\n"
    "  // Texture coordinates\n"
    "#ifdef DIFFUSE\n"
    "  vDiffuseUV = uv;\n"
    "  vDiffuseUV.y -= 0.2;\n"
    "#endif\n"
    "\n"
    "  // Clip plane\n"
    "#include<clipPlaneVertex>\n"
    "\n"
    "  // Fog\n"
    "#include<fogVertex>\n"
    "\n"
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
    "#ifdef DIFFUSE\n"
    "  // Fire\n"
    "  vec3 layerSpeed = vec3(-0.2, -0.52, -0.1) * speed;\n"
    "  \n"
    "  vDistortionCoords1.x = uv.x;\n"
    "  vDistortionCoords1.y = uv.y + layerSpeed.x * time / 1000.0;\n"
    "  \n"
    "  vDistortionCoords2.x = uv.x;\n"
    "  vDistortionCoords2.y = uv.y + layerSpeed.y * time / 1000.0;\n"
    "  \n"
    "  vDistortionCoords3.x = uv.x;\n"
    "  vDistortionCoords3.y = uv.y + layerSpeed.z * time / 1000.0;\n"
    "#endif\n"
    "}\n";

} // end of namespace BABYLON

#endif // end of BABYLON_MATERIALS_LIBRARY_FIRE_FIRE_VERTEX_FX_H
