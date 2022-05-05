#include <glad/glad.h> // include glad to get all the required OpenGL headers
#include <GLFW/glfw3.h>

#include <string>
#include <fstream>

#include <iostream>
#include <sstream>

#include <ctime>


class Shader
{
public:
	unsigned int ID;
	// constructor generates the shader on the fly
	// ------------------------------------------------------------------------
	Shader(const char* vertexPath, const char* fragmentPath);
	
	// activate the shader
	// ------------------------------------------------------------------------
	void use();
	// utility uniform functions
	// ------------------------------------------------------------------------
	void setBool(const std::string &name, bool value) const;
	// ------------------------------------------------------------------------
	void setInt(const std::string &name, int value) const;
	// ------------------------------------------------------------------------
	void setFloat(const std::string &name, float value) const;
	// ------------------------------------------------------------------------
	void setVector4f(const std::string &name, float valuex, float valuey, float valuez, float valuew);

private:
	// utility function for checking shader compilation/linking errors.
	// ------------------------------------------------------------------------
	void checkCompileErrors(unsigned int shader, std::string type);
};