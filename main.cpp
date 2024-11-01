#include "FAVE.h"

FAVE::Vertex vertices[] =
    { //               COORDINATES           /            COLORS          /           NORMALS         /       TEXTURE COORDINATES    //
        FAVE::Vertex{glm::vec3(-1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
        FAVE::Vertex{glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f)},
        FAVE::Vertex{glm::vec3(1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
        FAVE::Vertex{glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)}};

GLuint indices[] = {0, 1, 2, 0, 2, 3};

int main()
{
    FAVE::Core::init();
    FAVE::Window::open(1800, 1200, "FAVE");

    FAVE::Scene scene;

    FAVE::Shader shaderProgram("./resources/shaders/default.vert", "./resources/shaders/default.frag");

    std::vector<FAVE::Vertex> verts(vertices, vertices + sizeof(vertices) / sizeof(FAVE::Vertex));
    std::vector<GLuint> ind(indices, indices + sizeof(indices) / sizeof(GLuint));

    FAVE::Texture planksDiffuse("./resources/textures/planks.png", FAVE::TextureType::DIFFUSE, 0, GL_RGBA, GL_UNSIGNED_BYTE);
    FAVE::Texture planksSpecular("./resources/textures/planksSpec.png", FAVE::TextureType::SPECULAR, 1, GL_RED, GL_UNSIGNED_BYTE);

    FAVE::Material material(shaderProgram, &planksDiffuse, &planksSpecular);
    FAVE::Mesh floor(verts, ind, material);

    FAVE::Light light({1.0f, 1.0f, 1.0f, 1.0f});
    light.setPosition({0.5f, 0.5f, 0.5f});

    FAVE::Camera camera;

    scene.setCamera(&camera);
    scene.setLight(&light);
    scene.addObject(&floor);
    FAVE::Window::setScene(&scene);

    FAVE::Window::start();

    FAVE::Window::close();

    FAVE::Core::finish();

    return 0;
}
