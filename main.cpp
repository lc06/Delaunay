#include <glad/glad.h>
#include <glfw/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <StbImage/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <iostream>
#include "config.h"

#include "include/Delaunay.h"

int width = 800;
int height = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = (float)width  / 2.0f;
float lastY = (float)height / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int w, int h);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int loadTexture(char const *path);

int main() {
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, "Delaunay", nullptr, nullptr);

    if(window == nullptr) {
        std::cout << "GLFW Window Created Failed" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Init GLAD
    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "GLAD Initialized Failed!" << std::endl;
        return -1;
    }

    Delaunay delaunay(path + "resources/terrain.txt");

    std::vector<unsigned int> triIDs {};
    delaunay.generateTIN(triIDs);

    std::vector<float> vertices {};
    delaunay.getVertices(vertices);
    int verticesSize = vertices.size() * sizeof(float);

    Shader myShader((path + "resources/shaders/delaunay.vert").c_str(),
                    (path + "resources/shaders/delaunay.frag").c_str());
    Shader normalShader((path + "resources/shaders/normal.vert").c_str(),
                        (path + "resources/shaders/normal.frag").c_str(),
                        (path + "resources/shaders/normal.geom").c_str());

    // Prepare Data, Buffers, Shader...
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verticesSize, &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, triIDs.size() * sizeof(unsigned int), &triIDs[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 1, GL_FLOAT, GL_TRUE, 7 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(1 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    unsigned int terrainTexture  = loadTexture((path + "resources/textures/new.bmp").c_str());
//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    myShader.use();
    glm::mat4 model(1.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.f);
    myShader.setMat4("model", model);
    myShader.setMat4("view", view);
    myShader.setMat4("projection", projection);

    myShader.setVec3("ambient", glm::vec3(1.f, 1.f, 1.f));
    myShader.setVec3("diffuse", glm::vec3(1.f, 1.f, 1.f));
    myShader.setVec3("lightPosition", glm::vec3(2.0f, 2.0f, 2.0f));

    myShader.setInt("ourTexture", 0);

    normalShader.use();
    normalShader.setMat4("model", model);
    normalShader.setMat4("view", view);
    normalShader.setMat4("projection", projection);

    // Rendering loop
    while(!glfwWindowShouldClose(window)) {

        auto currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Resolve input
        processInput(window);

        // Clear Buffer
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.2f, 0.3f, 0.5f, 1.0f);

        // Use Shader, rendering models
        myShader.use();
        model = glm::mat4(1.0f);
        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
        myShader.setMat4("model", model);
        myShader.setMat4("view", view);
        myShader.setMat4("projection", projection);

        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, terrainTexture);
        glDrawElements(GL_TRIANGLES, triIDs.size(), GL_UNSIGNED_INT, nullptr);

//        glPointSize(3.0f);
//        glDrawArrays(GL_POINTS, 0, delaunay.getPointSize());
        glBindVertexArray(0);

//        normalShader.use();
//        normalShader.setMat4("model", model);
//        normalShader.setMat4("view", view);
//        normalShader.setMat4("projection", projection);
//
//        glBindVertexArray(VAO);
//        glDrawElements(GL_TRIANGLES, triIDs.size(), GL_UNSIGNED_INT, nullptr);
//
//        glPointSize(3.0f);
//        glDrawArrays(GL_POINTS, 0, delaunay.getPointSize());
//        glBindVertexArray(0);

        // Swap Buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Release Buffers
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int w, int h) {
    glViewport(0, 0, w, h);
    width = w;
    height = h;
}

void processInput(GLFWwindow* window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(char const *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        if(nrComponents == 4) {
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}