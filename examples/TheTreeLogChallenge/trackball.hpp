#ifndef TRACKBALL_HPP_
#define TRACKBALL_HPP_

#include "abcg.hpp"

class TrackBall {
 public:
  void mouseMove(const glm::ivec2& mousePosition);
  void mousePress(const glm::ivec2& mousePosition);
  void mouseRelease(const glm::ivec2& mousePosition);
  void resizeViewport(int width, int height);

  [[nodiscard]] glm::mat4 getRotation();
  [[nodiscard]] glm::mat4 getTranslation(glm::mat4 mat, float speed);

  void setAxis(glm::vec3 axis) { m_axis = axis; }
  void setVelocity(float velocity) { m_velocity = velocity; }
  float elapsedTime = 0.0f;

 private:
  const float m_maxVelocity{glm::radians(720.0f / 1000.0f)};

  glm::vec3 m_axis{0.0f, 1.0f, 0.0f};
  glm::vec3 m_position{-1.0f};
  float m_velocity{};
  glm::mat4 m_rotation{glm::rotate(glm::mat4(1.0f), 270.0f, m_axis)};
 
  
  
  glm::vec3 m_lastPosition{};
  abcg::ElapsedTimer m_lastTime{};
  bool m_mouseTracking{};

  float m_viewportWidth{};
  float m_viewportHeight{};

  [[nodiscard]] glm::vec3 project(const glm::vec2& mousePosition) const;
};

#endif