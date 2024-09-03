#pragma once

#include <android/native_window.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <memory>

class Graphics : public std::enable_shared_from_this<Graphics>
{
	struct Private{ explicit Private() = default; };
public:
	typedef std::shared_ptr<Graphics> Ptr;

	explicit Graphics(Private) {}
	virtual ~Graphics() = default;

	static std::shared_ptr<Graphics> Create() {
		return std::make_shared<Graphics>(Private());
	}

	void OnDraw();

	void OnCreateWindow(ANativeWindow *pWindow);
	void OnKillWindow();

private:
	void CreateSurfaceFromWindow(ANativeWindow *pWindow);
	void InitGeometry();
	void FreeGeometry();

	EGLDisplay m_Display = EGL_NO_DISPLAY;
	EGLSurface m_Surface = EGL_NO_SURFACE;
	EGLContext m_Context = EGL_NO_CONTEXT;
	EGLConfig m_configOpenGL = nullptr;
	bool m_isGraphicInited = false;
	bool m_isGeometryInited = false;
	int32_t m_Width = 0;
	int32_t m_Height = 0;

	GLuint VAO[1] = {0};
	GLuint VBO[2] = {0};
	GLuint shaderProgram = 0;
};
