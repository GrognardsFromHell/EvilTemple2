
/*
    These are contributed by the three triangle vertices.
 */
varying vec3 eyeSpacePos;
varying vec3 eyeSpaceNormal;

vec2 sphereMapping() {
    // Renormalize after interpolation
    vec3 u = normalize(eyeSpacePos);
    vec3 n = normalize(eyeSpaceNormal);

    vec3 f = u - 2 * n * dot(n, u);
    f.z += 1;

    float m = 2 * length(f);

    vec2 result;

    result.x = f.x / m + 0.5;
    result.y = f.y / m + 0.5;

    return result;
}
