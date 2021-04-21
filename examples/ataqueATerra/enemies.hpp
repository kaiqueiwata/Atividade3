#ifndef ENEMIES_HPP_
#define ENEMIES_HPP_

#include <list>
#include <random>

#include "abcg.hpp"
#include "gamedata.hpp"
#include "ship.hpp"

class OpenGLWindow;

class Enemies {
 public:
  void initializeGL(GLuint program);
  void paintGL();
  void terminateGL();

  void update(GameData m_gameData, float deltaTime);

 private:
  friend OpenGLWindow;

  GLuint m_program{};
  GLint m_colorLoc{};
  GLint m_rotationLoc{};
  GLint m_translationLoc{};
  GLint m_scaleLoc{};

  int CONST_QUANTIDADE_NAVES = 14;

  float CONST_TEMPO_ZIG_ZAG = 1;
  float tempo_atual_restante = CONST_TEMPO_ZIG_ZAG;
  int sentido = +1;


  struct Enemy {
    GLuint m_vao{};
    GLuint m_vbo{};

    float m_angularVelocity{};
    glm::vec4 m_color{1};
    bool m_hit{false};
    int m_polygonSides{};
    float m_rotation{};
    float m_scale{};
    glm::vec2 m_translation{glm::vec2(0)};
    glm::vec2 m_velocity{glm::vec2(0)};
  };

  std::list<Enemy> m_enemies;

  std::default_random_engine m_randomEngine;
  std::uniform_real_distribution<float> m_randomDist{-1.0f, 1.0f};

  Enemies::Enemy createEnemy(glm::vec2 translation = glm::vec2(0));
};

#endif