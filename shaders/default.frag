#version 450

layout(location = 0) in vec3 inPosWorld;
layout(location = 1) in vec3 inNormalWorld;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec4 outColor;

struct PointLight
{
    vec4 position;
    vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform Global 
{
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
} global;

layout(set = 1, binding = 0) uniform Material 
{
    float shininess;
} material;

layout(set = 1, binding = 1) uniform sampler2D tex;

layout(push_constant) uniform Push 
{
    mat4 modelMatrix; 
    mat4 normalMatrix;
} push;

void main()
{
    vec3 diffuseLight = global.ambientLightColor.xyz * global.ambientLightColor.w;
    vec3 specularLight = vec3(0.0);
    vec3 surfaceNormal = normalize(inNormalWorld);

    vec3 cameraPosWorld = global.inverseView[3].xyz;
    vec3 viewDirection = normalize(cameraPosWorld - inPosWorld);

    for (int i = 0; i < global.numLights; i++)
    {
        PointLight pointLight = global.pointLights[i];
        vec3 directionToLight = pointLight.position.xyz - inPosWorld;
        float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared
        directionToLight = normalize(directionToLight);
        vec3 intensity = pointLight.color.xyz * pointLight.color.w * attenuation;
        
        // diffuse light
        float cosAngleIncidence = max(dot(surfaceNormal, directionToLight), 0);
        diffuseLight += intensity * cosAngleIncidence ;

        // specular light
        vec3 halfAngle = normalize(directionToLight + viewDirection);
		float blinnTerm = dot(surfaceNormal, halfAngle);
		blinnTerm = clamp(blinnTerm, 0, 1);
		blinnTerm = pow(blinnTerm, material.shininess);
		specularLight += intensity * blinnTerm;
    }

    vec3 materialColor = texture(tex, inUV).rgb;
    outColor = vec4(diffuseLight * materialColor + specularLight * materialColor, 1.0);
}