#version 330 core 

//normal of the face
in vec3 fragNor;
in vec3 fragPos;
in vec3 viewDir;

//material diffuse color 
uniform vec3 MatDif;
uniform vec3 MatAmb;
uniform vec3 MatSpec;
uniform float shine;

//directional light
uniform vec3 dL1position;
uniform vec3 dL1color;

//point light
uniform vec3 pL1position;
uniform vec3 pL1color;
uniform float pL1constant;
uniform float pL1linear;
uniform float pL1quadratic;

//output fragment color
out vec4 color;

float dot(vec3 normal, vec3 lightDir);
vec3 calDirLight(vec3 normal);
vec3 calPointLight(vec3 normal);

void main()
{
	vec3 normal = normalize(fragNor);
	vec3 resLight = vec3(0,0,0);

	//calculate directional lighting
	resLight = calDirLight(normal);

	//calculate point lighting
	resLight += calPointLight(normal);

	color = vec4(resLight, 1.0);
}

vec3 calDirLight(vec3 normal)
{
	//diffuse lighting
	vec3 lightDir = normalize(dL1position - fragPos); 
	float diff = max(dot(normal,lightDir), 0.0);
	vec3 diffuse = diff * dL1color;

	//specular lighting
	vec3 halfVec = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal,halfVec), 0.0), shine);
	vec3 specular = spec * dL1color;

	vec3 result = diffuse * MatDif + dL1color * MatAmb + specular * MatSpec;
	return result;
}


vec3 calPointLight(vec3 normal)
{

	//diffuse lighting
	vec3 lightDir = normalize(pL1position - fragPos); 
	float diff = max(dot(normal,lightDir), 0.0);
	vec3 diffuse = diff * pL1color;

	//specular lighting
	vec3 halfVec = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal,halfVec), 0.0), shine);
	vec3 specular = spec * pL1color;

	//calculate the attenuation
	float distance = length(pL1position - fragPos);
	float intensity = 1.0;
	float attenuation = intensity / (pL1constant + pL1linear * distance + pL1quadratic *(distance * distance));

	vec3 result = pL1color * MatAmb + diffuse * MatDif * attenuation + specular * MatSpec * attenuation;
	return result;
}

float dot(vec3 normal, vec3 lightDir)
{
	float product = 0;
	for (int i = 0; i<3; i++)
	{
		product += normal[i]*lightDir[i];
	}
	return product;
}

float length(vec3 vector)
{
	float res = 0;
	for (int i = 0; i<3; i++)
	{
		res += vector[i]*vector[i];
	}
	return sqrt(res);
}
