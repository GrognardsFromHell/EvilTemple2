
const int Light_Directional = 1;
const int Light_Point = 2;

struct LightSource {
	int type;
	vec4 color;
	vec4 position;
	float attenuation; // 4/(light.Range*light.Range)
};

uniform LightSource lightSource;
uniform mat4 worldMatrix;
uniform mat4 worldInverseMatrix;
uniform mat4 worldViewInverseMatrix;

varying vec3 lightVector;
varying vec3 halfVector;
varying float lightDistance;

void lighting(vec4 vertexPosition)
{
	if (lightSource.type == Light_Directional) {
		lightVector = - vec3(normalize(worldInverseMatrix * lightSource.position));
		lightDistance = 0;
	} else {
		lightVector = vec3(lightSource.position - worldMatrix * vertexPosition);
		// TODO: Scaling might negatively affect this. Maybe convert to world instead?
		lightDistance = length(lightVector) * 2;
		lightVector = lightVector; // Normalize
	}
	
	vec3 eyeVector = vec3(worldViewInverseMatrix * vec4(0.0, 0.0, 1.0, 0.0));
	halfVector = normalize(lightVector + eyeVector);
}
