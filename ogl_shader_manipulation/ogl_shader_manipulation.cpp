// Розроблено на базі прикладу з: https://learnopengl.com/

// TODO: доперевод

#include "shader.h"

struct vclr3f
{
	float r, g, b;
};

// ГЛОБАЛЬНІ ЗМІННІ
Shader * shad;
int drawmode=0; //0 - triangle, 1 - rectangle, 2 - chart
bool ebokeydis = false;
bool randcolordis = false;
bool needshaderrefresh = true;
int clrindex = 0;
vclr3f shapecolor = {1.0f,0.0f,0.0f};
float chxmin, chymin, chxmax, chymax;

glm::mat4 proj; //сховище проекційної матриці

//попередні декларації
void rendertriangle();


//МАТЕМАТИЧНІ ФУНКЦІЇ

//розрахунок заданої функції
float calcFunc(float a, float b, float c, float x)
{
	return a * pow(x, 2) + b * x + c;
}

//заповнює вказаний вектор значеннями функції
void fillChart(float a, float b, float c, float x1, float x2, int steps, std::vector<float>&chartpts,
	float &xmin, float &xmax, float &ymin, float &ymax)
{
	chartpts.clear();
	float d = (x2 - x1) / (float)steps;

	for (int i = 0; i < steps; i++)
	{
		float cx = x1 + i * d;
		float cy = calcFunc(a, b, c, cx);

		if (i == 0)
		{
			xmin = cx; xmax = cx;
			ymin = cy; ymax = cy;
		}
		else
		{
			if (cx < xmin) xmin = cx;
			if (cy < ymin) ymin = cy;
			if (cx > xmax) xmax = cx;
			if (cy > ymax) ymax = cy;
		}

		chartpts.push_back(cx); //x coord
		chartpts.push_back(cy); //y coord
		chartpts.push_back(0.0f); //z coord

		printf("%d) %f; %f; %f;\n",(int)(i/3), chartpts[chartpts.size()-3], chartpts[chartpts.size()-2], chartpts[chartpts.size()-1]);

	}
}


//ВХІДНІ СТРУКТУРИ ДАНИХ

// масив вершин трикутника
//   кожні три точки - координата вершини
//   в тривимірному просторі
std::vector<float> vertices = {
	-0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	 0.0f,  0.5f, 0.0f
};
//контейнер вершин
float* verts;

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


// масив вершин графіка
std::vector<float> chvertices = {};
//контейнер вершин
float* chverts;

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
			drawmode++;
			if (drawmode > 2) drawmode = 0;
			needshaderrefresh = true;
			std::cout << "INPUT: Drawmode set to " << drawmode << "\n";
			
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

//оновлює значення уніформ
void RefreshUniforms()
{
	shad->setVector4f("nColor", shapecolor.r, shapecolor.g, shapecolor.b, 1.0f);
	shad->setMatrix4f("projection", proj);
}

//викликає підготовку шейдерів
void PrepShaders(const char* vertexPath, const char* fragmentPath)
{
	delete(shad);
	shad=new Shader(vertexPath, fragmentPath);
	shad->use();
}

void drawLine(float x1, float y1, float x2, float y2, bool axis)
{

	float lineverts[] = { x1, y1, 0.0f, x2, y2, 0.0f };
	shad->setBool("isBgColor", true);
	shad->setBool("isAxis", axis);
	//1.2 - заповнюємо вершинний масив
	glGenBuffers(1, &VBO); //генерація вершинного буфера
	glBindBuffer(GL_ARRAY_BUFFER, VBO); // вказання типу буфера (буфер масиву)
	glBufferData(GL_ARRAY_BUFFER, // передача буферу
		sizeof(float)*6,		//розміром з наш масив вершин	
		lineverts,				//нашого масиву вершин
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
	glDrawArrays(GL_LINES, 0, 6);

	// 4 - Відключаємо вершинний масив
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);

	shad->setBool("isBgColor", false);
}


int nsteps(float min, float max, float dlt)
{
	return int(((max-min)/dlt));
}

void renderchart()
{
	// 0 - Малюємо сітку за допомогою функції drawLine
	// 0.1 - рамка
	drawLine(chxmin, chymin, chxmin, chymax, false);
	drawLine(chxmin, chymax, chxmax, chymax, false);
	drawLine(chxmax, chymax, chxmax, chymin, false);
	drawLine(chxmax, chymin, chxmin, chymin, false);

	float deltax = 1;
	float deltay = 1;
	int nstepsx = nsteps(chxmin, chxmax, deltax);
	int nstepsy = nsteps(chymin, chymax, deltay);
	while (nstepsx > 20)
	{
		deltax = deltax * 10;
		nstepsx = nsteps(chxmin, chxmax, deltax);
	}

	while (nstepsy > 20)
	{
		deltay = deltay * 10;
		nstepsy = nsteps(chymin, chymax, deltay);
	}

	//вертикальні лінії
	float lp = 0;
	while (lp<chxmax)
	{		
		bool da = false;
		if (lp == 0) da = true;
		if ((lp>=chxmin) && (lp<=chxmax))
		drawLine(lp, chymin, lp, chymax, da);
		lp+=deltax;
	}
	lp = -deltax;
	while (lp > chxmin)
	{
		if ((lp >= chxmin) && (lp <= chxmax))
			drawLine(lp, chymin, lp, chymax, false);
		lp -= deltax;
	}

	//горизонтальні лінії
	lp = 0;
	while (lp < chymax)
	{
		bool da = false;
		if (lp == 0) da = true;
		if ((lp >= chymin) && (lp <= chymax))
			drawLine(chxmin, lp, chxmax, lp, da);
		lp+=deltay;
	}
	lp = -deltay;
	while (lp > chymin)
	{
		if ((lp >= chymin) && (lp <= chymax))
			drawLine(chxmin, lp, chxmax, lp, false);
		lp -= deltay;
	}

	//відновлюємо уніформи
	RefreshUniforms();
	
	// 1 - Виділення буферу для зберігання вершин

	//1.0 - відмальовуємо трикутник для перевірки роботи масштабу
	//rendertriangle();

	//1.1 - генеруємо дані
	delete(chverts);
	chverts = new float[chvertices.size()];
	for (int i = 0; i < chvertices.size(); i++)
	{
		chverts[i] = chvertices[i];
	}

	//1.2 - заповнюємо вершинний масив
	glGenBuffers(1, &VBO); //генерація вершинного буфера
	glBindBuffer(GL_ARRAY_BUFFER, VBO); // вказання типу буфера (буфер масиву)
	glBufferData(GL_ARRAY_BUFFER, // передача буферу
		sizeof(float)*chvertices.size(),		//розміром з наш масив вершин	
		chverts,				//нашого масиву вершин
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
	glDrawArrays(GL_LINE_STRIP, 0, (int)(chvertices.size()/3));

	// 4 - Відключаємо вершинний масив
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

void rendertriangle()
{

	RefreshUniforms();

	// 1 - Виділення буферу для зберігання вершин

	//1.1 - генеруємо дані
	delete(verts);
	verts = new float[vertices.size()];	
	for (int i = 0; i < vertices.size(); i++)
	{
		verts[i] = vertices[i];
	}
	
	//1.2 - заповнюємо вершинний масив
	glGenBuffers(1, &VBO); //генерація вершинного буфера
	glBindBuffer(GL_ARRAY_BUFFER, VBO); // вказання типу буфера (буфер масиву)
	glBufferData(GL_ARRAY_BUFFER, // передача буферу
		sizeof(float)*vertices.size(),		//розміром з наш масив вершин	
		verts,				//нашого масиву вершин
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

	RefreshUniforms();

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

//визначає проекцію для даного режиму відмальовки
void setproj(int dm)
{
	switch (dm)
	{
	case 0:
	case 1:
	{
		proj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f); 
		std::cout << "Set to default projection.\n";
		break;		
	}
	case 2:
	{
		proj = glm::ortho(chxmin, chxmax, chymin, chymax, -1.0f, 1.0f);
		printf("Set to chart-based projection: %f; %f; %f; %f.\n", chxmin, chxmax, chymin, chymax);
		break;
	}
	}
}

void renderblock()
{
	// Ця функція відповідає за остаточний вивід зображення

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	if (needshaderrefresh)
	{
		setproj(drawmode);
		needshaderrefresh = false;		
	}

	if (drawmode == 0) rendertriangle();
	if (drawmode == 1) renderrectangle();
	if (drawmode == 2) renderchart();

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

	// -- МАТЕМАТИКА --
	fillChart(10, 2, -7, -10, 10, 100, chvertices, chxmin, chxmax, chymin, chymax);

	// -- НАЛАШТУВАННЯ ЗВОРОТНОГО ВИКЛИКУ --
	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//побудова шейдерів
	PrepShaders("vert_shader.gls", "frag_shader.gls");

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
