#include "shader.h"

ShaderGL::ShaderGL() : ShaderProgram( 0 )
{
}

ShaderGL::~ShaderGL()
{
   if (ShaderProgram != 0) glDeleteProgram( ShaderProgram );
}

void ShaderGL::readShaderFile(std::string& shader_contents, const char* shader_path)
{
   std::ifstream file( shader_path, std::ios::in );
   if (!file.is_open()) {
      std::cerr << "Cannot open shader file: " << shader_path << "\n";
      return;
   }

   std::string line;
   while (!file.eof()) {
      getline( file, line );
      shader_contents.append( line + "\n" );
   }
   file.close();
}

std::string ShaderGL::getShaderTypeString(GLenum shader_type)
{
   switch (shader_type) {
      case GL_VERTEX_SHADER: return "Vertex Shader";
      case GL_FRAGMENT_SHADER: return "Fragment Shader";
      case GL_GEOMETRY_SHADER: return "Geometry Shader";
      case GL_TESS_CONTROL_SHADER: return "Tessellation Control Shader";
      case GL_TESS_EVALUATION_SHADER: return "Tessellation Evaluation Shader";
      case GL_COMPUTE_SHADER: return "Compute Shader";
      default: return "";
   }
}

bool ShaderGL::checkCompileError(GLenum shader_type, const GLuint& shader)
{
   GLint compiled = 0;
   glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );

   if (compiled == GL_FALSE) {
      GLint max_length = 0;
      glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &max_length );

      std::cerr << " ======= " << getShaderTypeString( shader_type ) << " log ======= \n";
      std::vector<GLchar> error_log(max_length);
      glGetShaderInfoLog( shader, max_length, &max_length, &error_log[0] );
      for (const auto& c : error_log) std::cerr << c;
      std::cerr << "\n";
      glDeleteShader( shader );
   }
   return compiled == GL_TRUE;
}

GLuint ShaderGL::getCompiledShader(GLenum shader_type, const char* shader_path)
{
   if (shader_path == nullptr) return 0;

   std::string shader_contents;
   readShaderFile( shader_contents, shader_path );

   const GLuint shader = glCreateShader( shader_type );
   const char* shader_source = shader_contents.c_str();
   glShaderSource( shader, 1, &shader_source, nullptr );
   glCompileShader( shader );
   if (!checkCompileError( shader_type, shader )) {
      std::cerr << "Could not compile shader\n";
      return 0;
   }
   return shader;
}

void ShaderGL::setShader(
   const char* vertex_shader_path,
   const char* fragment_shader_path,
   const char* geometry_shader_path,
   const char* tessellation_control_shader_path,
   const char* tessellation_evaluation_shader_path
)
{
   const GLuint vertex_shader = getCompiledShader( GL_VERTEX_SHADER, vertex_shader_path );
   const GLuint fragment_shader = getCompiledShader( GL_FRAGMENT_SHADER, fragment_shader_path );
   const GLuint geometry_shader = getCompiledShader( GL_GEOMETRY_SHADER, geometry_shader_path );
   const GLuint tessellation_control_shader = getCompiledShader( GL_TESS_CONTROL_SHADER, tessellation_control_shader_path );
   const GLuint tessellation_evaluation_shader = getCompiledShader( GL_TESS_EVALUATION_SHADER, tessellation_evaluation_shader_path );
   ShaderProgram = glCreateProgram();
   glAttachShader( ShaderProgram, vertex_shader );
   glAttachShader( ShaderProgram, fragment_shader );
   if (geometry_shader != 0) glAttachShader( ShaderProgram, geometry_shader );
   if (tessellation_control_shader != 0) glAttachShader( ShaderProgram, tessellation_control_shader );
   if (tessellation_evaluation_shader != 0) glAttachShader( ShaderProgram, tessellation_evaluation_shader );
   glLinkProgram( ShaderProgram );
   glDeleteShader( vertex_shader );
   glDeleteShader( fragment_shader );
   if (geometry_shader != 0) glDeleteShader( geometry_shader );
   if (tessellation_control_shader != 0) glDeleteShader( tessellation_control_shader );
   if (tessellation_evaluation_shader != 0) glDeleteShader( tessellation_evaluation_shader );
}

void ShaderGL::setComputeShader(const char* compute_shader_path)
{
   const GLuint compute_shader = getCompiledShader( GL_COMPUTE_SHADER, compute_shader_path );
   ShaderProgram = glCreateProgram();
   glAttachShader( ShaderProgram, compute_shader );
   glLinkProgram( ShaderProgram );
   glDeleteShader( compute_shader );
}

void ShaderGL::setRayUniformLocations()
{
   addUniformLocation( "FrameIndex" );

   Location.SphereNum = glGetUniformLocation( ShaderProgram, "SphereNum" );

   Location.Spheres.resize( 32 );
   for (int i = 0; i < static_cast<int>(Location.Spheres.size()); ++i) {
      Location.Spheres[i].Type = glGetUniformLocation( ShaderProgram, std::string("Sphere[" + std::to_string( i ) + "].Type").c_str() );
      Location.Spheres[i].Center = glGetUniformLocation( ShaderProgram, std::string("Sphere[" + std::to_string( i ) + "].Center").c_str() );
      Location.Spheres[i].Radius = glGetUniformLocation( ShaderProgram, std::string("Sphere[" + std::to_string( i ) + "].Radius").c_str() );
      Location.Spheres[i].Albedo = glGetUniformLocation( ShaderProgram, std::string("Sphere[" + std::to_string( i ) + "].Albedo").c_str() );
   }
}

void ShaderGL::setScreenUniformLocations()
{
   Location.ModelViewProjection = glGetUniformLocation( ShaderProgram, "ModelViewProjectionMatrix" );
   Location.Texture[0] = glGetUniformLocation( ShaderProgram, "BaseTexture" );
}

void ShaderGL::transferBasicTransformationUniforms(const glm::mat4& to_world, const CameraGL* camera, bool use_texture) const
{
   const glm::mat4 projection = camera->getProjectionMatrix();
   const glm::mat4 model_view_projection = projection * to_world;
   glProgramUniformMatrix4fv( ShaderProgram, Location.ModelViewProjection, 1, GL_FALSE, &model_view_projection[0][0] );

   for (const auto& texture : Location.Texture) {
      glProgramUniform1i( ShaderProgram, texture.second, texture.first );
   }
}

void ShaderGL::transferSphereUniformsToShader(const std::vector<Sphere>& spheres)
{
   const auto size = static_cast<int>(spheres.size());
   glProgramUniform1i( ShaderProgram, Location.SphereNum, size );

   for (int i = 0; i < size; ++i) {
      glProgramUniform1i( ShaderProgram, Location.Spheres[i].Type, static_cast<int>(spheres[i].Type) );
      glProgramUniform1f( ShaderProgram, Location.Spheres[i].Radius, spheres[i].Radius );
      glProgramUniform3fv( ShaderProgram, Location.Spheres[i].Center, 1, &spheres[i].Center[0] );
      glProgramUniform3fv( ShaderProgram, Location.Spheres[i].Albedo, 1, &spheres[i].Albedo[0] );
   }
}