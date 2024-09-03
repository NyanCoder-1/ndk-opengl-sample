#include "Graphics.hpp"
#include <iostream>
#include <vector>

constexpr const char *vertexShaderSrc = "#version 300 es\n"
	"in vec4 vPosition;\n"
	"in vec4 vColor;\n"
	"out vec4 fColor;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vPosition;\n"
	"	fColor = vColor;\n"
	"}\n";
constexpr const char *fragmentShaderSrc = "#version 300 es\n"
	"precision mediump float;\n"
	"in vec4 fColor;\n"
	"out vec4 FragColor;\n"
	"void main()\n"
	"{\n"
	"	FragColor = fColor;\n"
	"}\n";

void Graphics::OnDraw()
{
	if (m_Display == EGL_NO_DISPLAY || m_Surface == EGL_NO_SURFACE || m_Context == EGL_NO_CONTEXT)
		return;

	glClearColor(0.25f, 0.25f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (m_isGeometryInited) {
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO[0]);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	eglSwapBuffers(m_Display, m_Surface);
}


void Graphics::OnCreateWindow(ANativeWindow *pWindow)
{
	if (m_isGraphicInited) {
		Graphics::CreateSurfaceFromWindow(pWindow);
		eglMakeCurrent(m_Display, m_Surface, m_Surface, m_Context);

		if (!m_isGeometryInited)
			Graphics::InitGeometry();

		return;
	}

	// init Display
	m_Display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(m_Display, nullptr, nullptr);

	// init Surface
	Graphics::CreateSurfaceFromWindow(pWindow);

	// init Context
	EGLint contextAttrs[] = {EGL_CONTEXT_CLIENT_VERSION, 3/*openGL version 3*/ ,EGL_NONE };
	m_Context = eglCreateContext(m_Display, m_configOpenGL, nullptr, contextAttrs);

	if (eglMakeCurrent(m_Display, m_Surface, m_Surface, m_Context) == EGL_FALSE)
		return;

	EGLint w, h;
	eglQuerySurface(m_Display, m_Surface, EGL_WIDTH, &w);
	eglQuerySurface(m_Display, m_Surface, EGL_HEIGHT, &h);

	m_Width = w;
	m_Height = h;

	// Open GL states
	//glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	// for alpha color (transparency)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (!m_isGeometryInited)
		Graphics::InitGeometry();

	m_isGraphicInited = true; 
}

void Graphics::OnKillWindow()
{
	FreeGeometry();

	if (m_Display != EGL_NO_DISPLAY) {
		eglMakeCurrent(m_Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

		if (m_Context != EGL_NO_CONTEXT) {
			eglDestroyContext(m_Display, m_Context);
		}

		if (m_Surface != EGL_NO_SURFACE) {
			eglDestroySurface(m_Display, m_Surface);
		}

		eglTerminate(m_Display);
	}

	m_Display = EGL_NO_DISPLAY;
	m_Context = EGL_NO_CONTEXT;
	m_Surface = EGL_NO_SURFACE;
	m_isGraphicInited = false;
}

void Graphics::CreateSurfaceFromWindow(ANativeWindow *pWindow)
{
	const EGLint attrs[] = {EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
								EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
								EGL_BLUE_SIZE, 8,
								EGL_GREEN_SIZE, 8,
								EGL_RED_SIZE, 8,
								EGL_NONE};
	EGLint format;
	EGLint numConfigs;

	eglChooseConfig(m_Display, attrs, &m_configOpenGL, 1, &numConfigs);

	eglGetConfigAttrib(m_Display, m_configOpenGL, EGL_NATIVE_VISUAL_ID, &format);

	m_Surface = eglCreateWindowSurface(m_Display, m_configOpenGL, pWindow, nullptr);
}

void Graphics::InitGeometry()
{
	GLuint vertexShader;
	GLuint fragmentShader;

	{ // compile vertex shader
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
		glCompileShader(vertexShader);
		GLint status = 0;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE) {
			GLint infoLogLength = 0;
			glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);
			std::vector<char> vertexShaderErrorMessage(infoLogLength);
			glGetShaderInfoLog(vertexShader, infoLogLength, nullptr, &vertexShaderErrorMessage[0]);
			std::cerr << vertexShaderErrorMessage.data() << std::endl;
			glDeleteShader(vertexShader);

			return;
		}
	}
	{ // compile fragment shader
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSrc, nullptr);
		glCompileShader(fragmentShader);
		GLint status = 0;
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE) {
			GLint infoLogLength = 0;
			glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLogLength);
			std::vector<char> fragmentShaderErrorMessage(infoLogLength);
			glGetShaderInfoLog(fragmentShader, infoLogLength, nullptr, &fragmentShaderErrorMessage[0]);
			std::cerr << fragmentShaderErrorMessage.data() << std::endl;
			glDeleteShader(fragmentShader);

			return;
		}
	}

	shaderProgram = glCreateProgram();
	if (shaderProgram == 0)
		return;

	{ // link program
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);

		glLinkProgram(shaderProgram);
		GLint status = 0;
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
		if (status == GL_FALSE) {
			GLint infoLogLength = 0;
			glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
			std::vector<char> programErrorMessage(infoLogLength);
			glGetProgramInfoLog(shaderProgram, infoLogLength, nullptr, &programErrorMessage[0]);
			std::cerr << programErrorMessage.data() << std::endl;

			glDeleteProgram(shaderProgram);
			shaderProgram = 0;
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			return;
		}
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glUseProgram(shaderProgram);

	const GLfloat vertices[] = {
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.0f,  0.5f, 0.0f
	};
	const size_t verticesSize = sizeof(vertices);

	const GLfloat colors[] = {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	};
	const size_t colorsSize = sizeof(colors);

	const auto locPosition = glGetAttribLocation(shaderProgram, "vPosition");
	const auto locColor = glGetAttribLocation(shaderProgram, "vColor");

	glGenVertexArrays(1, VAO);
	glBindVertexArray(VAO[0]);

	glGenBuffers(2, VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, colorsSize, colors, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glVertexAttribPointer(locPosition, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(locPosition);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glVertexAttribPointer(locColor, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(locColor);

	m_isGeometryInited = true;
}
void Graphics::FreeGeometry()
{
	if (!m_isGeometryInited)
		return;

	glDeleteVertexArrays(1, VAO);
	VAO[0] = 0;
	glDeleteBuffers(2, VBO);
	VBO[0] = 0;
	VBO[1] = 0;

	glDeleteProgram(shaderProgram);
	shaderProgram = 0;

	m_isGeometryInited = false;
}
