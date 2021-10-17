#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <openGLSetup.h>

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
void setupObjects();
void setupParticles();
void drawParticles(glm::mat4 viewProjection, Shader* shaderProgram, GLenum mode);
void drawObjects(glm::mat4 viewProjection);
void drawCube(glm::mat4 model);
void drawPlane(glm::mat4 model);
glm::mat4 getViewProjectionMatrix();
float getRandomFloat(int min, int max);

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
    initGLFW();
    GLFWwindow* window = createGLFWWindow();
    if (window == NULL)
        return -1;
    if (loadOpenGLFunctionPointers() == -1)
        return -1;
    configureOpenGL();

    // setup mesh objects and particles
    // ---------------------------------------
    setupObjects();
    setupParticles();

    prevViewProj = getViewProjectionMatrix();


    // render loop
    // -----------
    // render every loopInterval seconds
    float loopInterval = 0.02f;
    auto begin = std::chrono::high_resolution_clock::now();

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
        drawParticles(viewProjection, rainShaderProgram, GL_LINES);
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