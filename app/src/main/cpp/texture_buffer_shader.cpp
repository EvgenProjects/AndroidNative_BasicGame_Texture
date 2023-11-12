#include <android/native_window.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android/asset_manager.h>
#include "texture_buffer_shader.h"

#include <string>

/////////// class TextureImageOpenGL //////////////
TextureImageOpenGL::TextureImageOpenGL()
{
	m_TextureId = 0;
}

TextureImageOpenGL::~TextureImageOpenGL()
{
}

bool TextureImageOpenGL::Create(AAssetManager* pAssetManager, const char* filename, GLuint imgFormat)
{
	m_TextureId = 0;
	int fileSize = 0;
	void* pFileData = LoadImageFromAssets(pAssetManager, filename, &fileSize);

	int width = 0;
	int height = 0;
	int dataOffset = 0;
	GetBmpWidthHeight(pFileData, fileSize, &width, &height, &dataOffset);

	void* pColorData = (char*)pFileData + dataOffset;
	Swap24BitColors(pColorData, fileSize-dataOffset);
	Bmp24BitUpDownLines(pColorData, width, height);

	GLuint textureID = 0;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, imgFormat, width, height, 0, imgFormat, GL_UNSIGNED_BYTE, pColorData);

	int status = glGetError();
	if (status == GL_NO_ERROR) // good
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else // error
	{
		textureID = 0;
		return false;
	}

	if (pFileData!= nullptr)
		delete[] ((char*)pFileData);

	glBindTexture(GL_TEXTURE_2D, 0);

	m_TextureId = textureID;
	return true;
}

int32_t TextureImageOpenGL::GetTextureId()
{
	return m_TextureId;
}

void* TextureImageOpenGL::LoadImageFromAssets(AAssetManager* pAssetManager, const char* filename, int* pFileSize)
{
	*pFileSize = 0;
	int readBytesCount = 0;
	char* buffer = nullptr;

	// open file
	AAsset* pAsset = AAssetManager_open(pAssetManager, filename, AASSET_MODE_UNKNOWN);
	if (pAsset== nullptr)
		return nullptr;

	int fileSize = AAsset_getLength64(pAsset);
	if (fileSize==0)
		return nullptr;

	*pFileSize = fileSize;

	buffer = new char [fileSize];
	if (buffer== nullptr)
		return nullptr;

	// read file
	readBytesCount = AAsset_read(pAsset, buffer, fileSize);
	AAsset_close(pAsset);

	if (readBytesCount==fileSize) // good
		return buffer;

	delete[] buffer;
	return nullptr;
}

void TextureImageOpenGL::Swap24BitColors(void* pColorData, int count)
{
	unsigned char* pBuffer = (unsigned char*)pColorData;
	for (int i=0; i<(count-2); i+=3)
		std::swap(pBuffer[i], pBuffer[i+2]);
}

void TextureImageOpenGL::Bmp24BitUpDownLines(void* pColorData, int width, int height)
{
	int bytesCountPerColor = 3;
	const int bytesCountFor1Line = width * bytesCountPerColor;
	char* pData =(char*) pColorData;
	char* pLine = new char[bytesCountFor1Line];
	int iBottom = 0;
	for (int iTop=0; iTop< height / 2; iTop++)
	{
		iBottom = height-1 - iTop;
		memcpy(pLine, pData+(iTop*bytesCountFor1Line), bytesCountFor1Line);
		memcpy(pData+(iTop*bytesCountFor1Line), pData+(iBottom*bytesCountFor1Line), bytesCountFor1Line);
		memcpy(pData+(iBottom*bytesCountFor1Line), pLine, bytesCountFor1Line);
	}
	delete[] pLine;
}

void TextureImageOpenGL::GetBmpWidthHeight(void* pFileData, int count, int* pWidth, int* pHeight, int* pDataOffset)
{
	const int DATA_OFFSET_OFFSET = 0x000A;
	const int WIDTH_OFFSET = 0x0012;
	const int HEIGHT_OFFSET = 0x0016;

	*pWidth = *(int*)((char*)pFileData + WIDTH_OFFSET);
	*pHeight = *(int*)((char*)pFileData + HEIGHT_OFFSET);
	*pDataOffset = *(int*)((char*)pFileData + DATA_OFFSET_OFFSET);
}

//////////// class BufferPointsOpenGL to create 3d points in video card buffer ///////////////
BufferPointsOpenGL::BufferPointsOpenGL()
{
	m_BufferId = 0;
	m_PointsCount = 0;
	m_PointsModeHowToDraw = 0;
}

BufferPointsOpenGL::~BufferPointsOpenGL()
{
}

bool BufferPointsOpenGL::Create(PosXYZ_TextureXY* pBuffer, int itemsCount, GLuint pointsModeHowToDraw)
{
	m_BufferId = 0;
	m_PointsCount = itemsCount;
	m_PointsModeHowToDraw = pointsModeHowToDraw;

	glGenBuffers(1, &m_BufferId);
	glBindBuffer(GL_ARRAY_BUFFER, m_BufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PosXYZ_TextureXY)*itemsCount, pBuffer, GL_STATIC_DRAW);
	return true;
}

int32_t BufferPointsOpenGL::GetBufferId()
{
	return m_BufferId;
}

GLuint BufferPointsOpenGL::GetPointsModeHowToDraw()
{
	return m_PointsModeHowToDraw;
}

GLuint BufferPointsOpenGL::GetPointsCount()
{
	return m_PointsCount;
}

//////////////////// class ShaderOpenGL ///////////////
ShaderOpenGL::ShaderOpenGL()
{
	m_ShaderId = 0;
	m_SHADER_TEXTURE_parameter_position_index = 0;
	m_SHADER_TEXTURE_parameter_texture_index = 0;
}

ShaderOpenGL::~ShaderOpenGL()
{
	Close();
}

bool ShaderOpenGL::Create()
{
	m_ShaderId = 0;

	char vertexShaderStr[] =
			"attribute vec3 aPos; \n"
			"attribute vec2 aTexCoord; \n"
			" \n"
			"varying vec2 TexCoord; \n"
			" \n"
			"void main() \n"
			"{ \n"
			"	gl_Position = vec4(aPos, 1.0); \n"
			"	gl_Position.x = gl_Position.x * 2.0 - 1.0; \n" // I use x from [0, 1] in my files. This line converts x from [0, 1] -> openGL standard [-1, 1]
			"	gl_Position.y = -(gl_Position.y * 2.0 - 1.0); \n" // I use y from [0, 1] in my files. This line converts y from [0, 1] -> openGL standard [1, -1]
			"	TexCoord = vec2(aTexCoord.x, aTexCoord.y); \n"
			"} \n";

	char fragmentShaderStr[] =
			"precision mediump float; \n"
			"varying vec2 TexCoord; \n"
			" \n"
			"uniform sampler2D textureGood; \n"
			"void main() \n"
			"{ \n"
			"   gl_FragColor = texture2D(textureGood, TexCoord); \n"
			"} \n";

	GLuint vertexShader;
	GLuint fragmentShader;
	GLint linked;

	// Load the vertex/fragment shaders
	vertexShader = LoadShader(vertexShaderStr, GL_VERTEX_SHADER);
	fragmentShader = LoadShader(fragmentShaderStr, GL_FRAGMENT_SHADER);

	// Create the program object
	int32_t myGraphicObject = glCreateProgram();
	if(myGraphicObject == 0)
	{
		return false;
	}

	// Attach shaders to program
	glAttachShader(myGraphicObject, vertexShader);
	glAttachShader(myGraphicObject, fragmentShader);

	// Link the program
	glLinkProgram(myGraphicObject);

	m_SHADER_TEXTURE_parameter_position_index = glGetAttribLocation(myGraphicObject, "aPos");
	m_SHADER_TEXTURE_parameter_texture_index = glGetAttribLocation(myGraphicObject, "aTexCoord");

	// Check the link status
	glGetProgramiv(myGraphicObject, GL_LINK_STATUS, &linked);
	if(!linked)
	{
		GLint infoLen = 0;
		glGetProgramiv(myGraphicObject, GL_INFO_LOG_LENGTH, &infoLen);

		if(infoLen > 1)
		{
			char* infoLog = new char[infoLen];
			glGetProgramInfoLog(myGraphicObject, infoLen, NULL, infoLog);
			//LOG_WARN("Error linking program:\n%s\n", infoLog);

			delete[] infoLog;
		}

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		glDeleteProgram(myGraphicObject);
		return false;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	m_ShaderId = myGraphicObject;
	return true;
}

void ShaderOpenGL::Close()
{
	if (m_ShaderId!=0)
	{
		glDeleteProgram(m_ShaderId);
		m_ShaderId = 0;
	}
}

GLuint ShaderOpenGL::LoadShader(const char *shaderSrc, GLenum type)
{
	GLuint shader;
	GLint compiled;

	// Create the shader object
	shader = glCreateShader(type);
	if(shader != 0)
	{
		// Load the shader source
		glShaderSource(shader, 1, &shaderSrc, NULL);

		// Compile the shader
		glCompileShader(shader);
		// Check the compile status
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

		if(!compiled)
		{
			GLint infoLen = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

			if(infoLen > 1)
			{
				char* infoLog = new char[infoLen];
				glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
				//LOG_WARN("Error compiling shader:\n%s\n", infoLog);
				delete[] infoLog;
			}
			glDeleteShader(shader);
			shader = 0;
		}
	}
	return shader;
}

void ShaderOpenGL::MakeActive()
{
	// Use the program object
	glUseProgram(m_ShaderId);

	glEnableVertexAttribArray(m_SHADER_TEXTURE_parameter_position_index);
	glEnableVertexAttribArray(m_SHADER_TEXTURE_parameter_texture_index);
}

void ShaderOpenGL::Disable()
{
	glDisableVertexAttribArray(m_SHADER_TEXTURE_parameter_position_index);
	glDisableVertexAttribArray(m_SHADER_TEXTURE_parameter_texture_index);
	glUseProgram(0);
}

void ShaderOpenGL::Draw(TextureImageOpenGL& rTextureImageOpenGL, BufferPointsOpenGL& rBufferPointsOpenGL)
{
	glBindTexture(GL_TEXTURE_2D, rTextureImageOpenGL.GetTextureId());

	// data
	glBindBuffer(GL_ARRAY_BUFFER, rBufferPointsOpenGL.GetBufferId());

	// vertex pos and texture coord
	glVertexAttribPointer(m_SHADER_TEXTURE_parameter_position_index,
						  3 /*fields with type float*/, // x,y,z
						  GL_FLOAT,
						  GL_FALSE,
						  sizeof(PosXYZ_TextureXY),
						  (GLvoid*)0
	);

	// texture
	glVertexAttribPointer(m_SHADER_TEXTURE_parameter_texture_index,
						  2 /*fields with type float*/, // texture_x, texture_y
						  GL_FLOAT,
						  GL_FALSE,
						  sizeof(PosXYZ_TextureXY),
						  (GLvoid*)( sizeof(PosXYZ_TextureXY::x) + sizeof(PosXYZ_TextureXY::y) + sizeof(PosXYZ_TextureXY::z) )
	);

	// draw
	glDrawArrays(rBufferPointsOpenGL.GetPointsModeHowToDraw(), 0, rBufferPointsOpenGL.GetPointsCount());
}