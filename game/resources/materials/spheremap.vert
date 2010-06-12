
varying vec3 eyeSpacePos;
varying vec3 eyeSpaceNormal;

void sphereMapping(vec4 vertPos, vec4 vertNormal, mat4 worldView, mat4 worldViewInverseTranspose) {
    eyeSpacePos = (worldView * vertPos).xyz;
    eyeSpaceNormal = (worldViewInverseTranspose * vertNormal).xyz;
}
