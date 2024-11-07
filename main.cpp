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
    std::vector<FAVE::Vertex> verts(vertices, vertices + sizeof(vertices) / sizeof(FAVE::Vertex)); // TODO implement reading obj
    std::vector<GLuint> ind(indices, indices + sizeof(indices) / sizeof(GLuint));

    FAVE::Core::init();
    FAVE::Window::open(1800, 1200, "FAVE");

    FAVE::Scene scene;

    FAVE::Shader shaderProgram("./resources/shaders/default.vert", "./resources/shaders/default.frag");

    FAVE::Texture planksDiffuse("./resources/textures/planks.png", FAVE::TextureType::DIFFUSE, 0, GL_RGBA, GL_UNSIGNED_BYTE);
    FAVE::Texture planksSpecular("./resources/textures/planksSpec.png", FAVE::TextureType::SPECULAR, 1, GL_RED, GL_UNSIGNED_BYTE);

    FAVE::Material material(shaderProgram, &planksDiffuse, &planksSpecular);
    // FAVE::Mesh floor(verts, ind, material);

    FAVE::FluidSimulation fluid(material, 50, 13, 50, 7, 0.2f);
    fluid.setPosition({-5.0f, -2.0f, -6.0f});

    FAVE::Light light({1.0f, 1.0f, 1.0f, 1.0f});
    light.setPosition({0.5f, 0.5f, 0.5f});

    FAVE::CameraController cameraController;
    FAVE::Camera camera;
    camera.setPosition({0.65f, 5.0f, 12.5f});
    camera.setOrientation({-0.03f, -0.22f, -1.0f});
    camera.attach(&cameraController);
    cameraController.m_camera = &camera;

    scene.setCamera(&camera);
    scene.setLight(&light);
    scene.addObject(&fluid);
    // scene.addObject(&floor);
    FAVE::Window::setScene(&scene);

    FAVE::Window::start();

    FAVE::Window::close();

    FAVE::Core::finish();

    return 0;
}
