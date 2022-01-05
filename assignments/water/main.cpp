#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <setup.h>
#include <limits>

#include <vector>
#include <chrono>

#include "shader.h"
#include "glmutils.h"

#include "plane_model.h"
#include "primitives.h"
#include <PerlinNoise.hpp>

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

struct Plane {
    glm::vec3 point;
    glm::vec3 N;
};

// function declarations
// ---------------------
void setupObjects();
void setupTest();
void drawObjects(glm::mat4 viewProjection);
void drawTest(unsigned int VBO, unsigned int VAO);
std::vector<float>* generateHeightMapTexture();
void drawCube(glm::mat4 model);
void drawPlane(glm::mat4 model);
glm::mat4 getViewProjectionMatrix();
glm::mat4 getPViewMatrix();
bool changeProjector();
float getRandomFloat(float min, float max);
glm::mat4 getRangeConversionMatrix(glm::mat4 viewProjectionMatrix, glm::mat4 pViewMatrix);
bool linePlaneIntersection(glm::vec3 a, glm::vec3 b, Plane plane, glm::vec3* contactPoint);
bool rayPlaneIntersection(glm::vec3 a, glm::vec3 b, Plane plane, glm::vec3* contactPoint);
bool linePlaneIntersection2(glm::vec3 a, glm::vec3 ray, Plane plane, glm::vec3* contactPoint);
bool changeProjector();
void setupWater();
glm::vec3 projectPointOntoPlane(glm::vec3 p, Plane plane);
unsigned int getGrid(glm::mat4 rangePViewMatrix);

// global variables used for rendering
// -----------------------------------
SceneObject cube;
SceneObject floorObj;
SceneObject planeBody;
SceneObject planeWing;
SceneObject planePropeller;
unsigned int testVAO;

Shader* objectShaderProgram;
Shader* testShader;
Shader* toBufferShader;

// global variables used for control
// ---------------------------------
float currentTime;
glm::mat4 prevViewProj;

//Water placement and displacement
Plane waterBase;
Plane waterUpperBound;
Plane waterLowerBound;

float counter = 0;
bool counterDir = true;
bool shouldBreak = false;
int gridSize = 100;


glm::mat4 rangeMat;
glm::mat4 invPView;

int main()
{
    //std::cout << "We got: " << v << std::endl;
    //return 0;
    initGLFW();
    GLFWwindow* window = createGLFWWindow();
    if (window == NULL)
        return -1;
    if (loadOpenGLFunctionPointers() == -1)
        return -1;
    configureOpenGL();

    // setup mesh objects and particles
    // ---------------------------------------
    //setupObjects();
    /*setupTest();
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    // bind vertex array object
    glBindVertexArray(VAO);
    unsigned int VBO;
    glGenBuffers(1, &VBO);*/
    generateHeightMapTexture();
    Shader* fromBuffer = new Shader("shaders/FromBuffer.vert", "shaders/object.frag");
    Shader* HeightMapProgram = new Shader("shaders/HeightMap.vert", "shaders/HeightMap.frag");
    Shader * WaterTest = new Shader("shaders/WaterTest.vert", "shaders/WaterTest.frag");
    // render loop
    // -----------
    // render every loopInterval seconds
    float loopInterval = 0.02f;
    auto begin = std::chrono::high_resolution_clock::now();
    setupWater();
    setupObjects();
    //setupTest();
    while (!glfwWindowShouldClose(window))
    {
        // update current time
        auto frameStart = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> appTime = frameStart - begin;
        currentTime = appTime.count();

        processInput(window);
        //if (!changeProjector())
        //    continue;

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

        // notice that we also need to clear the depth buffer (aka z-buffer) every new frame
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        WaterTest->setFloat("heightMapLayer", counter);
        counter += counterDir ? 0.01f : -0.01f;
        if (counter >= 0.9 && counterDir)
        {
            counterDir = false;
        }
        if (counter <= 0.1 && !counterDir)
        {
            counterDir = true;
        }

        glm::mat4 viewProjection = getViewProjectionMatrix();
        drawObjects(viewProjection);
        glUseProgram(WaterTest->ID);
        changeProjector();
        WaterTest->setMat4("viewProj", viewProjection);
        glm::mat4 pView = getPViewMatrix();
        rangeMat = getRangeConversionMatrix(viewProjection, pView);
        if (rangeMat != glm::mat4(1))
        {
            invPView = glm::inverse(pView);

            glm::mat4 finalMatrix = invPView * rangeMat;
            unsigned int VAO = getGrid(finalMatrix);
            glBindVertexArray(VAO);
            //glDrawElements(GL_TRIANGLES, 57000, GL_UNSIGNED_INT, 0); //TODO calculate 57000
            glDrawArrays(GL_POINTS, 0, gridSize*gridSize);
        }
        //drawObjects(viewProjection, VBO, VAO);

        /*glBindVertexArray(testVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);*/
        /*drawTest(VBO, VAO);
        glUseProgram(fromBuffer->ID);
        glBindVertexArray(VAO);
        int posAttributeLocation = glGetAttribLocation(fromBuffer->ID, "pos");
        glEnableVertexAttribArray(posAttributeLocation);
        glVertexAttribPointer(posAttributeLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

        //glDrawArrays(GL_TRIANGLES, 0, 3);*/
        glfwSwapBuffers(window);
        glfwPollEvents();

        // control render loop frequency
        std::chrono::duration<float> elapsed = std::chrono::high_resolution_clock::now()-frameStart;
        while (loopInterval > elapsed.count()) {
            elapsed = std::chrono::high_resolution_clock::now() - frameStart;
        }
    }

    delete objectShaderProgram;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

std::vector<float>* generateHeightMapTexture()
{
    int gridSize = 100;
    const siv::PerlinNoise::seed_type seed = 123456u;

    const siv::PerlinNoise perlin{ seed };
    float stepSize = 1.0f / (float)gridSize;
    std::vector<float>* heightMap = new std::vector<float>(gridSize * gridSize * gridSize);
    for (int i = 0; i < gridSize; i++)
    {
        for (int j = 0; j < gridSize; j++)
        {
            for (int k = 0; k < gridSize; k++)
            {
            
            float value = perlin.normalizedOctave3D_01(i * stepSize, j * stepSize, k * stepSize, 8);
            (*heightMap)[k + j * gridSize + i * gridSize * gridSize] = value;
            
            }
        }
    }

    unsigned int heightMapTex;
    glGenTextures(1, &heightMapTex);
    glBindTexture(GL_TEXTURE_3D, heightMapTex);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, gridSize, gridSize, gridSize, 0, GL_RED, GL_FLOAT, &((*heightMap)[0]));
    std::cout << "Yeah" << std::endl;
    return heightMap;
}

void setupTest()
{
    std::cout << "Hello World Larl" << std::endl;
    toBufferShader = new Shader("shaders/ToBuffer.vert");
    testShader = new Shader("shaders/test.vert", "shaders/object.frag");
    std::vector<float> vertices = 
    { 
        -1, 1, 0,
        1, 1, 0,
        -1, -1, 0,
        -1, -1, 0,
        1, 1, 0,
        1, -1, 0
    };
    testVAO = createVertexArray(vertices);
}

glm::vec3 tranformCornerOfGrid(glm::vec2 corner, glm::mat4 rangePViewMatrix)
{
    glm::vec4 lars = glm::vec4(corner, -1.0f, 1.0f);
    glm::vec4 henning = glm::vec4(corner, 1.0f, 1.0f);
    glm::vec4 larsss = rangeMat * lars;
    glm::vec4 henningggg = rangeMat * henning;
    glm::vec4 negY = invPView * larsss;
    glm::vec4 posY = invPView * henningggg;
    //glm::vec4 negY = rangePViewMatrix * 
    //glm::vec4 posY = rangePViewMatrix * 
    negY /= negY.w;
    posY /= posY.w;

    glm::vec3 contactPoint;
    //Plane waterBaseProjSpace;
    //glm::vec4 pointProjSpace = rangePViewMatrix * glm::vec4(waterBase.point, 1);
    //waterBaseProjSpace.point = pointProjSpace / pointProjSpace.w;
    //waterBaseProjSpace.N = glm::normalize(rangePViewMatrix * glm::vec4(waterBase.N, 0));
    if(rayPlaneIntersection(negY, posY, waterBase, &contactPoint))
        return contactPoint;
    else
    {
        std::cout << "Something went wrong with corner of grid" << std::endl;
        return { 0.0f, 0.0f, 0.0f };
    }
}

glm::vec3 index2D(int i, int j, std::vector<glm::vec3>* grid, int gridSize)
{
    return (*grid)[i * gridSize + j];
}

void index2D(int i, int j, glm::vec3 v, std::vector<glm::vec3>* grid, int gridSize)
{
    int index = i * gridSize + j;
    (*grid)[index] = v;
}

unsigned int getGrid(glm::mat4 rangePViewMatrix)
{
    std::vector<glm::vec3>* grid = new std::vector<glm::vec3>(gridSize*gridSize);
    float stepSize = 1.0f / (float)gridSize;
    
    shouldBreak = true;
    glm::vec3 ll = tranformCornerOfGrid({0, 0}, rangePViewMatrix); //ll
    glm::vec3 ul = tranformCornerOfGrid({0, 1}, rangePViewMatrix); //ul
    glm::vec3 lr = tranformCornerOfGrid({1, 0}, rangePViewMatrix); //lr
    glm::vec3 ur = tranformCornerOfGrid({1, 1}, rangePViewMatrix); //ur
    shouldBreak = true;

    index2D(0, 0, ll, grid, gridSize);
    index2D(0, gridSize-1, ul, grid, gridSize);
    index2D(gridSize-1, 0, lr, grid, gridSize);
    index2D(gridSize - 1, gridSize - 1, ur, grid, gridSize);

    for (int j = 1; j < gridSize - 1; j++)
    {
        glm::vec3 lv = glm::mix(ll, ul, stepSize * j);
        glm::vec3 rv = glm::mix(lr, ur, stepSize * j);
        index2D(0, j, lv, grid, gridSize);
        index2D(gridSize - 1, j, rv, grid, gridSize);
    }

    for (int j = 0; j < gridSize; j++)
    {
        for (int i = 1; i < gridSize - 1; i++)
        {
            glm::vec3 lv = index2D(0, j, grid, gridSize);
            glm::vec3 rv = index2D(gridSize-1, j, grid, gridSize);
            glm::vec3 v = glm::mix(lv, rv, stepSize * i);
            index2D(i, j, v, grid, gridSize);
        }
    }

    std::vector<float>* vertices = new std::vector<float>(gridSize * gridSize * 3);

    for (int i = 0; i < grid->size(); i++)
    {
        (*vertices)[i * 3 + 0] = (*grid)[i].x;
        (*vertices)[i * 3 + 1] = (*grid)[i].y;
        (*vertices)[i * 3 + 2] = (*grid)[i].z;
    }

    delete grid;
    std::vector<unsigned int>* indices = new std::vector<unsigned int>();

    for (int i = 0; i < gridSize - 2; i++)
    {
        for (int j = 0; j < gridSize - 2; j++)
        {
            //Right triangle
            indices->push_back(j + i * gridSize);
            indices->push_back((j + 1) + (i + 1) * gridSize);
            indices->push_back(j + (i + 1) * gridSize);

            //Left triangle
            indices->push_back(j + i * gridSize);
            indices->push_back((j + 1) + i * gridSize);
            indices->push_back((j + 1) + (i + 1) * gridSize); //TODO: Test this bullshit!!
        }
    }

    unsigned int VAO = createVertexArray(*vertices, *indices);

    delete vertices, indices;

    return VAO;
}

void drawTest(unsigned int VBO, unsigned int VAO)
{
    glUseProgram(toBufferShader->ID);
    glBindVertexArray(VAO);
    glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, VBO, 0, 9 * sizeof(GLfloat));
    glBeginTransformFeedback(GL_TRIANGLES);
    glBindVertexArray(testVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glEndTransformFeedback();
}

glm::mat4 getViewProjectionMatrix()
{
    glm::mat4 projection = glm::perspectiveFov(70.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, .01f, 100.0f);
    glm::mat4 view = glm::lookAt(camPosition, camPosition + camForward, glm::vec3(0, 1, 0));
    return projection * view;
}

glm::mat4 getPViewMatrix()
{
    glm::mat4 projection = glm::perspectiveFov(70.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, .01f, 100.0f);
    glm::mat4 view = glm::lookAt(projPosition, projLookAtPoint, glm::vec3(0, 1, 0));
    return projection * view;
}

bool changeProjector()
{
    if (camPosition.y >= waterUpperBound.point.y)
        projPosition = { camPosition.x, camPosition.y, camPosition.z };
    else
        projPosition = { camPosition.x, waterUpperBound.point.y + 1.0f, camPosition.z };

    glm::vec3 cf = camForward;
    if ((camPosition.y > waterBase.point.y && camForward.y > 0) || camPosition.y < waterBase.point.y && camForward.y < 0)
        cf = { cf.x, -cf.y, cf.z };


    glm::vec3 method1Point(0);
    glm::vec3 contactPoint;
    if (rayPlaneIntersection(camPosition, camPosition + cf, waterBase, &contactPoint))
    {
        method1Point = contactPoint;
    }
    else 
        std::cout << "Change projector method 1 not working" << std::endl;

    float fixedLength = 5.0f;
    glm::vec3 point = camPosition + (cf * fixedLength);
    point = projectPointOntoPlane(point, waterBase);
    glm::vec3 method2Point = point;

    float angle = glm::degrees(glm::acos(glm::abs(glm::dot(waterBase.N, camForward))));
    float mixVal = angle / 90.0f;
    //std::cout << "Angle: " << angle << "Mix: " << mixVal << std::endl;
    projLookAtPoint = glm::mix(method1Point, method2Point, mixVal);
    //std::cout << "m1: " << method1Point.y << ", m2: " << method2Point.y << std::endl;
    std::cout << projLookAtPoint << std::endl;
    return true;
}

void setupWater()
{
    glm::vec3 up = { 0, 1, 0 };
    waterBase.point = { 0, -10, 0 };
    waterBase.N = up;
    waterUpperBound.point = { 0, 0, 0 };
    waterUpperBound.N = up;
    waterLowerBound.point = { 0, -20, 0 };
    waterLowerBound.N = up;
}

glm::vec3 projectPointOntoPlane(glm::vec3 p, Plane plane)
{
    return p - (glm::dot(p - plane.point, plane.N) * plane.N);
}

template<typename T>
bool pointOnLine(T a, T b, T c)
{
    float dist1 = glm::distance(a, c) + glm::distance(c, b);
    float dist2 = glm::distance(a, b);
    float shouldBeZero = glm::abs(dist1 - dist2);
    return shouldBeZero < 0.005f;
}

bool linePlaneIntersection(glm::vec3 a, glm::vec3 b, Plane plane, glm::vec3* contactPoint)
{
    glm::vec3 ray = a - b;
    glm::vec3 diff = a - plane.point;
    float x = glm::dot(diff, plane.N);
    float y = glm::dot(ray, plane.N);
    if (y == 0)
        return false;
    float z = x / y;
    *contactPoint = a - ray * z;
    return pointOnLine(a, b, *contactPoint);
}

bool rayPlaneIntersection(glm::vec3 a, glm::vec3 b, Plane plane, glm::vec3* contactPoint)
{
    glm::vec3 ray = a - b;
    glm::vec3 diff = a - plane.point;
    float x = glm::dot(diff, plane.N);
    float y = glm::dot(ray, plane.N);
    if (y == 0)
        return false;
    float z = x / y;
    *contactPoint = a - ray * z;
    return true;
}

bool isVecEqual(glm::vec3 a, glm::vec3 b)
{
    return glm::distance(a, b) <= 0.005f;
}

bool linePlaneIntersection2(glm::vec3 a, glm::vec3 ray, Plane plane, glm::vec3* contactPoint)
{
    glm::vec3 diff = a - plane.point;
    float x = glm::dot(diff, plane.N);
    float y = glm::dot(ray, plane.N);
    if (y == 0)
        return false;
    float z = x / y;
    *contactPoint = a - ray * z;
    return isVecEqual(glm::distance(a, *contactPoint) * ray + a, *contactPoint);
}

glm::mat4 getRangeConversionMatrix(glm::mat4 viewProjectionMatrix, glm::mat4 pViewMatrix)
{
    glm::mat4 invViewProjectionMatrix = glm::inverse(viewProjectionMatrix);

    std::vector<glm::vec4> frustrumCorners =
    {
        {-1.0f,  1.0f, -1.0f,  1.0f}, //ULB
        { 1.0f,  1.0f, -1.0f,  1.0f}, //URB
        {-1.0f, -1.0f, -1.0f,  1.0f}, //LLB
        { 1.0f, -1.0f, -1.0f,  1.0f}, //LRB
        {-1.0f,  1.0f,  1.0f,  1.0f}, //ULF
        { 1.0f,  1.0f,  1.0f,  1.0f}, //URF
        {-1.0f, -1.0f,  1.0f,  1.0f}, //LLF
        { 1.0f, -1.0f,  1.0f,  1.0f}  //LRF
    };

    //Get corners in world space
    for (int i = 0; i < frustrumCorners.size(); i++)
    {
        frustrumCorners[i] = invViewProjectionMatrix * frustrumCorners[i];
        frustrumCorners[i] /= frustrumCorners[i].w;
    }

    std::vector<glm::vec3> intersections = std::vector<glm::vec3>();

    glm::vec3 contactPoint;

    if (linePlaneIntersection(frustrumCorners[0], frustrumCorners[1], waterLowerBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[0], frustrumCorners[1], waterUpperBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[0], frustrumCorners[2], waterLowerBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[0], frustrumCorners[2], waterUpperBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[0], frustrumCorners[4], waterLowerBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[0], frustrumCorners[4], waterUpperBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[3], frustrumCorners[1], waterLowerBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[3], frustrumCorners[1], waterUpperBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[3], frustrumCorners[2], waterLowerBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[3], frustrumCorners[2], waterUpperBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[3], frustrumCorners[7], waterLowerBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[3], frustrumCorners[7], waterUpperBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[5], frustrumCorners[1], waterLowerBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[5], frustrumCorners[1], waterUpperBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[5], frustrumCorners[4], waterLowerBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[5], frustrumCorners[4], waterUpperBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[5], frustrumCorners[7], waterLowerBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[5], frustrumCorners[7], waterUpperBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[6], frustrumCorners[4], waterLowerBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[6], frustrumCorners[4], waterUpperBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[6], frustrumCorners[2], waterLowerBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[6], frustrumCorners[2], waterUpperBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[6], frustrumCorners[7], waterLowerBound, &contactPoint))
        intersections.push_back(contactPoint);
    if (linePlaneIntersection(frustrumCorners[6], frustrumCorners[7], waterUpperBound, &contactPoint))
        intersections.push_back(contactPoint);

    //Add corners between planes
    for(glm::vec3 corner : frustrumCorners)
    {
        if (pointOnLine(waterUpperBound.point.y, waterLowerBound.point.y, corner.y))
            intersections.push_back(corner);
    }

    if (intersections.empty())
    {
        std::cout << "No need to render water, since not overlap between water and camera" << std::endl;
        return glm::mat4(1);
    }
    
    float minX = std::numeric_limits<float>::max();
    float minY = minX;
    float maxX = std::numeric_limits<float>::min();
    float maxY = maxX;

    //Project to waterBase, transform to projector space and get min/max x/y.
    for (int i = 0; i < intersections.size(); i++)
    {
        glm::vec4 pIntersection = pViewMatrix * glm::vec4(projectPointOntoPlane(intersections[i], waterBase), 1);
        intersections[i] = pIntersection / pIntersection.w;
        if (intersections[i].x < minX)
            minX = intersections[i].x;
        if (intersections[i].x > maxX)
            maxX = intersections[i].x;
        if (intersections[i].y < minY)
            minY = intersections[i].y;
        if (intersections[i].y > maxY)
            maxY = intersections[i].y;
    }

    glm::mat4 rangeMatrix = {
        {maxX - minX, 0,          0, minX},
        {0,           maxY- minY, 0, minY},
        {0,           0,          1, 0   },
        {0,           0,          0, 1   }
    };

    return glm::transpose(rangeMatrix);
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


void setupObjects() {
    // initialize shaders
    objectShaderProgram = new Shader("shaders/object.vert", "shaders/object.frag");

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

float getRandomFloat(float min, float max)
{
    static bool randomSeedSet = false;
    assert(max >= min);

    if (!randomSeedSet)
    {
        srand(time(NULL));
        randomSeedSet = true;
    }

    int diff = max - min;

    return ((float)rand()) / ((float)RAND_MAX) * diff + min;
}

void drawObjects(glm::mat4 viewProjection) {
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