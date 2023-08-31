#version 460 core

struct Material {
   sampler2D diffuse;
   sampler2D specular;
   float shininess;
};

struct Light {
   vec3 position;
   vec3 direction;
   float cutOff;

   vec3 ambient;
   vec3 diffuse;
   vec3 specular;

   float constant;
   float linear;
   float quadratic;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

void main() {
   vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

   vec3 lightDir = normalize(light.position - FragPos);
   float theta = dot(lightDir, normalize(-light.direction));

   float distance = length(light.position - FragPos);
   float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

   vec3 result;

   if (theta > light.cutOff) {
      vec3 norm = normalize(Normal);
      float diff = max(dot(norm, lightDir), 0.0);
      vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

      vec3 viewDir = normalize(viewPos - FragPos);
      vec3 reflectDir = reflect(-lightDir, norm);
      float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
      vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;

      result = (ambient + diffuse + specular) * attenuation;
   }
   else {
      result = ambient * attenuation;
   }
   FragColor = vec4(result, 1.0);
};
