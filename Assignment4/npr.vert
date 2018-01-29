varying vec3 lightDir, normal;
varying vec4 position;

void main()
{
	lightDir = normalize(vec3(gl_LightSource[0].position));
	normal = gl_NormalMatrix * gl_Normal;
	position = gl_ModelViewMatrix * gl_Vertex;

	gl_Position = ftransform();
	// gl_Position = gl_ModelViewProjectionMatrix * (gl_Vertex + vec4(normal, 1.0)*0.1);
} 
