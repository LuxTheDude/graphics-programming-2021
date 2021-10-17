#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <vector>
#include <chrono>

#include "shader.h"
#include "glmutils.h"

#include "plane_model.h"
#include "primitives.h"

// structures to hold render info
// -----------------------------
struct SceneObject{
    unsigned int VAO;
    unsigned int vertexCount;
    void drawSceneObject() const{
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES,  vertexCount, GL_UNSIGNED_INT, 0);
    }
};

struct Particles {
    unsigned int VAO;
    unsigned int vertexCount;
    void drawParticles(GLenum mode) const {
        glEnable(GL_BLEND);
        glBindVertexArray(VAO);
        glDrawArrays(mode, 0, vertexCount);
        glDisable(GL_BLEND);
    }
};

// function declarations
// ---------------------
unsigned int createArrayBuffer(const std::vector<float> &array);
unsigned int createElementArrayBuffer(const std::vector<unsigned int> &array);
unsigned int createVertexArray(const std::vector<float> &positions, const std::vector<float> &colors, const std::vector<unsigned int> &indices, Shader* shaderProgram);
unsigned int createVertexArray(const std::vector<float>& positions);
void setupObjects();
void setupParticles();
void drawParticles(glm::mat4 viewProjection, Shader* shaderProgram, GLenum mode);
void drawObjects(glm::mat4 viewProjection);
glm::mat4 getViewProjectionMatrix();
float getRandomFloat(int min, int max);

// glfw and input functions
// ------------------------
void cursorInRange(float screenX, float screenY, int screenW, int screenH, float min, float max, float &x, float &y);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void cursor_input_callback(GLFWwindow* window, double posX, double posY);
void drawCube(glm::mat4 model);
void drawPlane(glm::mat4 model);

// screen settings
// ---------------
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

// global variables used for rendering
// -----------------------------------
SceneObject cube;
SceneObject floorObj;
SceneObject planeBody;
SceneObject planeWing;
SceneObject planePropeller;

Particles particles;

Shader* objectShaderProgram;
Shader* snowShaderProgram;
Shader* rainShaderProgram;

// global variables used for control
// ---------------------------------
float currentTime;
glm::vec3 camForward(.0f, .0f, -1.0f);
glm::vec3 camPosition(.0f, 1.6f, 0.0f);
float linearSpeed = 0.15f, rotationGain = 30.0f;
float mousePrevX = 0.f, mousePrevY = 0.f;
glm::mat4 prevViewProj;

// global particles variables used for control
// -------------------------------------------
const unsigned int numSimulations = 10;
const unsigned int numParticles = 10000;
const unsigned int cubeSize = 30;
float gravityOffsets[numSimulations];
glm::vec3 windOffsets[numSimulations];
glm::vec3 randomOffsets[numSimulations];
const float gravity = -0.3f;
const glm::vec3 wind(0.1f, 0.0f, 0.01f);

int main()
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

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Exercise 5.2", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_input_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // setup mesh objects and particles
    // ---------------------------------------
    setupObjects();
    setupParticles();

    // set up the z-buffer
    // Notice that the depth range is now set to glDepthRange(-1,1), that is, a left handed coordinate system.
    // That is because the default openGL's NDC is in a left handed coordinate system (even though the default
    // glm and legacy openGL camera implementations expect the world to be in a right handed coordinate system);
    // so let's conform to that
    glDepthRange(-1,1); // make the NDC a LEFT handed coordinate system, with the camera pointing towards +z
    glEnable(GL_DEPTH_TEST); // turn on z-buffer depth test
    glDepthFunc(GL_LESS); // draws fragments that are closer to the screen in NDC
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);


    // render loop
    // -----------
    // render every loopInterval seconds
    float loopInterval = 0.02f;
    auto begin = std::chrono::high_resolution_clock::now();
    prevViewProj = getViewProjectionMatrix();

    while (!glfwWindowShouldClose(window))
    {
        // update current time
        auto frameStart = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> appTime = frameStart - begin;
        currentTime = appTime.count();

        processInput(window);

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

        // notice that we also need to clear the depth buffer (aka z-buffer) every new frame
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 viewProjection = getViewProjectionMatrix();
        drawObjects(viewProjection);
        drawParticles(viewProjection, rainShaderProgram, GL_LINE);
        prevViewProj = viewProjection;


        glfwSwapBuffers(window);
        glfwPollEvents();

        // control render loop frequency
        std::chrono::duration<float> elapsed = std::chrono::high_resolution_clock::now()-frameStart;
        while (loopInterval > elapsed.count()) {
            elapsed = std::chrono::high_resolution_clock::now() - frameStart;
        }
    }

    delete objectShaderProgram;
    delete snowShaderProgram;
    delete rainShaderProgram;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

glm::mat4 getViewProjectionMatrix()
{
    glm::mat4 projection = glm::perspectiveFov(70.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, .01f, 100.0f);
    glm::mat4 view = glm::lookAt(camPosition, camPosition + camForward, glm::vec3(0, 1, 0));
    return projection * view;
}

void drawParticles(glm::mat4 viewProjection, Shader* shaderProgram, GLenum mode) {
    shaderProgram->use();
    shaderProgram->setMat4("model", viewProjection);
    shaderProgram->setMat4("prevModel", prevViewProj);
    shaderProgram->setVec3("camPos", camPosition);
    shaderProgram->setVec3("camForward", camForward);
    shaderProgram->setVec3("velocity", glm::vec3(0, gravity, 0) + wind);
    shaderProgram->setFloat("cubeSize", cubeSize);

    for (unsigned int i = 0; i < numSimulations; i++)
    {
        gravityOffsets[i] += gravity;
        windOffsets[i] += wind;
        glm::vec3 offset = glm::vec3(0, gravityOffsets[i], 0) + windOffsets[i] + randomOffsets[i];
        offset -= camPosition + camForward + (((float) cubeSize) / 2.f);
        offset = glm::mod(offset, ((float) cubeSize));
        shaderProgram->setVec3("offset", offset);
        particles.drawParticles(mode);
    }
}


void drawObjects(glm::mat4 viewProjection){
    objectShaderProgram->use();
    glm::mat4 scale = glm::scale(1.f, 1.f, 1.f);

    // draw floor (the floor was built so that it does not need to be transformed)
    objectShaderProgram->setMat4("model", viewProjection);
    floorObj.drawSceneObject();

    // draw 2 cubes and 2 planes in different locations and with different orientations
    drawCube(viewProjection * glm::translate(2.0f, 1.f, 2.0f) * glm::rotateY(glm::half_pi<float>()) * scale);
    drawCube(viewProjection * glm::translate(-2.0f, 1.f, -2.0f) * glm::rotateY(glm::quarter_pi<float>()) * scale);

    drawPlane(viewProjection * glm::translate(-2.0f, .5f, 2.0f) * glm::rotateX(glm::quarter_pi<float>()) * scale);
    drawPlane(viewProjection * glm::translate(2.0f, .5f, -2.0f) * glm::rotateX(glm::quarter_pi<float>() * 3.f) * scale);
}


void drawCube(glm::mat4 model){
    // draw object
    objectShaderProgram->setMat4("model", model);
    cube.drawSceneObject();
}


void drawPlane(glm::mat4 model){

    // draw plane body and right wing
    objectShaderProgram->setMat4("model", model);
    planeBody.drawSceneObject();
    planeWing.drawSceneObject();

    // propeller,
    glm::mat4 propeller = model * glm::translate(.0f, .5f, .0f) *
                          glm::rotate(currentTime * 10.0f, glm::vec3(0.0,1.0,0.0)) *
                          glm::rotate(glm::half_pi<float>(), glm::vec3(1.0,0.0,0.0)) *
                          glm::scale(.5f, .5f, .5f);

    objectShaderProgram->setMat4("model", propeller);
    planePropeller.drawSceneObject();

    // right wing back,
    glm::mat4 wingRightBack = model * glm::translate(0.0f, -0.5f, 0.0f) * glm::scale(.5f,.5f,.5f);
    objectShaderProgram->setMat4("model", wingRightBack);
    planeWing.drawSceneObject();

    // left wing,
    glm::mat4 wingLeft = model * glm::scale(-1.0f, 1.0f, 1.0f);
    objectShaderProgram->setMat4("model", wingLeft);
    planeWing.drawSceneObject();

    // left wing back,
    glm::mat4 wingLeftBack =  model *  glm::translate(0.0f, -0.5f, 0.0f) * glm::scale(-.5f,.5f,.5f);
    objectShaderProgram->setMat4("model", wingLeftBack);
    planeWing.drawSceneObject();
}

void setupParticles() {
    snowShaderProgram = new Shader("shaders/snow.vert", "shaders/snow.frag");
    rainShaderProgram = new Shader("shaders/rain.vert", "shaders/rain.frag");

    std::vector<float> positions(numParticles * 3 * 2);
    for (unsigned int i = 0; i < numParticles*3*2; i += 3*2)
    {
        positions[i] = positions[i+3] = getRandomFloat(cubeSize, cubeSize * 2);
        positions[i+1] = positions[i+4] = getRandomFloat(cubeSize, cubeSize * 2);
        positions[i+2] = positions[i + 5] =  getRandomFloat(cubeSize, cubeSize * 2);
    }
    particles.VAO = createVertexArray(positions);
    particles.vertexCount = positions.size();

    for (unsigned int i = 0; i < numSimulations; i++)
    {
        gravityOffsets[i] = 0;
        windOffsets[i] = glm::vec3(0);
        randomOffsets[i] = glm::vec3(getRandomFloat(0, 5), getRandomFloat(0, 5), getRandomFloat(0, 5));
    }
}



void setupObjects(){
    // initialize shaders
    objectShaderProgram = new Shader("shaders/shader.vert", "shaders/shader.frag");

    // load floor mesh into openGL
    floorObj.VAO = createVertexArray(floorVertices, floorColors, floorIndices, objectShaderProgram);
    floorObj.vertexCount = floorIndices.size();

    // load cube mesh into openGL
    cube.VAO = createVertexArray(cubeVertices, cubeColors, cubeIndices, objectShaderProgram);
    cube.vertexCount = cubeIndices.size();

    // load plane meshes into openGL
    planeBody.VAO = createVertexArray(planeBodyVertices, planeBodyColors, planeBodyIndices, objectShaderProgram);
    planeBody.vertexCount = planeBodyIndices.size();

    planeWing.VAO = createVertexArray(planeWingVertices, planeWingColors, planeWingIndices, objectShaderProgram);
    planeWing.vertexCount = planeWingIndices.size();

    planePropeller.VAO = createVertexArray(planePropellerVertices, planePropellerColors, planePropellerIndices, objectShaderProgram);
    planePropeller.vertexCount = planePropellerIndices.size();
}

unsigned int createVertexArray(const std::vector<float>& positions) {
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    createArrayBuffer(positions);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    return VAO;
}


unsigned int createVertexArray(const std::vector<float> &positions, const std::vector<float> &colors, const std::vector<unsigned int> &indices, Shader* shaderProgram){
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


unsigned int createArrayBuffer(const std::vector<float> &array){
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, array.size() * sizeof(GLfloat), &array[0], GL_STATIC_DRAW);

    return VBO;
}


unsigned int createElementArrayBuffer(const std::vector<unsigned int> &array){
    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, array.size() * sizeof(unsigned int), &array[0], GL_STATIC_DRAW);

    return EBO;
}

// NEW!
// instead of using the NDC to transform from screen space you can now define the range using the
// min and max parameters
void cursorInRange(float screenX, float screenY, int screenW, int screenH, float min, float max, float &x, float &y){
    float sum = max - min;
    float xInRange = (float) screenX / (float) screenW * sum - sum/2.0f;
    float yInRange = (float) screenY / (float) screenH * sum - sum/2.0f;
    x = xInRange;
    y = -yInRange; // flip screen space y axis
}

void cursor_input_callback(GLFWwindow* window, double posX, double posY){
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

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    // TODO move the camera position based on keys pressed (use either WASD or the arrow keys)
    glm::vec3 dir(0.f,0.f,0.f);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        dir += camForward;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        dir += -glm::normalize(glm::cross(camForward, glm::vec3(0.f, 1.f, 0.f)));
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        dir += -camForward;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        dir += glm::normalize(glm::cross(camForward, glm::vec3(0.f, 1.f, 0.f)));
    if(glm::length(dir) != 0)
    camPosition += glm::normalize(dir)*linearSpeed;
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

float getRandomFloat(int min, int max)
{
    static bool randomSeedSet = false;
    assert(max > min);

    if (!randomSeedSet)
    {
        srand(time(NULL));
        randomSeedSet = true;
    }

    int diff = max - min;

    return ((float)rand()) / ((float)RAND_MAX) * diff + min;
}