#include "canvas.h"
   
CanvasGL::CanvasGL() : CanvasID( 0 ), COLOR0TextureID( 0 ), StencilTextureID( 0 )
{
}

CanvasGL::~CanvasGL()
{
   deleteAllTextures();
}

void CanvasGL::deleteAllTextures()
{
   if (COLOR0TextureID != 0) {
      glDeleteTextures( 1, &COLOR0TextureID );
      COLOR0TextureID = 0;
   }
   if (StencilTextureID != 0) {
      glDeleteTextures( 1, &StencilTextureID );
      StencilTextureID = 0;
   }
   if (CanvasID != 0) {
      glDeleteFramebuffers( 1, &CanvasID );
      CanvasID = 0;
   }
}

void CanvasGL::setCanvas(int width, int height, GLenum format, bool use_stencil)
{
   deleteAllTextures();

   glCreateTextures( GL_TEXTURE_2D, 1, &COLOR0TextureID );
   glTextureStorage2D( COLOR0TextureID, 1, format, width, height );
   glTextureParameteri( COLOR0TextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
   glTextureParameteri( COLOR0TextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTextureParameteri( COLOR0TextureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
   glTextureParameteri( COLOR0TextureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );

   glCreateFramebuffers( 1, &CanvasID );
   glNamedFramebufferTexture( CanvasID, GL_COLOR_ATTACHMENT0, COLOR0TextureID, 0 );

   if (use_stencil) {
      glCreateTextures( GL_TEXTURE_2D, 1, &StencilTextureID );
      glTextureStorage2D( StencilTextureID, 1, GL_STENCIL_INDEX8, width, height );
      glTextureParameteri( StencilTextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
      glTextureParameteri( StencilTextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTextureParameteri( StencilTextureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
      glTextureParameteri( StencilTextureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );

      glNamedFramebufferTexture( CanvasID, GL_STENCIL_ATTACHMENT, StencilTextureID, 0 );
   }

   glCheckNamedFramebufferStatus( CanvasID, GL_FRAMEBUFFER );
}

void CanvasGL::setMultiSampledCanvas(int width, int height, int sample_num, GLenum format, bool use_stencil)
{
   deleteAllTextures();

   glCreateTextures( GL_TEXTURE_2D_MULTISAMPLE, 1, &COLOR0TextureID );
   glTextureStorage2DMultisample( COLOR0TextureID, sample_num, format, width, height, GL_TRUE );

   glCreateFramebuffers( 1, &CanvasID );
   glNamedFramebufferTexture( CanvasID, GL_COLOR_ATTACHMENT0, COLOR0TextureID, 0 );

   if (use_stencil) {
      glCreateTextures( GL_TEXTURE_2D_MULTISAMPLE, 1, &StencilTextureID );
      glTextureStorage2DMultisample(
         StencilTextureID,
         sample_num,
         GL_STENCIL_INDEX8,
         width,
         height,
         GL_TRUE
      );

      glNamedFramebufferTexture( CanvasID, GL_STENCIL_ATTACHMENT, StencilTextureID, 0 );
   }

   glCheckNamedFramebufferStatus( CanvasID, GL_FRAMEBUFFER );
}

void CanvasGL::clearColor(int buffer_index) const
{
   constexpr std::array<GLfloat, 4> clear_color = {
      0.0f,
      0.0f,
      0.0f,
      0.0f
   };
   glClearNamedFramebufferfv( CanvasID, GL_COLOR, buffer_index, &clear_color[0] );
}

void CanvasGL::clearColor(const std::array<GLfloat, 4>& color, int buffer_index) const
{
   glClearNamedFramebufferfv( CanvasID, GL_COLOR, buffer_index, &color[0] );
}

void CanvasGL::clearStencil() const
{
   constexpr GLint zero = 0;
   glClearNamedFramebufferiv( CanvasID, GL_STENCIL, 0, &zero );
}

void CanvasGL::clearDepth() const
{
   constexpr GLfloat zero = 1.0f;
   glClearNamedFramebufferfv( CanvasID, GL_DEPTH, 0, &zero );
}