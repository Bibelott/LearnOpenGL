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

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

void main() {
    vec3 texDiff = texture(material.texture_diffuse1, TexCoords).rgb;

    vec3 ambient = light.ambient * texDiff;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texDiff;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfWay = normalize(lightDir + viewDir);
    float spec = pow(max(dot(halfWay, norm), 0.0), material.shininess); 
    vec3 specular = light.specular * spec * material.specular;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
};
