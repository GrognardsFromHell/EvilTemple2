
const int Light_Directional = 1;
const int Light_Point = 2;
const vec3 eyeVector = vec3(0.0, 0.0, -1.0);

const int MaxLights = 10;

uniform int lightSourceType[MaxLights];
uniform vec4 lightSourcePosition[MaxLights];
uniform vec4 lightSourceDirection[MaxLights];
uniform float lightSourceAttenuation[MaxLights];
uniform vec4 lightSourceColor[MaxLights];

uniform mat4 worldMatrix;

/*varying vec3 normal;
varying vec3 lightVector[MaxLights];
varying vec3 halfVector[MaxLights];
varying float attenuation[MaxLights];*/

varying vec4 lightingColor;

void lighting(vec4 vertexPosition, vec4 vertexNormal, mat4 worldViewMatrix, mat4 viewMatrix)
{
    /*normal = normalize(vec3(worldViewMatrix * normalize(vertexNormal)));

    vec3 vertPos3View = (worldViewMatrix * vertexPosition).xyz;
    vec3 vertPos3World = (worldMatrix * vertexPosition).xyz;

    for (int i = 0; i < MaxLights; ++i) {
        if (lightSourceType[i] == Light_Directional) {
                lightVector[i] = normalize(- vec3(viewMatrix * normalize(lightSourceDirection[i])));
                attenuation[i] = 1.0f;
        } else {
                lightVector[i] = lightSourcePosition[i].xyz - vertPos3World;
                // TODO: Scaling might negatively affect this. Maybe convert to world instead?
                float lightDistanceSquare = dot(lightVector[i], lightVector[i]);

                lightVector[i] = (viewMatrix * lightSourcePosition[i]).xyz - vertPos3View;
                lightVector[i] = normalize(lightVector[i]); // Normalize

                attenuation[i] = min(1, 1.0 / (lightSourceAttenuation[i] * lightDistanceSquare));
        }

        halfVector[i] = normalize(lightVector[i] + eyeVector);

    }*/

    vec4 color = vec4(0,0,0,1);

    vec3 normal = normalize(vec3(worldViewMatrix * normalize(vertexNormal)));

    vec3 vertPos3View = (worldViewMatrix * vertexPosition).xyz;
    vec3 vertPos3World = (worldMatrix * vertexPosition).xyz;

    for (int i = 0; i < MaxLights; ++i) {
        vec3 lightVector;
        float attenuation;

        if (lightSourceType[i] == Light_Directional) {
                lightVector = normalize(- vec3(viewMatrix * normalize(lightSourceDirection[i])));
                attenuation = 1.0f;
        } else {
                lightVector = lightSourcePosition[i].xyz - vertPos3World;

                // TODO: Scaling might negatively affect this. Maybe convert to world instead?
                float lightDistanceSquare = dot(lightVector, lightVector);

                lightVector = normalize((viewMatrix * lightSourcePosition[i]).xyz - vertPos3View);

                attenuation = min(1, 1.0 / (lightSourceAttenuation[i] * lightDistanceSquare));
        }

        vec3 halfVector = normalize(lightVector + eyeVector);

        float shininess = 50;

        // compute the dot product between normal and ldir
        float NdotL = max(dot(normal, lightVector), 0.0f);

        if (NdotL > 0.0) {
            color += attenuation * lightSourceColor[i] * NdotL;

            float NdotHV = max(dot(normal, halfVector),0.0);
            color += attenuation * lightSourceColor[i] * pow(NdotHV, shininess);
        }
    }

    lightingColor = color;
}
