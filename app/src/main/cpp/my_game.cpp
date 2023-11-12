#include <android/native_window.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "my_game.h"

MyGame::MyGame(AAssetManager* pAssetManager)
{
	m_Width = 0;
	m_Height = 0;
	m_pAssetManager = pAssetManager;
}

void MyGame::OnActiveFocus()
{
}

void MyGame::OnLostFocus()
{
}

bool MyGame::OnHandleTouch(AInputEvent* pEvent)
{
	return false; // event not handled
}

void MyGame::OnNextTick()
{
}

// open GL
void MyGame::DrawGraphic_OpenGL()
{
	if (m_Display == EGL_NO_DISPLAY || m_Surface == EGL_NO_SURFACE || m_Context == EGL_NO_CONTEXT)
		return;

	// green color
	glClearColor(0.72f, 0.87f, 0.55f, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	m_ShaderOpenGL.MakeActive();
	m_ShaderOpenGL.Draw(m_TextureImage_Hero, m_BufferPoints_Hero);
	m_ShaderOpenGL.Draw(m_TextureImage_Ghost, m_BufferPoints_Ghost1);
	m_ShaderOpenGL.Draw(m_TextureImage_Ghost, m_BufferPoints_Ghost2);
	m_ShaderOpenGL.Disable();

	eglSwapBuffers(m_Display, m_Surface);
}

void MyGame::CreateSurfaceFromWindow_OpenGL(ANativeWindow* pWindow)
{
	const EGLint attribs[] = {EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
							  EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
							  EGL_BLUE_SIZE, 8,
							  EGL_GREEN_SIZE, 8,
							  EGL_RED_SIZE, 8,
							  EGL_NONE};
	EGLint format;
	EGLint numConfigs;

	eglChooseConfig(m_Display, attribs, &m_configOpenGL, 1, &numConfigs);

	eglGetConfigAttrib(m_Display, m_configOpenGL, EGL_NATIVE_VISUAL_ID, &format);

	m_Surface = eglCreateWindowSurface(m_Display, m_configOpenGL, pWindow, NULL);
}

void MyGame::KillSurface_OpenGL()
{
	eglMakeCurrent(m_Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	if (m_Surface != EGL_NO_SURFACE)
	{
		eglDestroySurface(m_Display, m_Surface);
		m_Surface = EGL_NO_SURFACE;
	}
}

bool MyGame::MakeCurrent_Display_Surface_Context_OpenGL()
{
	if (eglMakeCurrent(m_Display, m_Surface, m_Surface, m_Context) == EGL_FALSE)
		return false;
	return true;
}

bool MyGame::InitGraphic_OpenGL(ANativeWindow* pWindow)
{
	// init Display
	m_Display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(m_Display, nullptr, nullptr);

	// init Surface
	CreateSurfaceFromWindow_OpenGL(pWindow);

	// init Context
	EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2,EGL_NONE };
	m_Context = eglCreateContext(m_Display, m_configOpenGL, NULL, contextAttribs);

	if (!MakeCurrent_Display_Surface_Context_OpenGL())
		return false;

	EGLint w, h;
	eglQuerySurface(m_Display, m_Surface, EGL_WIDTH, &w);
	eglQuerySurface(m_Display, m_Surface, EGL_HEIGHT, &h);

	m_Width = w;
	m_Height = h;

	// Open GL states
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	// for alpha color (transparency)
	glEnable( GL_BLEND );
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// create shader (for drawing vertex very fast)
    m_ShaderOpenGL.Create();

	// load textures
	m_TextureImage_Hero.Create(m_pAssetManager, "hero.bmp", GL_RGB); // my file from assets folder
	m_TextureImage_Ghost.Create(m_pAssetManager, "ghost.bmp", GL_RGB); // my file from assets folder

	// create display points and texture points
	float z = 0;
	PosXYZ_TextureXY* pBuffer = nullptr;

	pBuffer = new PosXYZ_TextureXY[]{
			{.x=0.0, .y=0.5, .z=z, .texture_x=0, .texture_y=0}, // left top
			{.x=0.4, .y=0.5, .z=z, .texture_x=1, .texture_y=0}, // right top
			{.x=0.0, .y=0.9, .z=z, .texture_x=0, .texture_y=1}, // left bottom
			{.x=0.4, .y=0.9, .z=z, .texture_x=1, .texture_y=1} // right bottom
	};
	m_BufferPoints_Hero.Create(pBuffer, 4,GL_TRIANGLE_STRIP);
	delete[] pBuffer;

	pBuffer = new PosXYZ_TextureXY[]{
			{.x=0.5, .y=0.2, .z=z, .texture_x=0, .texture_y=0}, // left top
			{.x=0.7, .y=0.2, .z=z, .texture_x=1, .texture_y=0}, // right top
			{.x=0.5, .y=0.4, .z=z, .texture_x=0, .texture_y=1}, // left bottom
			{.x=0.7, .y=0.4, .z=z, .texture_x=1, .texture_y=1} // right bottom
	};
	m_BufferPoints_Ghost1.Create(pBuffer, 4,GL_TRIANGLE_STRIP);
	delete[] pBuffer;

	pBuffer = new PosXYZ_TextureXY[]{
			{.x=0.8, .y=0.6, .z=z, .texture_x=0, .texture_y=0}, // left top
			{.x=1,   .y=0.6, .z=z, .texture_x=1, .texture_y=0}, // right top
			{.x=0.8, .y=0.8, .z=z, .texture_x=0, .texture_y=1}, // left bottom
			{.x=1,   .y=0.8, .z=z, .texture_x=1, .texture_y=1} // right bottom
	};
	m_BufferPoints_Ghost2.Create(pBuffer, 4,GL_TRIANGLE_STRIP);
	delete[] pBuffer;

	return true;
}

void MyGame::CloseGraphic_OpenGL()
{
	if (m_Display != EGL_NO_DISPLAY)
	{
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

    m_ShaderOpenGL.Close();
}