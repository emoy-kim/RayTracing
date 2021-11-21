#pragma once

#include "base.h"

class CanvasGL final
{
public:
   CanvasGL();
   ~CanvasGL();

   [[nodiscard]] GLuint getCanvasID() const { return CanvasID; }
   [[nodiscard]] GLuint getColor0TextureID() const { return COLOR0TextureID; }
   void setCanvas(int width, int height, GLenum format, bool use_stencil = false);
   void setMultiSampledCanvas(int width, int height, int sample_num, GLenum format, bool use_stencil = false);
   void clearColor(int buffer_index = 0) const;
   void clearColor(const std::array<GLfloat, 4>& color, int buffer_index = 0) const;
   void clearColor(const glm::vec4& color, int buffer_index = 0) const
   {
      clearColor( std::array<GLfloat, 4>{ color.r, color.g, color.b, color.a }, buffer_index );
   }
   void clearStencil() const;
   void clearDepth() const;

private:
   GLuint CanvasID;
   GLuint COLOR0TextureID;
   GLuint StencilTextureID;

   void deleteAllTextures();
};

class ImageGL final
{
public:
   explicit ImageGL(int format = GL_RGBA8) : Type( 0 ), Format( format ), TextureID( 0 )
   {
      switch (Format) {
         case GL_RGBA8: Type = GL_UNSIGNED_BYTE; break;
         case GL_RGBA16F: Type = GL_HALF_FLOAT; break;
         case GL_RGBA32F: Type = GL_FLOAT; break;
         default: break;
      }
   }
   ~ImageGL() { if (TextureID != 0) glDeleteTextures( 1, &TextureID ); }

   [[nodiscard]] GLuint getTextureID() const { return TextureID; }
   void create(int width, int height)
   {
      if (TextureID != 0) return;
      glCreateTextures( GL_TEXTURE_2D, 1, &TextureID );
      glTextureStorage2D( TextureID, 1, Format, width, height );
   }
   void deleteTexture()
   {
      if (TextureID != 0) {
         glDeleteTextures( 1, &TextureID );
         TextureID = 0;
      }
   }
   void setFilters() const
   {
      glTextureParameteri( TextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
      glTextureParameteri( TextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   }
   void setWraps(GLint wrap_parameter) const
   {
      glTextureParameteri( TextureID, GL_TEXTURE_WRAP_S, wrap_parameter );
      glTextureParameteri( TextureID, GL_TEXTURE_WRAP_T, wrap_parameter );
   }
   void clear() const
   {
      constexpr GLuint clear_color[4] = { 0, 0, 0, 0 };
      glClearTexImage( TextureID, 0, GL_RGBA, Type, clear_color );
   }
   void clear(const void* color) const { glClearTexImage( TextureID, 0, GL_RGBA, Type, color ); }

private:
   uint Type;
   int Format;
   GLuint TextureID;
};