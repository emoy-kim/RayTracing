#pragma once

#include "base.h"
#include "camera.h"

class ShaderGL
{
public:
   struct SphereLocationSet
   {
      GLint Center;
      GLint Radius;

      SphereLocationSet() : Center( 0 ), Radius( 0 ) {}
   };

   struct LocationSet
   {
      GLint ModelViewProjection, SphereNum;
      std::map<GLint, GLint> Texture; // <binding point, texture id>
      std::vector<SphereLocationSet> Spheres;

      LocationSet() : ModelViewProjection( 0 ), SphereNum( 0 ) {}
   };

   ShaderGL();
   virtual ~ShaderGL();

   void setShader(
      const char* vertex_shader_path,
      const char* fragment_shader_path,
      const char* geometry_shader_path = nullptr,
      const char* tessellation_control_shader_path = nullptr,
      const char* tessellation_evaluation_shader_path = nullptr
   );
   void setComputeShader(const char* compute_shader_path);
   void setRayUniformLocations();
   void setScreenUniformLocations();
   void addUniformLocation(const std::string& name)
   {
      CustomLocations[name] = glGetUniformLocation( ShaderProgram, name.c_str() );
   }
   void transferBasicTransformationUniforms(const glm::mat4& to_world, const CameraGL* camera, bool use_texture = false) const;
   void transferShpereUniformsToShader(int sphere_num);
   void uniform1i(const char* name, int value) const
   {
      glProgramUniform1i( ShaderProgram, CustomLocations.find( name )->second, value );
   }
   void uniform1f(const char* name, float value) const
   {
      glProgramUniform1f( ShaderProgram, CustomLocations.find( name )->second, value );
   }
   void uniform1fv(const char* name, int count, const float* value) const
   {
      glProgramUniform1fv( ShaderProgram, CustomLocations.find( name )->second, count, value );
   }
   void uniform2fv(const char* name, const glm::vec2& value) const
   {
      glProgramUniform2fv( ShaderProgram, CustomLocations.find( name )->second, 1, &value[0] );
   }
   void uniform2fv(const char* name, int count, const float* value) const
   {
      glProgramUniform2fv( ShaderProgram, CustomLocations.find( name )->second, count, value );
   }
   void uniform3fv(const char* name, const glm::vec3& value) const
   {
      glProgramUniform3fv( ShaderProgram, CustomLocations.find( name )->second, 1, &value[0] );
   }
   void uniform4fv(const char* name, const glm::vec4& value) const
   {
      glProgramUniform4fv( ShaderProgram, CustomLocations.find( name )->second, 1, &value[0] );
   }
   void uniformMat3fv(const char* name, const glm::mat3& value) const
   {
      glProgramUniformMatrix3fv( ShaderProgram, CustomLocations.find( name )->second, 1, GL_FALSE, &value[0][0] );
   }
   void uniformMat4fv(const char* name, const glm::mat4& value) const
   {
      glProgramUniformMatrix4fv( ShaderProgram, CustomLocations.find( name )->second, 1, GL_FALSE, &value[0][0] );
   }
   [[nodiscard]] GLuint getShaderProgram() const { return ShaderProgram; }
   [[nodiscard]] GLint getLocation(const std::string& name) const { return CustomLocations.find( name )->second; }

protected:
   GLuint ShaderProgram;
   LocationSet Location;
   std::unordered_map<std::string, GLint> CustomLocations;

   static void readShaderFile(std::string& shader_contents, const char* shader_path);
   [[nodiscard]] static std::string getShaderTypeString(GLenum shader_type);
   [[nodiscard]] static bool checkCompileError(GLenum shader_type, const GLuint& shader);
   [[nodiscard]] static GLuint getCompiledShader(GLenum shader_type, const char* shader_path);
};