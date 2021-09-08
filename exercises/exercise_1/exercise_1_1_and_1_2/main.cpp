#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

const char* fragmentShaderSourceOrange = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"	FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\0";

const char* fragmentShaderSourceYellow = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"	FragColor = vec4(1.0f, 1.0f, 0f, 1.0f);\n"
"}\0";


unsigned int VAOs[2];
unsigned int VBOs[2];
unsigned int EBO;

/*float vertices[] =
{
	 0.5f,  0.5f, 0.0f, // top right
	 0.5f, -0.5f, 0.0f, // bottom right
	-0.5f, -0.5f, 0.0f, // bottom left
	-0.5f,  0.5f, 0.0f // top left
};*/

float vertices[] = {
	// first triangle
	-0.9f, -0.5f, 0.0f,  // left 
	-0.0f, -0.5f, 0.0f,  // right
	-0.45f, 0.5f, 0.0f,  // top 
	// second triangle
	 0.0f, -0.5f, 0.0f,  // left
	 0.9f, -0.5f, 0.0f,  // right
	 0.45f, 0.5f, 0.0f   // top 
};

unsigned int indices[] = {
	0, 1, 3,
	1, 2, 3
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void init_glfw()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

GLFWwindow* createWindow(int x, int y, char* name)
{
	GLFWwindow* window = glfwCreateWindow(x, y, name, NULL, NULL);
	if (window == NULL)
	{
		glfwTerminate();
		std::cout << "Failed to create GLFW window" << std::endl;
		throw "ERROR";
	}
}

void loadGlad()
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		throw "ERROR";
	}
}

unsigned int createAndCompileShader(GLenum type, const char* source)
{
	unsigned int shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		throw "ERROR";
	}

	return shader;
}

unsigned int createAndLinkShaderProgram(unsigned int* shaders, int numOfShaders)
{
	unsigned int shaderProgram = glCreateProgram();
	for (int i = 0; i < numOfShaders; i++)
	{
		glAttachShader(shaderProgram, shaders[i]);
	}
	glLinkProgram(shaderProgram);

	int success;
	char infoLog[512];
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::PROGRAM::SHADER::LINK_FAILED\n" << infoLog << std::endl;
		throw "ERROR";
	}

	return shaderProgram;
}

int main()
{
	init_glfw();
	GLFWwindow* window = createWindow(800, 600, "LearnOpenGL");
	glfwMakeContextCurrent(window);
	loadGlad();

	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//Load and compile the shaders and link them in a program
	unsigned int vertexShader = createAndCompileShader(GL_VERTEX_SHADER, vertexShaderSource);
	unsigned int fragmentShaderOrange = createAndCompileShader(GL_FRAGMENT_SHADER, fragmentShaderSourceOrange);
	unsigned int fragmentShaderYellow = createAndCompileShader(GL_FRAGMENT_SHADER, fragmentShaderSourceYellow);
	unsigned int shaderPrograms[2];
	unsigned int shaders1[] = { vertexShader, fragmentShaderOrange };
	shaderPrograms[0] = createAndLinkShaderProgram(shaders1, 2);
	unsigned int shaders2[] = { vertexShader, fragmentShaderYellow };
	shaderPrograms[1] = createAndLinkShaderProgram(shaders2, 2);


	//clean up shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShaderOrange);
	glDeleteShader(fragmentShaderYellow);

	//Generate objects
	glGenVertexArrays(2, VAOs);
	glGenBuffers(2, VBOs);
	glGenBuffers(1, &EBO);
	
	//Binding VAO - Will record the next stuff i do with VBO and EBO
	glBindVertexArray(VAOs[0]);

	//Create VBO and attach our vertices data to it
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//Inform OpenGL how to interpret out vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//Create EBO and attach our indices to it
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//Binding VAO - Will record the next stuff i do with VBO and EBO
	glBindVertexArray(VAOs[1]);

	//Create VBO and attach our vertices data to it
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//Inform OpenGL how to interpret out vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//Unbind VAO (Which also unbinds VBO)
	glBindVertexArray(0);

	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		//Set background color
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shaderPrograms[0]);

		//Bind our VAO that we've set up, draw triangle, then unbind
		glBindVertexArray(VAOs[0]);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(VAOs[1]);
		glUseProgram(shaderPrograms[1]);
		glDrawArrays(GL_TRIANGLES, 3, 3);
		glBindVertexArray(0);
		
		//Show our drawn buffer on window, and listen for events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(2, VAOs);
	glDeleteBuffers(2, VBOs);
	glDeleteBuffers(1, &EBO);
	glDeleteProgram(shaderPrograms[0]);
	glDeleteProgram(shaderPrograms[1]);


	glfwTerminate();
	return 0;
}