#pragma once
#include <vector>
#include "shader.h"
#include "glmutils.h"

//OpenGL helpers
unsigned int createArrayBuffer(const std::vector<float>& array);
unsigned int createElementArrayBuffer(const std::vector<unsigned int>& array);
unsigned int createVertexArray(const std::vector<float>& positions, const std::vector<float>& colors, const std::vector<unsigned int>& indices, Shader* shaderProgram);
unsigned int createVertexArray(const std::vector<float>& positions);
unsigned int createVertexArray(const std::vector<float>& positions, const std::vector<unsigned int>& indices);

//GLFW and input functions
void cursorInRange(float screenX, float screenY, int screenW, int screenH, float min, float max, float& x, float& y);
void cursor_input_callback(GLFWwindow* window, double posX, double posY);
void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// screen settings
// ---------------
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

//global variables for control
int precipitationType = 1;
float mousePrevX = 0.f, mousePrevY = 0.f;
glm::vec3 camForward(.0f, .0f, -1.0f);
glm::vec3 camPosition(.0f, 1.6f, 0.0f);
float linearSpeed = 0.15f, rotationGain = 30.0f;

void initGLFW()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
    #endif
}

GLFWwindow* createGLFWWindow()
{
    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Exercise 5.2", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return window;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_input_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    return window;
}

int loadOpenGLFunctionPointers()
{
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
}

void configureOpenGL()
{
    // set up the z-buffer
    // Notice that the depth range is now set to glDepthRange(-1,1), that is, a left handed coordinate system.
    // That is because the default openGL's NDC is in a left handed coordinate system (even though the default
    // glm and legacy openGL camera implementations expect the world to be in a right handed coordinate system);
    // so let's conform to that
    glDepthRange(-1, 1); // make the NDC a LEFT handed coordinate system, with the camera pointing towards +z
    glEnable(GL_DEPTH_TEST); // turn on z-buffer depth test
    glDepthFunc(GL_LESS); // draws fragments that are closer to the screen in NDC
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
}


//OpenGL Helpers
//------------------------------------------------------------------------
unsigned int createVertexArray(const std::vector<float>& positions) {
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    createArrayBuffer(positions);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    return VAO;
}

unsigned int createVertexArray(const std::vector<float>& positions, const std::vector<unsigned int>& indices) {
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    createArrayBuffer(positions);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    createElementArrayBuffer(indices);

    return VAO;
}


unsigned int createVertexArray(const std::vector<float>& positions, const std::vector<float>& colors, const std::vector<unsigned int>& indices, Shader* shaderProgram) {
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    // bind vertex array object
    glBindVertexArray(VAO);

    // set vertex shader attribute "pos"
    createArrayBuffer(positions); // creates and bind  the VBO
    int posAttributeLocation = glGetAttribLocation(shaderProgram->ID, "pos");
    glEnableVertexAttribArray(posAttributeLocation);
    glVertexAttribPointer(posAttributeLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // set vertex shader attribute "color"
    createArrayBuffer(colors); // creates and bind the VBO
    int colorAttributeLocation = glGetAttribLocation(shaderProgram->ID, "color");
    glEnableVertexAttribArray(colorAttributeLocation);
    glVertexAttribPointer(colorAttributeLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // creates and bind the EBO
    createElementArrayBuffer(indices);

    return VAO;
}


unsigned int createArrayBuffer(const std::vector<float>& array) {
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, array.size() * sizeof(GLfloat), &array[0], GL_STATIC_DRAW);

    return VBO;
}


unsigned int createElementArrayBuffer(const std::vector<unsigned int>& array) {
    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, array.size() * sizeof(unsigned int), &array[0], GL_STATIC_DRAW);

    return EBO;
}






//GLFW and input functions
//---------------------------------------------------------------------------------------

// NEW!
// instead of using the NDC to transform from screen space you can now define the range using the
// min and max parameters
void cursorInRange(float screenX, float screenY, int screenW, int screenH, float min, float max, float& x, float& y) {
    float sum = max - min;
    float xInRange = (float)screenX / (float)screenW * sum - sum / 2.0f;
    float yInRange = (float)screenY / (float)screenH * sum - sum / 2.0f;
    x = xInRange;
    y = -yInRange; // flip screen space y axis
}

void cursor_input_callback(GLFWwindow* window, double posX, double posY) {
    // TODO - rotate the camera position based on mouse movements
    //  if you decide to use the lookAt function, make sure that the up vector and the
    //  vector from the camera position to the lookAt target are not collinear
    float x, y;
    cursorInRange(posX, posY, SCR_WIDTH, SCR_HEIGHT, 0, 1, x, y);
    float dx = mousePrevX - x;
    float dy = mousePrevY - y;
    glm::mat4 horizontalRot = glm::rotateY(glm::radians(rotationGain) * dx);
    glm::vec3 newForward = horizontalRot * glm::vec4(camForward, 1.f);
    glm::vec3 verticalRotAxis = glm::cross(newForward, glm::vec3(0.f, 1.f, 0.f));
    glm::mat4 verticalRot = glm::rotate(glm::radians(rotationGain) * -dy, verticalRotAxis);
    newForward = verticalRot * glm::vec4(newForward, 1.f);
    newForward = glm::normalize(newForward);
    float dotProd = glm::dot(newForward, glm::vec3(0.f, 1.f, 0.f));
    if (glm::abs(dotProd) < .99f)
        camForward = newForward;


    mousePrevX = x;
    mousePrevY = y;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        precipitationType = 1;

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        precipitationType = 2;

    // TODO move the camera position based on keys pressed (use either WASD or the arrow keys)
    glm::vec3 dir(0.f, 0.f, 0.f);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        dir += camForward;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        dir += -glm::normalize(glm::cross(camForward, glm::vec3(0.f, 1.f, 0.f)));
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        dir += -camForward;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        dir += glm::normalize(glm::cross(camForward, glm::vec3(0.f, 1.f, 0.f)));
    if (glm::length(dir) != 0)
        camPosition += glm::normalize(dir) * linearSpeed;
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

