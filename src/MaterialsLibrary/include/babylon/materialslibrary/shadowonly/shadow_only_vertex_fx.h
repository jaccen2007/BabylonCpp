﻿#ifndef BABYLON_MATERIALS_LIBRARY_SHADOWONLY_SHADOW_ONLY_VERTEX_FX_H
#define BABYLON_MATERIALS_LIBRARY_SHADOWONLY_SHADOW_ONLY_VERTEX_FX_H

namespace BABYLON {

extern const char* shadowOnlyVertexShader;

const char* shadowOnlyVertexShader
  = R"ShaderCode(

#ifdef GL_ES
  precision highp float;
#endif

// Attributes
attribute vec3 position;
#ifdef NORMAL
attribute vec3 normal;
#endif

#include<bonesDeclaration>

// Uniforms
#include<instancesDeclaration>

uniform mat4 view;
uniform mat4 viewProjection;

#ifdef POINTSIZE
uniform float pointSize;
#endif

// Output
varying vec3 vPositionW;
#ifdef NORMAL
varying vec3 vNormalW;
#endif

#ifdef VERTEXCOLOR
varying vec4 vColor;
#endif


#include<clipPlaneVertexDeclaration>

#include<fogVertexDeclaration>
#include<__decl__lightFragment>[0..maxSimultaneousLights]

void main(void) {

#include<instancesVertex>
#include<bonesVertex>

    vec4 worldPos = finalWorld * vec4(position, 1.0);

    gl_Position = viewProjection * worldPos;

    vPositionW = vec3(worldPos);

#ifdef NORMAL
    vNormalW = normalize(vec3(finalWorld * vec4(normal, 0.0)));
#endif

    // Clip plane
#include<clipPlaneVertex>

    // Fog
#include<fogVertex>
#include<shadowsVertex>[0..maxSimultaneousLights]

    // Point size
#ifdef POINTSIZE
    gl_PointSize = pointSize;
#endif
}

)ShaderCode";

} // end of namespace BABYLON

#endif // end of BABYLON_MATERIALS_LIBRARY_SHADOWONLY_SHADOW_ONLY_VERTEX_FX_H
