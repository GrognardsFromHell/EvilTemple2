
/*
    These are contributed by the three triangle vertices.
 */
varying vec3 eyeSpacePos;
varying vec3 eyeSpaceNormal;

vec2 sphereMapping() {
    vec3 reflection;

    reflection = reflect(normalize(eyeSpacePos), normalize(eyeSpaceNormal));
    reflection.z += 1;
    reflection = normalize(reflection);
    return reflection.xy * 0.5 + 0.5;
}
