
const int Light_Directional = 1;
const int Light_Point = 2;

uniform int lightSourceType;
uniform vec4 lightSourceColor;
uniform float lightSourceAttenuation;

varying vec3 normal;
varying vec3 lightVector;
varying vec3 halfVector;
varying float lightDistance;

vec4 lighting(float shininess, vec4 materialColor)
{
	vec4 color = vec4(0,0,0,1);
	
	vec3 n = normalize(normal);
	vec3 lightDir = normalize(lightVector);

	float attenuation = 1.0f;
	
	if (lightSourceType == Light_Point) {
		attenuation = min(1, 1.0 / (lightSourceAttenuation * lightDistance * lightDistance));
	}
	
	/* compute the dot product between normal and ldir */
	float NdotL = max(dot(n,lightDir), 0.0f);

	if (NdotL > 0.0) {
		color += attenuation * lightSourceColor * NdotL;
		
		vec3 halfV = normalize(halfVector);
		float NdotHV = max(dot(n, halfV),0.0);
		color += attenuation * lightSourceColor * pow(NdotHV, shininess);
	}

        return color * materialColor;
}

vec4 lightingGlossmap(float shininess, vec4 materialColor, sampler2D glossmapSampler, vec2 texCoords)
{
    // Retrieve the specular material color from the glossmap
    vec4 specularColor = texture2D(glossmapSampler, texCoords);

    vec4 color = vec4(0,0,0,1);

    vec3 n = normalize(normal);
    vec3 lightDir = normalize(lightVector);

    float attenuation = 1.0f;

    if (lightSourceType == Light_Point) {
            attenuation = min(1, 1.0 / (lightSourceAttenuation * lightDistance * lightDistance));
    }

    /* compute the dot product between normal and ldir */
    float NdotL = max(dot(n,lightDir), 0.0f);

    if (NdotL > 0.0) {
            color += materialColor * attenuation * lightSourceColor * NdotL;

            vec3 halfV = normalize(halfVector);
            float NdotHV = max(dot(n, halfV),0.0);
            color += specularColor * attenuation * lightSourceColor * pow(NdotHV, shininess);
    }

    return color;
}
