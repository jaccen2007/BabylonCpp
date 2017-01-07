﻿#ifndef BABYLON_MATERIALS_LIBRARY_GRID_GRID_FRAGMENT_FX_H
#define BABYLON_MATERIALS_LIBRARY_GRID_GRID_FRAGMENT_FX_H

namespace BABYLON {

extern const char* gridPixelShader;

const char* gridPixelShader
  = "#extension GL_OES_standard_derivatives : enable\n"
    "\n"
    "#define SQRT2 1.41421356\n"
    "#define PI 3.14159\n"
    "\n"
    "#ifdef GL_ES\n"
    "precision highp float;\n"
    "#endif\n"
    "\n"
    "uniform vec3 mainColor;\n"
    "uniform vec3 lineColor;\n"
    "uniform vec4 gridControl;\n"
    "\n"
    "// Varying\n"
    "varying vec3 vPosition;\n"
    "varying vec3 vNormal;\n"
    "\n"
    "#include<fogFragmentDeclaration>\n"
    "\n"
    "float getVisibility(float position) {\n"
    "  // Major grid line every Frequency defined in material.\n"
    "  float majorGridFrequency = gridControl.y;\n"
    "  if (floor(position + 0.5) == floor(position / majorGridFrequency + 0.5) * majorGridFrequency)\n"
    "  {\n"
    "  return 1.0;\n"
    "  }  \n"
    "  \n"
    "  return gridControl.z;\n"
    "}\n"
    "\n"
    "float getAnisotropicAttenuation(float differentialLength) {\n"
    "  const float maxNumberOfLines = 10.0;\n"
    "  return clamp(1.0 / (differentialLength + 1.0) - 1.0 / maxNumberOfLines, 0.0, 1.0);\n"
    "}\n"
    "\n"
    "float isPointOnLine(float position, float differentialLength) {\n"
    "  float fractionPartOfPosition = position - floor(position + 0.5); // fract part around unit [-0.5; 0.5]\n"
    "  fractionPartOfPosition /= differentialLength; // adapt to the screen space size it takes\n"
    "  fractionPartOfPosition = clamp(fractionPartOfPosition, -1., 1.);\n"
    "  \n"
    "  float result = 0.5 + 0.5 * cos(fractionPartOfPosition * PI); // Convert to 0-1 for antialiasing.\n"
    "  return result;  \n"
    "}\n"
    "\n"
    "float contributionOnAxis(float position) {\n"
    "  float differentialLength = length(vec2(dFdx(position), dFdy(position)));\n"
    "  differentialLength *= SQRT2;  // Multiply by SQRT2 for diagonal length\n"
    "  \n"
    "  // Is the point on the line.\n"
    "  float result = isPointOnLine(position, differentialLength);\n"
    "  \n"
    "  // Add dynamic visibility.\n"
    "  float visibility = getVisibility(position);\n"
    "  result *= visibility;\n"
    "  \n"
    "  // Anisotropic filtering.\n"
    "  float anisotropicAttenuation = getAnisotropicAttenuation(differentialLength);\n"
    "  result *= anisotropicAttenuation;\n"
    "  \n"
    "  return result;\n"
    "}\n"
    "\n"
    "float normalImpactOnAxis(float x) {\n"
    "  float normalImpact = clamp(1.0 - 2.8 * abs(x * x * x), 0.0, 1.0);\n"
    "  return normalImpact;\n"
    "}\n"
    "\n"
    "void main(void) {\n"
    "  \n"
    "  // Scale position to the requested ratio.\n"
    "  float gridRatio = gridControl.x;\n"
    "  vec3 gridPos = vPosition / gridRatio;\n"
    "  \n"
    "  // Find the contribution of each coords.\n"
    "  float x = contributionOnAxis(gridPos.x);\n"
    "  float y = contributionOnAxis(gridPos.y);\n"
    "  float z = contributionOnAxis(gridPos.z); \n"
    "  \n"
    "  // Find the normal contribution.\n"
    "  vec3 normal = normalize(vNormal);\n"
    "  x *= normalImpactOnAxis(normal.x);\n"
    "  y *= normalImpactOnAxis(normal.y);\n"
    "  z *= normalImpactOnAxis(normal.z);\n"
    "  \n"
    "  // Create the grid value by combining axis.\n"
    "  float grid = clamp(x + y + z, 0., 1.);\n"
    "  \n"
    "  // Create the color.\n"
    "  vec3 color = mix(mainColor, lineColor, grid);\n"
    "\n"
    "#ifdef FOG\n"
    "  #include<fogFragment>\n"
    "#endif\n"
    "\n"
    "#ifdef TRANSPARENT\n"
    "  float opacity = clamp(grid, 0.08, gridControl.w);\n"
    "  gl_FragColor = vec4(color.rgb, opacity);\n"
    "#else\n"
    "  // Apply the color.\n"
    "  gl_FragColor = vec4(color.rgb, 1.0);\n"
    "#endif\n"
    "}\n";

} // end of namespace BABYLON

#endif // end of BABYLON_MATERIALS_LIBRARY_GRID_GRID_FRAGMENT_FX_H
