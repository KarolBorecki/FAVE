#include "FAVE.h"

#define MAX_FILEPATH_LENGTH 100
FAVE::Vertex vertices[] =
    { //               COORDINATES           /            COLORS          /           NORMALS         /       TEXTURE COORDINATES    //
        FAVE::Vertex{glm::vec3(-1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
        FAVE::Vertex{glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f)},
        FAVE::Vertex{glm::vec3(1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
        FAVE::Vertex{glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)}};

GLuint indices[] = {0, 1, 2, 0, 2, 3};

char default_vertex_shader_path[MAX_FILEPATH_LENGTH];
char default_fragment_shader_path[MAX_FILEPATH_LENGTH];
char planks_texture_path[MAX_FILEPATH_LENGTH];
char planks_specular_texture_path[MAX_FILEPATH_LENGTH];
const char *get_resources_path(char *dest, const char *resource_name)
{
    memset(&(dest[0]), 0, strlen(dest));
    FAVE::log("dest: \"%s\"", dest);
    const char *resources_dir = "./resources/";
    strcat(dest, resources_dir);
    strcat(dest, resource_name);
    FAVE::log("dest: \"%s\"", dest);
    return dest;
}

int main()
{
    std::vector<FAVE::Vertex> verts(vertices, vertices + sizeof(vertices) / sizeof(FAVE::Vertex)); // TODO implement reading obj
    std::vector<GLuint> ind(indices, indices + sizeof(indices) / sizeof(GLuint));

    FAVE::Core::init();
    FAVE::Window::open(900, 700, "FAVE");

    FAVE::Scene scene;

    FAVE::Shader shaderProgram(get_resources_path(default_vertex_shader_path, "shaders/default.vert"), get_resources_path(default_fragment_shader_path, "shaders/default.frag"));

    FAVE::Texture planksDiffuse(get_resources_path(planks_texture_path, "textures/planks.png"), FAVE::TextureType::DIFFUSE, 0, GL_RGBA, GL_UNSIGNED_BYTE);
    FAVE::Texture planksSpecular(get_resources_path(planks_specular_texture_path, "textures/planksSpec.png"), FAVE::TextureType::SPECULAR, 1, GL_RED, GL_UNSIGNED_BYTE);

    FAVE::Material material(shaderProgram, &planksDiffuse, &planksSpecular);
    // FAVE::Mesh floor(verts, ind, material);

    FAVE::log("creating object......");
    FAVE::FluidSimulation2D fluid(material, 1000.0f, 0.25f, 120, 70, 100);
    fluid.setPosition({0.0f, 0.0f, 0.0f});

    FAVE::log("creating light......");
    FAVE::Light light({1.0f, 1.0f, 1.0f, 1.0f});
    light.setPosition({0.5f, 0.5f, 0.5f});

    FAVE::log("creating camera......");
    FAVE::CameraController cameraController;
    FAVE::Camera camera;
    camera.setPosition({15.71f, 8.86f, 38.01f});
    camera.setOrientation({0.03f, 0.0f, -1.29f});
    FAVE::log("attaching camera to controller......");
    camera.attach(&cameraController);
    FAVE::log("setting camera to controller......");
    cameraController.m_camera = &camera;

    FAVE::log("adding objects to scene......");
    scene.setCamera(&camera);
    scene.setLight(&light);
    scene.addObject(&fluid);
    // scene.addObject(&floor);
    FAVE::log("Scene setting......");
    FAVE::Window::setScene(&scene);

    FAVE::log("window starting......");
    FAVE::Window::start();

    FAVE::Window::close();

    FAVE::Core::finish();

    return 0;
}
