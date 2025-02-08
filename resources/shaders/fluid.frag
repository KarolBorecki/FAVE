#version 330 core

out vec4 FragColor;

in vec3 crntPos;
in vec3 Normal;
in vec3 color;
in vec2 texCoord;

uniform sampler2D diffuse0;  // Tekstura wody (jeśli istnieje)
uniform sampler2D refractionMap; // Renderowana scena dla efektu refrakcji
uniform vec3 lightPos;
uniform vec3 camPos;
uniform vec4 lightColor;
uniform bool useTexture;

const float IOR = 1.33; // Współczynnik załamania wody
const float fresnelPower = 5.0;
const float transparency = 0.75; // Podstawowa przezroczystość wody

vec3 fresnelEffect(vec3 normal, vec3 viewDir) {
    float fresnel = pow(1.0 - max(dot(normal, viewDir), 0.0), fresnelPower);
    return vec3(fresnel);
}

void main()
{
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(camPos - crntPos);

    // Efekt Fresnela dla odbić/reflekcji
    float fresnel = pow(1.0 - max(dot(viewDir, normal), 0.0), fresnelPower);

    // Refrakcja – przesunięcie próbki w teksturze na podstawie normalnej
    vec2 refractedUV = texCoord + normal.xy * (1.0 / IOR) * 0.02;
    vec4 refractedColor = texture(refractionMap, refractedUV);

    // Pobranie koloru wody
    vec4 waterColor = useTexture ? texture(diffuse0, texCoord) : vec4(color, 1.0);

    // Połączenie refrakcji i koloru wody
    vec4 finalColor = mix(refractedColor, waterColor, transparency);

    // Uwzględnienie Fresnela - bardziej przezroczyste na wprost, bardziej refleksyjne na bokach
    finalColor.rgb += fresnel * lightColor.rgb * 0.5;

    FragColor = finalColor;
}
