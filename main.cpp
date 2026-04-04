#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "Model.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Камера
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// Время
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Мышь
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Поле зрения
float fov = 45.0f;

// Параметры движения
float lowerPlatformZ = 0.0f;      // [-1.0 ; 1.0]
float upperPlatformAngle = 0.0f;  // [-180 ; 180]
float arrowZ = 0.0f;              // [-0.4 ; 0.9]
float pistonY = 0.1f;             // [0.1 ; 0.8]

// Центр вращения верхней платформы
glm::vec3 upperPivot(0.0f, -0.1f, 0.2f);

void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int main()
{
    if (!glfwInit())
    {
        std::cerr << "ERROR: could not start GLFW3" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lab 7 - Affine Transformations", NULL, NULL);
    if (!window)
    {
        std::cerr << "ERROR: could not create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    GLenum ret = glewInit();
    if (ret != GLEW_OK)
    {
        std::cerr << "GLEW init error: " << glewGetErrorString(ret) << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    Shader shaderProgram("vertex_shader.glsl", "fragment_shader.glsl");

    Model baseModel("base.obj");
    Model lowerModel("lower.obj");
    Model upperModel("upper.obj");
    Model arrowModel("arrow.obj");
    Model pistonModel("piston.obj");

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderProgram.use();

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(
            glm::radians(fov),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f,
            100.0f
        );

        shaderProgram.setMat4("view", view);
        shaderProgram.setMat4("projection", projection);
        shaderProgram.setVec3("viewPos", cameraPos);

        // Свет
        shaderProgram.setVec3("light.position", glm::vec3(2.0f, 2.0f, 2.0f));
        shaderProgram.setVec3("light.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
        shaderProgram.setVec3("light.diffuse", glm::vec3(0.7f, 0.7f, 0.7f));
        shaderProgram.setVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));

        // Материал
        shaderProgram.setVec3("material.ambient", glm::vec3(1.0f, 0.5f, 0.0f));
        shaderProgram.setVec3("material.diffuse", glm::vec3(1.0f, 0.5f, 0.0f));
        shaderProgram.setVec3("material.specular", glm::vec3(0.8f, 0.8f, 0.8f));
        shaderProgram.setFloat("material.shininess", 32.0f);

        // 1. Основание
        glm::mat4 baseMatrix = glm::mat4(1.0f);
        shaderProgram.setMat4("model", baseMatrix);
        baseModel.Draw(shaderProgram);

        // 2. Нижняя платформа: вперед/назад по Z
        glm::mat4 lowerMatrix = baseMatrix;
        lowerMatrix = glm::translate(lowerMatrix, glm::vec3(0.0f, 0.0f, lowerPlatformZ));
        shaderProgram.setMat4("model", lowerMatrix);
        lowerModel.Draw(shaderProgram);

        // 3. Верхняя платформа: вращение вокруг Y с заданным центром
        glm::mat4 upperMatrix = lowerMatrix;
        upperMatrix = glm::translate(upperMatrix, upperPivot);
        upperMatrix = glm::rotate(
            upperMatrix,
            glm::radians(upperPlatformAngle),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        upperMatrix = glm::translate(upperMatrix, -upperPivot);
        shaderProgram.setMat4("model", upperMatrix);
        upperModel.Draw(shaderProgram);

        // 4. Стрела: движение по Z, зависит от верхней платформы
        glm::mat4 arrowMatrix = upperMatrix;
        arrowMatrix = glm::translate(arrowMatrix, glm::vec3(0.0f, 0.0f, arrowZ));
        shaderProgram.setMat4("model", arrowMatrix);
        arrowModel.Draw(shaderProgram);

        // 5. Поршень: движение по Y, зависит от стрелы
        glm::mat4 pistonMatrix = arrowMatrix;
        pistonMatrix = glm::translate(pistonMatrix, glm::vec3(0.0f, pistonY, 0.0f));
        shaderProgram.setMat4("model", pistonMatrix);
        pistonModel.Draw(shaderProgram);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    float cameraSpeed = 2.5f * deltaTime;
    float moveSpeed = 1.0f * deltaTime;
    float rotateSpeed = 60.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Камера
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    // Нижняя платформа по Z
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        lowerPlatformZ -= moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        lowerPlatformZ += moveSpeed;
    lowerPlatformZ = std::clamp(lowerPlatformZ, -1.0f, 1.0f);

    // Верхняя платформа вращение вокруг Y
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        upperPlatformAngle -= rotateSpeed;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        upperPlatformAngle += rotateSpeed;
    upperPlatformAngle = std::clamp(upperPlatformAngle, -180.0f, 180.0f);

    // Стрела по Z
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        arrowZ -= moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
        arrowZ += moveSpeed;
    arrowZ = std::clamp(arrowZ, -0.4f, 0.9f);

    // Поршень по Y
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
        pistonY -= moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
        pistonY += moveSpeed;
    pistonY = std::clamp(pistonY, 0.1f, 0.8f);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;

    lastX = (float)xpos;
    lastY = (float)ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;

    if (fov < 1.0f) fov = 1.0f;
    if (fov > 45.0f) fov = 45.0f;
}