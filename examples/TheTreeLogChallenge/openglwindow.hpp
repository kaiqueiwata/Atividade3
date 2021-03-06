#ifndef OPENGLWINDOW_HPP_
#define OPENGLWINDOW_HPP_

#include <string_view>
#include <imgui.h>

#include "abcg.hpp"
#include "model.hpp"
#include "trackball.hpp"
#include "camera.hpp"

class OpenGLWindow : public abcg::OpenGLWindow {
 protected:
  void handleEvent(SDL_Event& ev) override;
  void initializeGL() override;
  void paintGL() override;
  void paintUI() override;
  void resizeGL(int width, int height) override;
  void terminateGL() override;
  float m_jumpSpeed{0.0f};
  bool isJumping;


 private:
  int m_viewportWidth{};
  int m_viewportHeight{};

  Model m_model;
  int m_trianglesToDraw{};

  Camera m_camera;
  float m_dollySpeed{0.0f};
  float m_truckSpeed{0.0f};
  float m_panSpeed{0.0f};

  //velocidade base do Tronco
  float m_LogSpeed{1.0f};

  //Fator de aceleracao do pulo
  float m_jumpSpeedFactor{1.0f};

  //usado para pular quando soltar o espaco 
  bool jumpButtonPressed = false;

  //fator de velocidade incremental do tronco
  float m_logSpeedFactor{1.2f};
  //booleano para indicar quando se deve acelerar o jogo
  bool acelerar;

  //variavel que vai indicar se houver colisao entre o personagem e o tronco
  bool houveColisao;
  //pontuacao: cada arvore saltada ganha um ponto
  int pontos;

  TrackBall m_trackBallModel;
  TrackBall m_trackBallLight;

  float m_zoom{};

  glm::mat4 m_modelMatrix{1.0f};
  glm::mat4 m_projMatrix{1.0f};

  //Fonte do game over
  ImFont* m_font_game_over{};

  //Fonte do game over
  ImFont* m_font_title{};

  //Fonte de mensagem que sera exibida a cada 5 pontos 
  ImFont* m_font_message{};


  //TIMERS:
  //Tempo necessario para resetar a posicao da tronco
  float timer= 3.5f;
  //tempo decorrido desde o reset
  float elapsedTime = 0.0f;

  //Tempo para mostrar o fim de jogo e a pontuacao final e entao reiniciar o jogo
  float restartTimer = 3.0f;

  //timer para mostrar mensagens na tela
  float displayMsgTimer = 0.7f;
  //Tempo decorrido da mensagem
  float elapsedMsgTimer = 0.0f;
  // Shaders
  const std::vector<const char*> m_shaderNames{
      "texture"};
  std::vector<GLuint> m_programs;
  int m_currentProgramIndex{};

  // Mapping mode
  // 0: triplanar; 1: cylindrical; 2: spherical; 3: from mesh
  int m_mappingMode{3};

  // Light and material properties
  glm::vec4 m_lightDir{-1.0f, -1.0f, -1.0f, 0.0f};
  glm::vec4 m_Ia{1.0f};
  glm::vec4 m_Id{1.0f};
  glm::vec4 m_Is{1.0f};
  glm::vec4 m_Ka;
  glm::vec4 m_Kd;
  glm::vec4 m_Ks;
  float m_shininess{};

  // Skybox
  const std::string m_skyShaderName{"skybox"};
  GLuint m_skyVAO{};
  GLuint m_skyVBO{};
  GLuint m_skyProgram{};

  // clang-format off
  const std::array<glm::vec3, 36>  m_skyPositions{
    // Front
    glm::vec3{-1, -1, +1}, glm::vec3{+1, -1, +1}, glm::vec3{+1, +1, +1},
    glm::vec3{-1, -1, +1}, glm::vec3{+1, +1, +1}, glm::vec3{-1, +1, +1},
    // Back
    glm::vec3{+1, -1, -1}, glm::vec3{-1, -1, -1}, glm::vec3{-1, +1, -1},
    glm::vec3{+1, -1, -1}, glm::vec3{-1, +1, -1}, glm::vec3{+1, +1, -1},
    // Right
    glm::vec3{+1, -1, -1}, glm::vec3{+1, +1, -1}, glm::vec3{+1, +1, +1},
    glm::vec3{+1, -1, -1}, glm::vec3{+1, +1, +1}, glm::vec3{+1, -1, +1},
    // Left
    glm::vec3{-1, -1, +1}, glm::vec3{-1, +1, +1}, glm::vec3{-1, +1, -1},
    glm::vec3{-1, -1, +1}, glm::vec3{-1, +1, -1}, glm::vec3{-1, -1, -1},
    // Top
    glm::vec3{-1, +1, +1}, glm::vec3{+1, +1, +1}, glm::vec3{+1, +1, -1},
    glm::vec3{-1, +1, +1}, glm::vec3{+1, +1, -1}, glm::vec3{-1, +1, -1},
    // Bottom
    glm::vec3{-1, -1, -1}, glm::vec3{+1, -1, -1}, glm::vec3{+1, -1, +1},
    glm::vec3{-1, -1, -1}, glm::vec3{+1, -1, +1}, glm::vec3{-1, -1, +1}
  };
  // clang-format on

  void initializeSkybox();
  void renderSkybox();
  void terminateSkybox();
  void loadModel(std::string_view path);
  void update();
  void translateModel(float speed);
  void resetModelPosition();
  void checkCollisions();
  void restart();
  float getZPos(glm::mat4 matrix);
};

#endif