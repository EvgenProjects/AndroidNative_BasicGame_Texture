#pragma once

#include <GLES2/gl2.h>

struct PosXYZ_TextureXY
{
	float x;
	float y;
	float z;
	float texture_x;
	float texture_y;
};

class TextureImageOpenGL
{
	public:
	TextureImageOpenGL();
	virtual ~TextureImageOpenGL();

	public:
	bool Create(AAssetManager* pAssetManager, const char* filename, GLuint imgFormat);
	int32_t GetTextureId();

	private:
	GLuint m_TextureId;
	void* LoadImageFromAssets(AAssetManager* pAssetManager, const char* filename, int* pFileSize);
	void Swap24BitColors(void* pBuffer, int count);
	void Bmp24BitUpDownLines(void* pColorData, int width, int height);
	void GetBmpWidthHeight(void* pFileData, int count, int* pWidth, int* pHeight, int* pDataOffset);
};

class BufferPointsOpenGL
{
	public:
	BufferPointsOpenGL();
	virtual ~BufferPointsOpenGL();

	public:
	bool Create(PosXYZ_TextureXY* pBuffer, int pointsCount, GLuint pointsModeHowToDraw);
	int32_t GetBufferId();
	GLuint GetPointsModeHowToDraw();
	GLuint GetPointsCount();

	private:
	GLuint m_BufferId;
	GLuint m_PointsModeHowToDraw;
	GLuint m_PointsCount;
};

class ShaderOpenGL
{
	public:
	ShaderOpenGL();
	virtual ~ShaderOpenGL();

	public:
	bool Create();
	void MakeActive();
	void Draw(TextureImageOpenGL& rTextureImageOpenGL, BufferPointsOpenGL& rBufferPointsOpenGL);
	void Disable();
	void Close();

	private:
	GLuint LoadShader(const char *shaderSrc, GLenum type);
	int32_t m_ShaderId;
	int m_SHADER_TEXTURE_parameter_position_index;
	int m_SHADER_TEXTURE_parameter_texture_index;
};