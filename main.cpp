#define _USE_MATH_DEFINES

#include "mesh.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "voxel_array.hpp"
#include "octree.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "gl_includes.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include <random>

void reloadShaders();

// Window parameters
GLFWwindow *g_window {};

// GPU objects
GLuint g_program {};  // A GPU program contains at least a vertex shader and a fragment shader

Camera g_camera {};

std::shared_ptr<Mesh> g_mesh {};
std::shared_ptr<Octree> g_octree {};

float g_fps = 0.0f;

bool g_reloadShaders = false;

std::shared_ptr<VoxelArray> g_voxelArray {};

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback(GLFWwindow *window, int width, int height) {
    g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    glViewport(0, 0, (GLint)width, (GLint)height);  // Dimension of the rendering region in the window
}

// Executed each time a key is entered.
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if(action == GLFW_PRESS) {
        if (key == GLFW_KEY_W) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        if (key == GLFW_KEY_F) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        if(key == GLFW_KEY_R) {
            g_reloadShaders = true;
        }
        if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)) {
            glfwSetWindowShouldClose(window, true);  // Closes the application if the escape key is pressed
        }

    }
}

float g_cameraDistance = 64.0f;
float g_cameraAngleX = 0.0f;

// Scroll for zooming
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    g_cameraDistance -= yoffset * 0.1f;
    g_cameraDistance = std::max(g_cameraDistance, 0.1f);

    g_cameraAngleX -= xoffset * 0.04f;
}

void errorCallback(int error, const char *desc) {
    std::cout << "Error " << error << ": " << desc << std::endl;
}

void initGLFW() {
    glfwSetErrorCallback(errorCallback);

    // Initialize GLFW, the library responsible for window management
    if (!glfwInit()) {
        std::cerr << "ERROR: Failed to init GLFW" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Before creating the window, set some option flags
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // Create the window
    g_window = glfwCreateWindow(
        1024, 768,
        "Voxel octrees rendering", nullptr, nullptr);
    if (!g_window) {
        std::cerr << "ERROR: Failed to open window" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    // Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
    glfwMakeContextCurrent(g_window);
    glfwSetWindowSizeCallback(g_window, windowSizeCallback);
    glfwSetKeyCallback(g_window, keyCallback);
    glfwSetScrollCallback(g_window, scrollCallback);
}

void initImGui() {
    // Setup Dear ImGui context
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(g_window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
}

void initOpenGL() {
    // Load extensions for modern OpenGL
    if (!gladLoadGL()) {
        std::cerr << "ERROR: Failed to initialize OpenGL context" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    glEnable(GL_DEBUG_OUTPUT);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);  // specify the background color, used any time the framebuffer is cleared

    // Disable v-sync
    glfwSwapInterval(0);
}

void initGPUprogram() {
    g_program = glCreateProgram();
    loadShader(g_program, GL_VERTEX_SHADER, "resources/vertexShader.glsl");
    loadShader(g_program, GL_FRAGMENT_SHADER, "resources/fragmentShader.glsl");
    glLinkProgram(g_program);

    glUseProgram(g_program);
}


void initCPUgeometry() {
    g_mesh = Mesh::genPlane();
    g_voxelArray = std::make_shared<VoxelArray>(128, 128, 128);

    g_octree = std::make_shared<Octree>(7);

    for(int i=0; i<128; i++) {
        for(int j=0; j<128; j++) {
            for(int k=0; k<128; k++) {
                glm::vec3 color = g_voxelArray->colorData[i + j * 128 + k * 128 * 128];
                if(glm::length(color) > 0.0f) {
                    unsigned int r = color.x * 255.0f;
                    unsigned int g = color.y * 255.0f;
                    unsigned int b = color.z * 255.0f;
                    g_octree->insert(i, j, k, r << 16 | g << 8 | b);
                }
            }
        }
    }

    //g_octree->print();

    g_octree->generateTexture();
}

void initCamera() {
    int width, height;
    glfwGetWindowSize(g_window, &width, &height);
    g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));

    g_camera.setPosition(glm::vec3(0.0, 0.0, 3.0));
    g_camera.setNear(0.1);
    g_camera.setFar(80);

    g_camera.setFoV(90);
}

void init() {
    initGLFW();
    initOpenGL();

    initCPUgeometry();
    initGPUprogram();
    initCamera();
    
    initImGui();
}

void clear() {
    glDeleteProgram(g_program);

    glfwDestroyWindow(g_window);
    glfwTerminate();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

}

void renderUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Start drawing here 

    ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("FPS: %.1f", g_fps);

    ImGui::End();

    // End drawing here

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// The main rendering call
void render() {

    if(g_reloadShaders) {
        reloadShaders();
        g_reloadShaders = false;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Erase the color and z buffers.

    const glm::mat4 viewMatrix = g_camera.computeViewMatrix();
    const glm::mat4 projMatrix = g_camera.computeProjectionMatrix();
    
    setUniform(g_program, "u_viewMat", viewMatrix);
    setUniform(g_program, "u_projMat", projMatrix);
    setUniform(g_program, "u_invViewMat", glm::inverse(viewMatrix));
    setUniform(g_program, "u_invProjMat", glm::inverse(projMatrix));

    setUniform(g_program, "u_modelMat", glm::mat4(1.0f));
    setUniform(g_program, "u_transposeInverseModelMat", glm::mat4(1.0f));

    setUniform(g_program, "u_cameraPosition", g_camera.getPosition());

    setUniform(g_program, "u_time", static_cast<float>(glfwGetTime()));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, g_voxelArray->colorTextureID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, g_voxelArray->normalTextureID);
    setUniform(g_program, "u_voxelMap.colorTex", 0);
    setUniform(g_program, "u_voxelMap.normalTex", 1);
    setUniform(g_program, "u_voxelMap.size", glm::ivec3(g_voxelArray->sizeX, g_voxelArray->sizeY, g_voxelArray->sizeZ));

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_3D, g_octree->textureID);
    setUniform(g_program, "u_octreeTex", 3);
    setUniform(g_program, "u_octreeDepth", g_octree->treeDepth);

    // Render objects
    
    g_mesh->render();

    renderUI();
}

// Update any accessible variable based on the current time
void update(const float currentTimeInSec) {

    // Update the FPS computation
    static int frameCount = 0;
    static float lastTime = glfwGetTime();
    frameCount++;

    if (currentTimeInSec - lastTime >= 1.0) {
        g_fps = static_cast<float>(frameCount) / (currentTimeInSec - lastTime);
        frameCount = 0;
        lastTime = currentTimeInSec;
    }

    // Update the camera position

    glm::vec3 targetPosition = glm::vec3(g_voxelArray->sizeX, g_voxelArray->sizeY, g_voxelArray->sizeZ) * 0.5f;
    g_camera.setTarget(targetPosition);

    glm::vec3 cameraOffset = glm::normalize(glm::vec3(cos(g_cameraAngleX), 0.0f, sin(g_cameraAngleX))) * (1.1f + g_cameraDistance);
    g_camera.setPosition(targetPosition + cameraOffset);
}

void reloadShaders() {
    
    GLuint g_newProgram = glCreateProgram();
    loadShader(g_newProgram, GL_VERTEX_SHADER, "../resources/vertexShader.glsl");
    loadShader(g_newProgram, GL_FRAGMENT_SHADER, "../resources/fragmentShader.glsl");
    glLinkProgram(g_newProgram);

    if(g_newProgram) {
        if(g_program)
            glDeleteProgram(g_program);
        g_program = g_newProgram;

        std::cout << "Shaders reloaded" << std::endl;
    } else {
        std::cout << "Shaders reloading error" << std::endl;
    }

    glUseProgram(g_program);

}

int main(int argc, char **argv) {
    init();
    while (!glfwWindowShouldClose(g_window)) {
        update(static_cast<float>(glfwGetTime()));
        render();
        glfwSwapBuffers(g_window);
        glfwPollEvents();
    }
    clear();
    return EXIT_SUCCESS;
}
