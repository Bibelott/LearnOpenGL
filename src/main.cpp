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

float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera camera(glm::vec3(0.0f, 20.0f, 200.0f));
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

  // stbi_set_flip_vertically_on_load(1);

  glEnable(GL_DEPTH_TEST);

  Shader shader("src/shader.vert", "src/shader.frag");
  Shader instanceShader("src/instanceShader.vert", "src/instanceShader.frag");

  Model planet("resources/planet.obj");
  Model rock("resources/rock.obj");

  unsigned int amount = 100000;
  glm::mat4 *modelMatrices;
  modelMatrices = new glm::mat4[amount];
  srand(glfwGetTime());
  float radius = 150.0f;
  float offset = 25.0f;

  for (unsigned int i = 0; i < amount; i++) {
    glm::mat4 model = glm::mat4(1.0f);

    float angle = (float)i / (float)amount * 360.0f;

    float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
    float x = sin(angle) * radius + displacement;

    displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
    float y = displacement * 0.4f;

    displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
    float z = cos(angle) * radius + displacement;

    model = glm::translate(model, glm::vec3(x, y, z));

    float scale = (rand() % 20) / 100.0f + 0.05f;
    model = glm::scale(model, glm::vec3(scale));

    float rotAngle = (rand() % 360);
    model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

    modelMatrices[i] = model;
  }

  unsigned int buffer;
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

  for (unsigned int i = 0; i < rock.meshes.size(); i++) {
    unsigned int VAO = rock.meshes[i].VAO;
    glBindVertexArray(VAO);
    std::size_t v4s = sizeof(glm::vec4);

    glBindVertexBuffer(1, buffer, 0, 4 * v4s);
    glVertexBindingDivisor(1, 1);

    glEnableVertexAttribArray(3);
    glVertexAttribFormat(3, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(3, 1);

    glEnableVertexAttribArray(4);
    glVertexAttribFormat(4, 4, GL_FLOAT, GL_FALSE, v4s);
    glVertexAttribBinding(4, 1);

    glEnableVertexAttribArray(5);
    glVertexAttribFormat(5, 4, GL_FLOAT, GL_FALSE, 2 * v4s);
    glVertexAttribBinding(5, 1);

    glEnableVertexAttribArray(6);
    glVertexAttribFormat(6, 4, GL_FLOAT, GL_FALSE, 3 * v4s);
    glVertexAttribBinding(6, 1);

    glBindVertexArray(0);
  }

  double timer = 0.0;
  unsigned int frameCount = 0;

  instanceShader.use();
  instanceShader.setInt("texture_diffuse1", 0);

  // render loop
  while(!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;

    timer += deltaTime;
    frameCount++;
    if (timer >= 1.0) {
      std::cout << frameCount << std::endl;
      timer = 0.0;
      frameCount = 0;
    }
    
    processInput(window);

    // render
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();

    glm::mat4 view = camera.GetViewMatrix();
    shader.setMat4("view", view);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 300.0f);
    shader.setMat4("projection", projection);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
    model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
    shader.setMat4("model", model);
    planet.Draw(shader);

    instanceShader.use();
    instanceShader.setMat4("view", view);
    instanceShader.setMat4("projection", projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rock.textures_loaded[0].id);
    for (unsigned int i = 0; i < rock.meshes.size(); i++) {
      glBindVertexArray(rock.meshes[i].VAO);
      glDrawElementsInstanced(GL_TRIANGLES, rock.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amount);
      glBindVertexArray(0);
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
