
const int Light_Directional = 1;
const int Light_Point = 2;

struct LightSource {
	int type;
	vec4 color;
	vec4 position;
	float attenuation; // 4/(light.Range*light.Range)
};

uniform LightSource lightSource;

uniform mat4 worldViewInverseMatrix;
varying vec3 normal;
varying vec3 lightVector;
varying vec3 halfVector;
varying float lightDistance;

vec4 lighting(float shininess)
{
	vec4 color = vec4(0,0,0,1);
	
	vec3 n = normalize(normal);
	vec3 lightDir = normalize(lightVector);

	float attenuation = 1.0f;
	
	if (lightSource.type == Light_Point) {
		attenuation = min(1, 1.0 / (lightSource.attenuation * lightDistance * lightDistance));
	}
	
	/* compute the dot product between normal and ldir */
	float NdotL = max(dot(n,lightDir), 0.0f);

	if (NdotL > 0.0) {
		color += attenuation * lightSource.color * NdotL;
		
		vec3 halfV = normalize(halfVector);
		float NdotHV = max(dot(n, halfV),0.0);
		color += attenuation * lightSource.color * pow(NdotHV, shininess);
	}

	return color;
}