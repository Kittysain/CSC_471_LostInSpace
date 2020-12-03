#version 330 core 
out vec4 color;
in vec3 fragNor;

void main()
{
	vec3 normal = normalize(fragNor);
	float depth=normal.z;
	vec3 solColor = vec3(0.49*depth,0.28*depth,0.58*depth);
	color = vec4(solColor, 1.0);
}
