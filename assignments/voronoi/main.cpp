#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <shader.h>

#include <iostream>
#include <vector>
#include <math.h>

// structure to hold the info necessary to render an object
struct SceneObject {
    unsigned int VAO;           // vertex array object handle
    unsigned int vertexCount;   // number of vertices in the object
    float r, g, b;              // for object color
    float x, y;                 // for position offset
};

// declaration of the function you will implement in voronoi 1.1
SceneObject instantiateCone(float r, float g, float b, float offsetX, float offsetY);
// mouse, keyboard and screen reshape glfw callbacks
void button_input_callback(GLFWwindow* window, int button, int action, int mods);
void key_input_callback(GLFWwindow* window, int button, int other,int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// settings
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

// global variables we will use to store our objects, shaders, and active shader
std::vector<SceneObject> sceneObjects;
std::vector<Shader> shaderPrograms;
Shader* activeShader;


int main()
{
    srand(time(0)); //Init random number generation
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment - Voronoi Diagram", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // setup frame buffer size callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // setup input callbacks
    glfwSetMouseButtonCallback(window, button_input_callback); // NEW!
    glfwSetKeyCallback(window, key_input_callback); // NEW!

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // NEW!
    // build and compile the shader programs
    shaderPrograms.push_back(Shader("shaders/shader.vert", "shaders/color.frag"));
    shaderPrograms.push_back(Shader("shaders/shader.vert", "shaders/distance.frag"));
    shaderPrograms.push_back(Shader("shaders/shader.vert", "shaders/distance_color.frag"));
    activeShader = &shaderPrograms[0];

    // NEW!
    // set up the z-buffer
    glDepthRange(1,-1); // make the NDC a right handed coordinate system, with the camera pointing towards -z
    glEnable(GL_DEPTH_TEST); // turn on z-buffer depth test
    glDepthFunc(GL_LESS); // draws fragments that are closer to the screen in NDC

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // background color
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        // notice that now we are clearing two buffers, the color and the z-buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render the cones
        glUseProgram(activeShader->ID);

        // TODO voronoi 1.3
        // Iterate through the scene objects, for each object:
        // - bind the VAO; set the uniform variables; and draw.
        // CODE HERE
        for (const SceneObject &so : sceneObjects)
        {
            glBindVertexArray(so.VAO);
            activeShader->setVec2("offset", glm::vec2(so.x, so.y));
            activeShader->setVec3("color", glm::vec3(so.r, so.g, so.b));
            glDrawElements(GL_TRIANGLES, so.vertexCount, GL_UNSIGNED_INT, 0);
        }


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}


// creates a cone triangle mesh, uploads it to openGL and returns the VAO associated to the mesh
SceneObject instantiateCone(float r, float g, float b, float offsetX, float offsetY) {
    // TODO voronoi 1.1
    // (exercises 1.7 and 1.8 can help you with implementing this function)

    // Create an instance of a SceneObject,
    SceneObject sceneObject{};

    // you will need to store offsetX, offsetY, r, g and b in the object.
    // CODE HERE
    sceneObject.x = offsetX;
    sceneObject.y = offsetY;
    sceneObject.r = r;
    sceneObject.g = g;
    sceneObject.b = b;
    // Build the geometry into an std::vector<float> or float array.
    // CODE HERE
    std::vector<float> data{ 0.0f, 0.0f, 1.0f };
    float segments = 30.0f;
    float step = (360.0f / segments);
    float radius = 3.0f;
    float pi = 3.14159f;
    for (int a = 0; a < 360; a += step)
    {
        data.push_back(radius * cos(a * pi / 180));
        data.push_back(radius * sin(a * pi / 180));
        data.push_back(0.0f);
    }

    std::vector<unsigned int> indices;
    for (int i = 1; i <= segments-1; i += 1)
    {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back((i + 1));
    }

    indices.push_back(0);
    indices.push_back(segments);
    indices.push_back(1);
    // Store the number of vertices in the mesh in the scene object.
    // CODE HERE
    sceneObject.vertexCount = data.size();
    // Declare and generate a VAO and VBO (and an EBO if you decide the work with indices).
    // CODE HERE
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // Bind and set the VAO and VBO (and optionally a EBO) in the correct order.
    // CODE HERE
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), &data[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    // Set the position attribute pointers in the shader.
    // CODE HERE
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    // Store the VAO handle in the scene object.
    // CODE HERE
    sceneObject.VAO = VAO;

    // 'return' the scene object for the cone instance you just created.
    return sceneObject;
}

// glfw: called whenever a mouse button is pressed
void button_input_callback(GLFWwindow* window, int button, int action, int mods){
    // TODO voronoi 1.2
    // (exercises 1.9 and 2.2 can help you with implementing this function)

    // Test button press, see documentation at:
    //     https://www.glfw.org/docs/latest/input_guide.html#input_mouse_button
    // CODE HERE
    // If a left mouse button press was detected, call instantiateCone:
    // - Push the return value to the back of the global 'vector<SceneObject> sceneObjects'.
    // - The click position should be transformed from screen coordinates to normalized device coordinates,
    //   to obtain the offset values that describe the position of the object in the screen plane.
    // - A random value in the range [0, 1] should be used for the r, g and b variables.
    // CODE HERE
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        float max_rand = (float)(RAND_MAX);
        double x, y;
        int screenX, screenY;
        glfwGetCursorPos(window, &x, &y);
        glfwGetWindowSize(window, &screenX, &screenY);
        float ndcX = (float)x / (float)screenX * 2.0f - 1.0f;
        float ndcY = ((float)y / (float)screenY * 2.0f - 1.0f) * -1.0f;
        SceneObject so = instantiateCone(rand() / max_rand, rand() / max_rand, rand() / max_rand, ndcX, ndcY);
        sceneObjects.push_back(so);
    }
}

// glfw: called whenever a keyboard key is pressed
void key_input_callback(GLFWwindow* window, int button, int other,int action, int mods){
    // TODO voronoi 1.4

    // Set the activeShader variable by detecting when the keys 1, 2 and 3 were pressed;
    // see documentation at https://www.glfw.org/docs/latest/input_guide.html#input_keyboard
    // Key 1 sets the activeShader to &shaderPrograms[0];
    //   and so on.
    // CODE HERE
    if (action != GLFW_PRESS)
        return;
    switch (button)
    {
	case GLFW_KEY_KP_1:
        activeShader = &shaderPrograms[0];
        break;
    case GLFW_KEY_KP_2:
        activeShader = &shaderPrograms[1];
        break;
    case GLFW_KEY_KP_3:
        activeShader = &shaderPrograms[2];
        break;
    default:
        break;
    }
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}