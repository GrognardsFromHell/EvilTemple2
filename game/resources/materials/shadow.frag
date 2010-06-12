#version 120

uniform mat4 viewProjectionMatrix;
uniform mat4 worldMatrix;
const vec4 invertedLightDirection = vec4(0.447, 0.89, 0, 0);

attribute vec4 vertexPosition;

void main() {
  vec4 v = worldMatrix * vertexPosition;

  float lambda = v.y / invertedLightDirection.y; // y must be non-zero
  v.y = 1; // For depth bias
  v.x -= invertedLightDirection.x * lambda;
  v.z -= invertedLightDirection.z * lambda;

  gl_Position = viewProjectionMatrix * v;
}
