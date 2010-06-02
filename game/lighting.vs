
const int Light_Directional = 1;
const int Light_Point = 2;

uniform int lightSourceType;
uniform vec4 lightSourcePosition;

uniform mat4 worldInverseMatrix;
uniform mat4 worldViewMatrix;
uniform mat4 worldViewInverseMatrix;
uniform mat4 viewMatrix;

varying vec3 normal;
varying vec3 lightVector;
varying vec3 halfVector;
varying float lightDistance;

void lighting(vec4 vertexPosition, vec4 vertexNormal)
{
	normal = normalize(vec3(worldViewMatrix * vertexNormal));

	if (lightSourceType == Light_Directional) {
		lightVector = normalize(- vec3(viewMatrix * lightSourcePosition));
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
