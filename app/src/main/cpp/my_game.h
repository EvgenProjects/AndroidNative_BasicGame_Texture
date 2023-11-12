#include <android/input.h>
#include <android/asset_manager.h>
#include <EGL/egl.h>
#include "texture_buffer_shader.h"

class MyGame
{
    // constructor
    public: MyGame(AAssetManager* pAssetManager);

    // my objects
    protected: TextureImageOpenGL m_TextureImage_Hero;
    protected: TextureImageOpenGL m_TextureImage_Ghost;

    protected: BufferPointsOpenGL m_BufferPoints_Hero;
    protected: BufferPointsOpenGL m_BufferPoints_Ghost1;
    protected: BufferPointsOpenGL m_BufferPoints_Ghost2;

    protected: ShaderOpenGL m_ShaderOpenGL;

    // fields
    protected: AAssetManager* m_pAssetManager;
    protected: EGLDisplay m_Display = EGL_NO_DISPLAY;
    protected: EGLSurface m_Surface = EGL_NO_SURFACE;
    protected: EGLContext m_Context = EGL_NO_CONTEXT;
    protected: EGLConfig m_configOpenGL = nullptr;
    protected: int32_t m_Width;
    protected: int32_t m_Height;

    // events
    public: void OnActiveFocus();
    public: void OnLostFocus();
    public: bool OnHandleTouch(AInputEvent* pEvent);
    public: void OnNextTick();

    // OpenGL
    public: void CreateSurfaceFromWindow_OpenGL(ANativeWindow* pWindow); // calling when window set focus (need recreate Surface OpenGL)
    public: void KillSurface_OpenGL(); // calling when window kill focus (need destroy Surface OpenGL)
    public: bool MakeCurrent_Display_Surface_Context_OpenGL();
    public: bool InitGraphic_OpenGL(ANativeWindow* pWindow);
    public: void CloseGraphic_OpenGL();
    public: void DrawGraphic_OpenGL();
};

