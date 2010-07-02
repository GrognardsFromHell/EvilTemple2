
const int Light_Directional = 1;
const int Light_Point = 2;

uniform int lightSourceType;
uniform vec4 lightSourcePosition;
uniform vec4 lightSourceDirection;

varying vec3 normal;
varying vec3 lightVector;
varying vec3 halfVector;
varying float lightDistance;

void lighting(vec4 vertexPosition, vec4 vertexNormal, mat4 worldViewMatrix, mat4 viewMatrix)
{
        normal = normalize(vec3(worldViewMatrix * normalize(vertexNormal)));

        if (lightSourceType == Light_Directional) {
                lightVector = normalize(- vec3(viewMatrix * normalize(lightSourceDirection)));
                lightDistance = 0;
        } else {
                lightVector = vec3(viewMatrix * lightSourcePosition) - vec3(worldViewMatrix * vertexPosition);
                // TODO: Scaling might negatively affect this. Maybe convert to world instead?
                lightDistance = length(lightVector);
                lightVector = normalize(lightVector); // Normalize
        }

        vec3 eyeVector = vec3(0.0, 0.0, -1.0);
        halfVector = normalize(lightVector + eyeVector);
}
