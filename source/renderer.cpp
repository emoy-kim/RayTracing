#include "renderer.h"

RendererGL::RendererGL() : 
   Window( nullptr ), FrameWidth( 2000 ), FrameHeight( 1000 ), ClickedPoint( -1, -1 ),
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

void RendererGL::error(int e, const char* description)
{
   std::ignore = e;
   puts( description );
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
      case GLFW_KEY_UP:
         MainCamera->moveForward();
         break;
      case GLFW_KEY_DOWN:
         MainCamera->moveBackward();
         break;
      case GLFW_KEY_LEFT:
         MainCamera->moveLeft();
         break;
      case GLFW_KEY_RIGHT:
         MainCamera->moveRight();
         break;
      case GLFW_KEY_W:
         MainCamera->moveUp();
         break;
      case GLFW_KEY_S:
         MainCamera->moveDown();
         break;
      case GLFW_KEY_I:
         MainCamera->resetCamera();
         break;
      case GLFW_KEY_SPACE:
         break;
      case GLFW_KEY_P: {
         const glm::vec3 pos = MainCamera->getCameraPosition();
         std::cout << "Camera Position: " << pos.x << ", " << pos.y << ", " << pos.z << "\n";
      } break;
      case GLFW_KEY_Q:
      case GLFW_KEY_ESCAPE:
         cleanupWrapper( window );
         break;
      default:
         return;
   }
}

void RendererGL::cursor(GLFWwindow* window, double xpos, double ypos)
{
   if (MainCamera->getMovingState()) {
      const auto x = static_cast<int>(round( xpos ));
      const auto y = static_cast<int>(round( ypos ));
      const int dx = x - ClickedPoint.x;
      const int dy = y - ClickedPoint.y;
      MainCamera->moveForward( -dy );
      MainCamera->rotateAroundWorldY( -dx );

      if (glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT ) == GLFW_PRESS) {
         MainCamera->pitch( -dy );
      }

      ClickedPoint.x = x;
      ClickedPoint.y = y;
   }
}

void RendererGL::mouse(GLFWwindow* window, int button, int action, int mods)
{
   std::ignore = mods;
   if (button == GLFW_MOUSE_BUTTON_LEFT) {
      const bool moving_state = action == GLFW_PRESS;
      if (moving_state) {
         double x, y;
         glfwGetCursorPos( window, &x, &y );
         ClickedPoint.x = static_cast<int>(round( x ));
         ClickedPoint.y = static_cast<int>(round( y ));
      }
      MainCamera->setMovingState( moving_state );
   }
}

void RendererGL::mousewheel(GLFWwindow* window, double xoffset, double yoffset) const
{
   std::ignore = window;
   std::ignore = xoffset;
   if (yoffset >= 0.0) MainCamera->zoomIn();
   else MainCamera->zoomOut();
}

void RendererGL::reshape(GLFWwindow* window, int width, int height) const
{
   std::ignore = window;
   MainCamera->updateWindowSize( width, height );
   glViewport( 0, 0, width, height );
}

void RendererGL::registerCallbacks() const
{
   glfwSetErrorCallback( errorWrapper );
   glfwSetWindowCloseCallback( Window, cleanupWrapper );
   glfwSetKeyCallback( Window, keyboardWrapper );
   glfwSetCursorPosCallback( Window, cursorWrapper );
   glfwSetMouseButtonCallback( Window, mouseWrapper );
   glfwSetScrollCallback( Window, mousewheelWrapper );
   glfwSetFramebufferSizeCallback( Window, reshapeWrapper );
}

void RendererGL::drawScene() const
{
   glUseProgram( Shader->getShaderProgram() );
   Shader->transferShpereUniformsToShader( 2 );
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

}

void RendererGL::play()
{
   if (glfwWindowShouldClose( Window )) initialize();

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