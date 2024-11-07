#version 330 core

out vec4 FragColor;

in vec3 crntPos;
in vec3 Normal;
in vec3 color;
in vec2 texCoord;

uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform vec4 lightColor;
uniform vec3 lightPos;
uniform vec3 camPos;
uniform bool useTexture; // New uniform to check if texture is used

vec4 pointLight()
{   
    vec3 lightVec = lightPos - crntPos;
    float dist = length(lightVec);
    float a = 3.0;
    float b = 0.7;
    float inten = 1.0f / (a * dist * dist + b * dist + 1.0f);

    float ambient = 0.20f;
    vec3 normal = normalize(Normal);
    vec3 lightDirection = normalize(lightVec);
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    float specularLight = 0.50f;
    vec3 viewDirection = normalize(camPos - crntPos);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
    float specular = specAmount * specularLight;

    vec4 textureColor = texture(diffuse0, texCoord);
    vec4 finalColor = textureColor * (diffuse * inten + ambient) + texture(specular0, texCoord).r * specular * inten;

    return useTexture ? finalColor : vec4(color, 1.0) * (diffuse * inten + ambient) + vec4(1.0) * specular * inten; // Use default color if no texture
}

vec4 direcLight()
{
    float ambient = 0.30f;
    vec3 normal = normalize(Normal);
    vec3 lightDirection = normalize(vec3(1.0f, 1.0f, 0.0f));
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    float specularLight = 0.30f;
    vec3 viewDirection = normalize(camPos - crntPos);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
    float specular = specAmount * specularLight;

    vec4 textureColor = texture(diffuse0, texCoord);
    vec4 finalColor = textureColor * (diffuse + ambient) + texture(specular0, texCoord).r * specular;

    return useTexture ? finalColor : vec4(color, 1.0) * (diffuse + ambient) + vec4(1.0) * specular; // Use default color if no texture
}

vec4 spotLight()
{
    float outerCone = 0.90f;
    float innerCone = 0.95f;
    float ambient = 0.20f;

    vec3 normal = normalize(Normal);
    vec3 lightDirection = normalize(lightPos - crntPos);
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    float specularLight = 0.50f;
    vec3 viewDirection = normalize(camPos - crntPos);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
    float specular = specAmount * specularLight;

    float angle = dot(vec3(0.0f, -1.0f, 0.0f), -lightDirection);
    float inten = clamp((angle - outerCone) / (innerCone - outerCone), 0.0f, 1.0f);

    vec4 textureColor = texture(diffuse0, texCoord);
    vec4 finalColor = textureColor * (diffuse * inten + ambient) + texture(specular0, texCoord).r * specular * inten;

    return useTexture ? finalColor : vec4(color, 1.0) * (diffuse * inten + ambient) + vec4(1.0) * specular * inten; // Use default color if no texture
}

void main()
{
    FragColor = direcLight(); // Change this to pointLight() or spotLight() as needed
}
