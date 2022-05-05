// Розроблено на базі прикладу з: https://learnopengl.com/

// TODO: доперевод

#include "shader.h"

struct vclr3f
{
	float r, g, b;
};

// ГЛОБАЛЬНІ ЗМІННІ
Shader * shad;
bool useebo = false;
bool ebokeydis = false;
bool randcolordis = false;
bool needshaderrefresh = true;
int clrindex = 0;
vclr3f shapecolor = {1.0f,0.0f,0.0f};

//ВХІДНІ СТРУКТУРИ ДАНИХ

// масив вершин трикутника
//   кожні три точки - координата вершини
//   в тривимірному просторі
float vertices[] = {
	-0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	 0.0f,  0.5f, 0.0f
};

//масив вершин чотирикутника
float ebovertices[] = {
	 0.6f,  0.4f, 0.0f,  // top right
	 0.6f, -0.4f, 0.0f,  // bottom right
	-0.6f, -0.4f, 0.0f,  // bottom left
	-0.6f,  0.4f, 0.0f   // top left 
};
unsigned int eboindices[] = {  // note that we start from 0!
	0, 1, 3,   // first triangle
	1, 2, 3    // second triangle
};

// пам'ять для вершинного буфера
unsigned int VBO;
// пам'ять для об'єкту масиву вершин
unsigned int VAO;
// пам'ять для елементного буферу
unsigned int EBO;

//ФУНКЦІЇ ЗВОРОТНОГО ВИКЛИКУ
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	//аналог ReshapeFunc з GLUT
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
	//Обробник подій вводу

	//Вихід на клавішу ESC
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//Переключення режиму на клавішу E
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		if (!ebokeydis)
		{
			ebokeydis = true;
			if (useebo) useebo = false; else useebo = true;
			if (useebo)
				std::cout << "INPUT: EBO enabled\n";
			if (!useebo)
				std::cout << "INPUT: EBO disabled\n";
		}
	}
	
	//переключення кольору на клавішу R
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		if (!randcolordis)
		{
			randcolordis = true;
			shapecolor.r = (float)(rand() % 256) / 256.0f;
			shapecolor.g = (float)(rand() % 256) / 256.0f;
			shapecolor.b = (float)(rand() % 256) / 256.0f;
			std::printf("INPUT: Color regenerated: %f %f %f\n",shapecolor.r, shapecolor.g,shapecolor.b);
			needshaderrefresh = true;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE)
	{
		if (ebokeydis) ebokeydis = false;
	}

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
	{
		if (randcolordis) randcolordis = false;
	}
}

//ФУНКЦІЇ РЕНДЕРІНГУ
void PrepShaders(const char* vertexPath, const char* fragmentPath)
{
	delete(shad);
	shad=new Shader(vertexPath, fragmentPath);
	shad->use();
	shad->setVector4f("nColor", shapecolor.r, shapecolor.g, shapecolor.b, 1.0f);
}

void rendertriangle()
{
	// 1 - Виділення буферу для зберігання вершин

	glGenBuffers(1, &VBO); //генерація вершинного буфера
	glBindBuffer(GL_ARRAY_BUFFER, VBO); // вказання типу буфера (буфер масиву)
	glBufferData(GL_ARRAY_BUFFER, // передача буферу
		sizeof(vertices),		//розміром з наш масив вершин	
		vertices,				//нашого масиву вершин
		GL_STATIC_DRAW);		//в статичному режимі

	/*
	GL_STREAM_DRAW: the data is set only once and used by the GPU at most a few times.
	GL_STATIC_DRAW: the data is set only once and used many times.
	GL_DYNAMIC_DRAW: the data is changed a lot and used many times.
	*/

	// 2 - Генерація вершинного масиву
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//призначаємо атрибути вершин
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	/*
	- The first parameter specifies which vertex attribute we want to configure. Remember that we specified the location of the position vertex attribute in the vertex shader with layout (location = 0). This sets the location of the vertex attribute to 0 and since we want to pass data to this vertex attribute, we pass in 0.
	- The next argument specifies the size of the vertex attribute. The vertex attribute is a vec3 so it is composed of 3 values.
	- The third argument specifies the type of the data which is GL_FLOAT (a vec* in GLSL consists of floating point values).
	- The next argument specifies if we want the data to be normalized. If we're inputting integer data types (int, byte) and we've set this to GL_TRUE, the integer data is normalized to 0 (or -1 for signed data) and 1 when converted to float. This is not relevant for us so we'll leave this at GL_FALSE.
	- The fifth argument is known as the stride and tells us the space between consecutive vertex attributes. Since the next set of position data is located exactly 3 times the size of a float away we specify that value as the stride. Note that since we know that the array is tightly packed (there is no space between the next vertex attribute value) we could've also specified the stride as 0 to let OpenGL determine the stride (this only works when values are tightly packed). Whenever we have more vertex attributes we have to carefully define the spacing between each vertex attribute but we'll get to see more examples of that later on.
	- The last parameter is of type void* and thus requires that weird cast. This is the offset of where the position data begins in the buffer. Since the position data is at the start of the data array this value is just 0. We will explore this parameter in more detail later on
	*/

	// 3 - Відмальовка
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// 4 - Відключаємо вершинний масив
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

void renderrectangle()
{

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ebovertices), ebovertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(eboindices), eboindices, GL_STATIC_DRAW);

	glBindVertexArray(VAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &EBO);
}


void renderblock()
{
	// Ця функція відповідає за остаточний вивід зображення

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	if (needshaderrefresh)
	{
		PrepShaders("vert_shader.gls", "frag_shader.gls");
		needshaderrefresh = false;		
	}

	if (!useebo) rendertriangle();
	else renderrectangle();

}


//ГОЛОВНА ФУНКЦІЯ
int main()
{
	// ініціюємо генератор випадкових чисел
	srand(time(NULL));

	// -- СТВОРЮЄМО ВІКНО GLFW --
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Shader-based render", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// -- GLAD --
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}


	// -- НАЛАШТУВАННЯ ЗВОРОТНОГО ВИКЛИКУ --
	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// -- ЗАПУСК ГОЛОВНОГО ЦИКЛУ --
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		//тут мають викликатись задачі відмальовки
		renderblock();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Вихід з додатку
	glfwTerminate();

	return 0;
}
