#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTex;

out vec3 crntPos;
out vec3 Normal;
out vec3 color;
out vec2 texCoord;

uniform mat4 camMatrix;
uniform vec3 scale;
uniform vec3 rotation;
uniform vec3 position;

mat4 createRotationMatrix(vec3 rotation) {
    float x = radians(rotation.x);
    float y = radians(rotation.y);
    float z = radians(rotation.z);

    mat4 rotX = mat4(
        1, 0,      0,     0,
        0, cos(x), -sin(x), 0,
        0, sin(x), cos(x),  0,
        0, 0,      0,      1
    );

    mat4 rotY = mat4(
        cos(y),  0, sin(y), 0,
        0,       1, 0,      0,
        -sin(y), 0, cos(y), 0,
        0,       0, 0,      1
    );

    mat4 rotZ = mat4(
        cos(z), -sin(z), 0, 0,
        sin(z), cos(z),  0, 0,
        0,      0,       1, 0,
        0,      0,       0, 1
    );

    return rotZ * rotY * rotX;
}

void main()
{
    mat4 model = mat4(1.0);
    model = model * createRotationMatrix(rotation);
    model = model * mat4(
        scale.x, 0.0,    0.0,    0.0,
        0.0,    scale.y, 0.0,    0.0,
        0.0,    0.0,    scale.z, 0.0,
        position.x, position.y, position.z, 1.0
    );

    crntPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal; 
    color = aColor;
    texCoord = aTex;

    gl_Position = camMatrix * vec4(crntPos, 1.0);
}
