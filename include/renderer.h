/*
 * Author: Emoy Kim
 * E-mail: emoy.kim_AT_gmail.com
 * 
 * This code is a free software; it can be freely used, changed and redistributed.
 * If you use any version of the code, please reference the code.
 * 
 */

#pragma once

#include "canvas.h"
#include "object.h"

class RendererGL
{
public:
   RendererGL(const RendererGL&) = delete;
   RendererGL(const RendererGL&&) = delete;
   RendererGL& operator=(const RendererGL&) = delete;
   RendererGL& operator=(const RendererGL&&) = delete;


   RendererGL();
   ~RendererGL() = default;

   void play();

private:
   inline static RendererGL* Renderer = nullptr;
   GLFWwindow* Window;
   int FrameWidth;
   int FrameHeight;
   int FrameIndex;
   glm::ivec2 ClickedPoint;
   std::vector<Sphere> Spheres;
   std::unique_ptr<CameraGL> MainCamera;
   std::unique_ptr<ShaderGL> Shader;
   std::unique_ptr<ShaderGL> ScreenShader;
   std::unique_ptr<ObjectGL> ScreenObject;
   std::unique_ptr<CanvasGL> FinalCanvas;

   void registerCallbacks() const;
   void initialize();

   static void printOpenGLInformation();

   static void cleanup(GLFWwindow* window);
   static void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
   void reshape(GLFWwindow* window, int width, int height) const;
   static void cleanupWrapper(GLFWwindow* window) { RendererGL::cleanup( window ); }
   static void keyboardWrapper(GLFWwindow* window, int key, int scancode, int action, int mods)
   {
      RendererGL::keyboard( window, key, scancode, action, mods );
   }
   static void reshapeWrapper(GLFWwindow* window, int width, int height) { Renderer->reshape( window, width, height ); }

   void setSpheres();
   void drawScene() const;
   void drawScreen() const;
   void render() const;
   void update();
   static void writeTexture(GLuint texture_id, int width, int height, const std::string& name = {});

   // 16 and 32 do well, anything in between or below is bad.
   // 32 seems to do well on laptop/desktop Windows Intel and on NVidia/AMD as well.
   // further hardware-specific tuning might be needed for optimal performance.
   inline static constexpr int ThreadGroupSize = 32;
   [[nodiscard]] static int getGroupSize(int size)
   {
      return (size + ThreadGroupSize - 1) / ThreadGroupSize;
   }
};