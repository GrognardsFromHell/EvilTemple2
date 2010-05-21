
const int Light_Directional = 1;
const int Light_Point = 2;

struct LightSource {
	int type;
	vec4 color;
	vec4 position;
	float attenuation; // 4/(light.Range*light.Range)
};

uniform LightSource lightSource;

varying vec3 lightVector;
varying vec3 halfVector;
varying float lightDistance;

vec4 lighting(vec3 normal, float shininess)
{
	vec4 color = vec4(0,0,0,1);
	
	float attenuation = 1.0f;
	
	if (lightSource.type == Light_Point) {
		attenuation = min(1, 1.0 / (lightSource.attenuation * lightDistance * lightDistance));
	} 
	
	/* compute the dot product between normal and ldir */
	float NdotL = max(dot(normal,normalize(lightVector)), 0.0f);

	if (NdotL > 0.0) {
		color += attenuation * lightSource.color * NdotL;
		
		vec3 halfV = normalize(halfVector);
		float NdotHV = max(dot(normal, halfV),0.0);
		color += attenuation * lightSource.color * pow(NdotHV, shininess);
	}

	return color;
}
