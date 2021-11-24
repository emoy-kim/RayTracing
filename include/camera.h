#pragma once

#include "base.h"

class CameraGL
{
public:
   explicit CameraGL(float near_plane = -1.0f, float far_plane = 1.0f);

   [[nodiscard]] const glm::mat4& getProjectionMatrix() const { return ProjectionMatrix; }
   void updateWindowSize(int width, int height);

private:
   int Width;
   int Height;
   float NearPlane;
   float FarPlane;
   glm::mat4 ProjectionMatrix;
};