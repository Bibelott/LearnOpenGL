#include <iostream>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iterator>
#include <shader.hpp>
#include <camera.hpp>
#include <model.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const unsigned int SHADOW_WIDHT = 1024, SHADOW_HEIGHT = 1024;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 400;
float lastY = 300;
bool firstMouse = true;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int loadTexture(const char *path);

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 16);

  GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);

  if (window == NULL) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD\n";
    return -1;
  }

  glViewport(0, 0, 800, 600);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_callback);

  glfwSetScrollCallback(window, scroll_callback);

  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_FRAMEBUFFER_SRGB);

  Model cube("resources/cube/cube.obj");
  Model quad("resources/quad/quad.obj");

  Shader shader("src/cubeShader.vert", "src/cubeShader.frag");
  Shader depthShader("src/depthShader.vert", "src/depthShader.frag");

  shader.use();
  shader.setVec3("light.direction", 2.0f, -4.0f, 1.0f);
  shader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
  shader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
  shader.setVec3("light.specular", 0.8f, 0.8f, 0.8f);
  shader.setInt("shadowMap", 3);

  unsigned int depthMapFBO;
  glGenFramebuffers(1, &depthMapFBO);

  unsigned int depthMap;
  glGenTextures(1, &depthMap);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDHT, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // render loop
  while(!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;

    processInput(window);

    glm::mat4 lightSpaceMatrix;
    
    {
      glViewport(0, 0, SHADOW_WIDHT, SHADOW_HEIGHT);
      glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
      glClear(GL_DEPTH_BUFFER_BIT);

      glm::mat4 view = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f),
                                   glm::vec3( 0.0f, 0.0f,  0.0f),
                                   glm::vec3( 0.0f, 1.0f,  0.0f));

      glm::mat4 projection;
      projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.5f, 20.0f);

      lightSpaceMatrix = projection * view;

      glm::mat4 model = glm::mat4(1.0f);

      depthShader.use();
      depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
      depthShader.setMat4("model", model);

      cube.Draw(depthShader);

      model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
      model = glm::scale(model, glm::vec3(10.0f, 1.0f, 10.0f));
      depthShader.setMat4("model", model);

      quad.Draw(depthShader);

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    {
      glViewport(0, 0, 800, 600);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glm::mat4 view = camera.GetViewMatrix();

      glm::mat4 projection;
      projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 300.0f);

      glm::mat4 model = glm::mat4(1.0f);

      shader.use();
      shader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
      shader.setFloat("material.shininess", 32.0f);

      shader.setMat4("view", view);
      shader.setMat4("projection", projection);
      shader.setMat4("model", model);
      shader.setVec3("viewPos", camera.Position);
      shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, depthMap);

      cube.Draw(shader);

      shader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
      shader.setFloat("material.shininess", 4.0f);
      model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
      model = glm::scale(model, glm::vec3(10.0f, 1.0f, 10.0f));
      shader.setMat4("model", model);

      quad.Draw(shader);
    }

    // end loop
    glfwSwapBuffers(window);
    glfwPollEvents();
    lastFrame = currentFrame;
  }

  glfwTerminate();
  return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.ProcessKeyboard(FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.ProcessKeyboard(LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.ProcessKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }
  
  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;
  lastX = xpos;
  lastY = ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
  camera.ProcessMouseScroll(yoffset);
}

unsigned int loadTexture(char const * path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = 0;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
