This project can:
 • Read bmp files from Assets folder and draw textures using shaders
 
 • This project contains only C++ code and the code is minimal
 
 • Renders the screen 50 times per second using OpenGL
 
 • Works on an old tablet, phone with version Android 4 (this version was released in 2011)
 
 • The project is very, very simple. Contains 3 files:
    main.cpp ... with 150 lines of Android system code (init device, gain focus, kill focus, handle use events like finger touch, move). This file is finished and you don't need to edit it.
    my_game.cpp ... with 180 lines of OpenGL code (init OpenGL surface, fill background, load hero.bmp and ghost.bmp from assets, draw textures, close OpenGL). This class for editting.
    texture_buffer_shader ... with 360 lines of OpenGL code (class TextureImageOpenGL for loading bmp file from the assets folder, class BufferPointsOpenGL for storing 3D points in video card buffer, class ShaderOpenGL for very fast drawing of 3D points using OpenGL shader). This file is finished and you don't need to edit it.
 
 • This project uses Android NDK (native C++ library)
 
 • This project uses  OpenGL ES (2D & 3D graphic library)
  

1) Open Android Studio
2) Compile project and run
You will see result:
![image](https://github.com/EvgenProjects/AndroidNative_BasicGame_Texture/assets/38002631/ecc448f2-6cf1-4726-b61b-25b9c0be0452)
