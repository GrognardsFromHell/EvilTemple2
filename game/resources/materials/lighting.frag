
const int Light_Directional = 1;
const int Light_Point = 2;
const int Light_Spot = 3;

/*
const int MaxLights = 6;
uniform vec4 lightSourceColor[MaxLights];
*/

/*varying vec3 normal;
varying vec3 lightVector[MaxLights];
varying vec3 halfVector[MaxLights];
varying float attenuation[MaxLights];*/

varying vec4 lightingColor;

vec4 lighting(vec4 specularColor) {
    return lightingColor;

    /*vec4 color = vec4(0,0,0,1);

    vec3 n = normalize(normal);

    for (int i = 0; i < MaxLights; ++i) {
        vec3 lightDir = normalize(lightVector[i]);

        // compute the dot product between normal and ldir
        float NdotL = max(dot(n,lightDir), 0.0f);

        if (NdotL > 0.0) {
            color += attenuation[i] * lightSourceColor[i] * NdotL;

            vec3 halfV = normalize(halfVector[i]);
            float NdotHV = max(dot(n, halfV),0.0);
            color += specularColor * attenuation[i] * lightSourceColor[i] * pow(NdotHV, shininess);
        }
    }

    return color * materialColor;*/
}

vec4 lighting()
{
    return lighting(vec4(1,1,1,1));
}

vec4 lightingGlossmap(sampler2D glossmapSampler, vec2 texCoords)
{
    // Retrieve the specular material color from the glossmap
    vec4 specularColor = texture2D(glossmapSampler, texCoords);


    return lighting(vec4(1,1,1,1));
}
