#version 460 core

struct Material {
    sampler2D texture_diffuse1;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in VS_OUT {
    vec2 TexCoords;
    vec3 Normal;
    vec3 FragPos;
    vec4 FragPosLightSpace;
} fs_in;

out vec4 FragColor;

uniform Material material;
uniform Light light;
uniform vec3 viewPos;
uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float shadow = currentDepth > closestDepth ? 1.0 : 0.0;

    return shadow;
}

void main() {
    vec3 texDiff = texture(material.texture_diffuse1, fs_in.TexCoords).rgb;

    vec3 ambient = light.ambient * texDiff;

    vec3 norm = normalize(fs_in.Normal);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texDiff;

    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 halfWay = normalize(lightDir + viewDir);
    float spec = pow(max(dot(halfWay, norm), 0.0), material.shininess); 
    vec3 specular = light.specular * spec * material.specular;

    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);
    FragColor = vec4(result, 1.0);
};
