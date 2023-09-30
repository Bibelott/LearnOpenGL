#version 460 core

struct Material {
    sampler2D texture_diffuse1;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in VS_OUT {
    vec2 TexCoords;
    vec3 Normal;
    vec3 FragPos;
} fs_in;

out vec4 FragColor;

uniform Material material;
uniform Light light;
uniform vec3 viewPos;
uniform samplerCube shadowMap;
uniform float far_plane;

float ShadowCalculation(vec3 fragPos) {
    vec3 fragToLight = fragPos - light.position;
    float closestDepth = texture(shadowMap, fragToLight).r * far_plane;
    float currentDepth = length(fragToLight);

    vec3 sampleOffsetDirections[20] = vec3[] (
        vec3( 1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1),
        vec3( 1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
        vec3( 1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
        vec3( 1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
        vec3( 0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
    );

    float bias = 0.15;
    float shadow = 0.0;
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 100.0;
    for (int i = 0; i < samples; i++) {
        float closestDepth = texture(shadowMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= far_plane;
        if (currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    
    shadow /= float(samples);
    return shadow;
}

void main() {
    vec3 texDiff = texture(material.texture_diffuse1, fs_in.TexCoords).rgb;

    vec3 ambient = light.ambient * texDiff;

    vec3 norm = normalize(fs_in.Normal);
    vec3 lightDir = normalize(light.position - fs_in.FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texDiff;

    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 halfWay = normalize(lightDir + viewDir);
    float spec = pow(max(dot(halfWay, norm), 0.0), material.shininess); 
    vec3 specular = light.specular * spec * material.specular;

    float distance = length(light.position - fs_in.FragPos);
    float shadow = ShadowCalculation(fs_in.FragPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular) * attenuation;
    FragColor = vec4(result, 1.0);
};
