#version 330 core
uniform sampler2D Texture0;

//directional light
uniform vec3 dL1position;
uniform vec3 dL1color;

//point light
uniform vec3 pL1position;
uniform vec3 pL1color;
uniform float pL1constant;
uniform float pL1linear;
uniform float pL1quadratic;

uniform float flip;
uniform float shine;
vec4 Ka;

in vec2 vTexCoord;
in vec3 fragNor;
in vec3 fragPos;
in vec3 viewDir;

out vec4 Outcolor;

vec4 calDirLight(vec3 normal, vec4 texColor0);
vec4 calPointLight(vec3 normal, vec4 texColor0);

void main() {

    Ka = vec4(0.2,0.2,0.2,0);
    vec3 normal = flip * normalize(fragNor);
	vec4 texColor0 = texture(Texture0, vTexCoord);
    
    Outcolor = calDirLight(normal, texColor0);
    Outcolor += calPointLight(normal, texColor0);

}

vec4 calDirLight(vec3 normal, vec4 texColor0)
{
	//diffuse lighting
	vec3 lightDir = normalize(dL1position - fragPos); 
	float diff = max(dot(normal,lightDir), 0.0);
    vec4 diffuse = diff * texColor0 * vec4(dL1color, 1.0);

	//specular lighting
	vec3 halfVec = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal,halfVec), 0.0), shine);
    vec4 specular = spec * texColor0 * vec4(dL1color, 1.0);
	
    vec4 result = Ka * texColor0 + diffuse + specular;
	result = vec4(result.xyz, 1.0);
	return result;
}

vec4 calPointLight(vec3 normal, vec4 texColor0)
{
	//diffuse lighting
	vec3 lightDir = normalize(pL1position - fragPos); 
	float diff = max(dot(normal,lightDir), 0.0);
    vec4 diffuse = diff * texColor0 * vec4(dL1color, 1.0);

	//specular lighting
	vec3 halfVec = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal,halfVec), 0.0), shine);
    vec4 specular = spec * texColor0 * vec4(dL1color, 1.0);
    
	//calculate the attenuation
	float distance = length(pL1position - fragPos);
	float intensity = 1.0;
	float attenuation = intensity / (pL1constant + pL1linear * distance + pL1quadratic *(distance * distance));

	vec4 result = Ka * texColor0 + diffuse * attenuation + specular * attenuation;
	result = vec4(result.xyz, 1.0);
	return result;
}