#include "renderer.h"

RendererGL::RendererGL() : 
   Window( nullptr ), FrameWidth( 2000 ), FrameHeight( 1000 ), FrameIndex( 0 ), ClickedPoint( -1, -1 ),
   MainCamera( std::make_unique<CameraGL>() ), ScreenObject( std::make_unique<ObjectGL>() )
{
   Renderer = this;

   initialize();
   printOpenGLInformation();
}

void RendererGL::printOpenGLInformation()
{
   std::cout << "====================== [ Renderer Information ] ================================================\n";
   std::cout << " - GLFW version supported: " << glfwGetVersionString() << "\n";
   std::cout << " - OpenGL renderer: " << glGetString( GL_RENDERER ) << "\n";
   std::cout << " - OpenGL version supported: " << glGetString( GL_VERSION ) << "\n";
   std::cout << " - OpenGL shader version supported: " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << "\n";
   std::cout << "================================================================================================\n";
}

void RendererGL::initialize()
{
   if (!glfwInit()) {
      std::cout << "Cannot Initialize OpenGL...\n";
      return;
   }
   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
   glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

   Window = glfwCreateWindow( FrameWidth, FrameHeight, "Main Camera", nullptr, nullptr );
   glfwMakeContextCurrent( Window );

   if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return;
   }
   
   registerCallbacks();
   
   glEnable( GL_DEPTH_TEST );
   glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

   MainCamera->updateWindowSize( FrameWidth, FrameHeight );

   const std::string shader_directory_path = std::string(CMAKE_SOURCE_DIR) + "/shaders";
   Shader = std::make_unique<ShaderGL>();
   Shader->setComputeShader( std::string(shader_directory_path + "/raytracer.comp").c_str() );
   Shader->setRayUniformLocations();

   ScreenShader = std::make_unique<ShaderGL>();
   ScreenShader->setShader(
      std::string(shader_directory_path + "/screen.vert").c_str(),
      std::string(shader_directory_path + "/screen.frag").c_str()
   );
   ScreenShader->setScreenUniformLocations();

   FinalCanvas = std::make_unique<CanvasGL>();
   FinalCanvas->setCanvas( FrameWidth, FrameHeight, GL_RGBA8 );
}

void RendererGL::cleanup(GLFWwindow* window)
{
   glfwSetWindowShouldClose( window, GLFW_TRUE );
}

void RendererGL::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   std::ignore = scancode;
   std::ignore = mods;
   if (action != GLFW_PRESS) return;

   switch (key) {
      case GLFW_KEY_Q:
      case GLFW_KEY_ESCAPE:
         cleanupWrapper( window );
         break;
      default:
         return;
   }
}

void RendererGL::reshape(GLFWwindow* window, int width, int height) const
{
   std::ignore = window;
   MainCamera->updateWindowSize( width, height );
   glViewport( 0, 0, width, height );
}

void RendererGL::registerCallbacks() const
{
   glfwSetWindowCloseCallback( Window, cleanupWrapper );
   glfwSetKeyCallback( Window, keyboardWrapper );
   glfwSetFramebufferSizeCallback( Window, reshapeWrapper );
}

void RendererGL::setSpheres()
{
   Spheres = {
      { Sphere::TYPE::LAMBERTIAN, 0.5f, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.8f, 0.3f, 0.3f) },
      { Sphere::TYPE::LAMBERTIAN, 100.0f, glm::vec3(0.0f, -100.5f, -1.0f), glm::vec3(0.8f, 0.8f, 0.0f) },
      { Sphere::TYPE::METAL, 0.5f, glm::vec3(1.0f, 0.0f, -1.0f), glm::vec3(0.8f, 0.6f, 0.2f) },
      { Sphere::TYPE::METAL, 0.5f, glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.8f, 0.8f, 0.8f) }
   };
}

void RendererGL::drawScene() const
{
   glUseProgram( Shader->getShaderProgram() );
   Shader->transferSphereUniformsToShader( Spheres );
   Shader->uniform1i( "FrameIndex", FrameIndex );
   glBindImageTexture( 0, FinalCanvas->getColor0TextureID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8 );
   glDispatchCompute( getGroupSize( FrameWidth ), getGroupSize( FrameHeight ), 1 );
   glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}

void RendererGL::drawScreen() const
{
   MainCamera->updateWindowSize( FrameWidth, FrameHeight );
   glViewport( 0, 0, FrameWidth, FrameHeight );

   glBindFramebuffer( GL_FRAMEBUFFER, 0 );
   glUseProgram( ScreenShader->getShaderProgram() );

   const glm::mat4 to_world = glm::scale( glm::mat4(1.0f), glm::vec3(FrameWidth, FrameHeight, 1.0f) );
   ScreenShader->transferBasicTransformationUniforms( to_world, MainCamera.get() );
   glBindTextureUnit( 0, FinalCanvas->getColor0TextureID() );
   glBindVertexArray( ScreenObject->getVAO() );
   glDrawArrays( ScreenObject->getDrawMode(), 0, ScreenObject->getVertexNum() );
}

void RendererGL::render() const
{
   glClear( OPENGL_COLOR_BUFFER_BIT | OPENGL_DEPTH_BUFFER_BIT );

   drawScene();
   drawScreen();

   glBindVertexArray( 0 );
   glUseProgram( 0 );
}

void RendererGL::update()
{
   // if animation is needed, update here ...
}

void RendererGL::play()
{
   if (glfwWindowShouldClose( Window )) initialize();

   setSpheres();
   ScreenObject->setSquareObject( GL_TRIANGLES, true );

   const double update_time = 0.1;
   double last = glfwGetTime(), time_delta = 0.0;
   while (!glfwWindowShouldClose( Window )) {
      const double now = glfwGetTime();
      time_delta += now - last;
      last = now;
      if (time_delta >= update_time) {
         update();
         time_delta -= update_time;
      }

      render();
      FrameIndex++;

      glfwSwapBuffers( Window );
      glfwPollEvents();
   }
   glfwDestroyWindow( Window );
}

void RendererGL::writeTexture(GLuint texture_id, int width, int height, const std::string& name)
{
   const int size = width * height * 4;
   auto* buffer = new uint8_t[size];
   glGetTextureImage( texture_id, 0, GL_BGRA, GL_UNSIGNED_BYTE, size, buffer );
   const std::string description = name.empty() ? std::string() : "(" + name + ")";
   const std::string file_name = "../" + std::to_string( texture_id ) + description + ".png";
   FIBITMAP* image = FreeImage_ConvertFromRawBits(
      buffer, width, height, width * 4, 32,
      FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, false
   );
   FreeImage_Save( FIF_PNG, image, file_name.c_str() );
   FreeImage_Unload( image );
   delete [] buffer;
}