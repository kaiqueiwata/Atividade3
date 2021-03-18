#include "ship.hpp"

#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/rotate_vector.hpp>

void Ship::initializeGL(GLuint program) {
  terminateGL();

  m_program = program;
  m_colorLoc = glGetUniformLocation(m_program, "color");
  m_rotationLoc = glGetUniformLocation(m_program, "rotation");
  m_scaleLoc = glGetUniformLocation(m_program, "scale");
  m_translationLoc = glGetUniformLocation(m_program, "translation");

  m_rotation = 0.0f;
  m_translation = glm::vec2(0);
  m_translation.y = -0.8; // Posiciona nave em baixo
  m_velocity = glm::vec2(0);


  // clang-format off
  std::array<glm::vec2, 29> positions{

      // Ship body
      glm::vec2{+00.0f, +14.5f}, glm::vec2{-01.8f, +06.4f}, //1,2
      glm::vec2{-04.8f, +05.0f}, glm::vec2{-04.8f, +01.6f}, //3,4
      glm::vec2{-14.6f, -04.0f}, glm::vec2{-14.6f, -07.4f}, //5,6
      glm::vec2{-06.4f, -09.4f}, glm::vec2{-7.2f, -10.0f},  //7,8
      glm::vec2{-07.2f, -12.0f}, glm::vec2{+7.2f, -12.0f},  //9,10
      glm::vec2{+07.2f, -10.0f}, glm::vec2{+06.4f, -09.4f}, //11,12
      glm::vec2{+14.6f, -07.4f}, glm::vec2{+14.6f, -04.0f}, //13,14
      glm::vec2{+04.8f, +01.6f}, glm::vec2{+04.8f, +05.0f}, //15,16
      glm::vec2{+01.8f, +06.4f}, glm::vec2{+00.0f, +05.0f}, //17,18
      glm::vec2{+00.0f, -04.0f}, glm::vec2{+00.0f, -07.4f}, //19,20
      glm::vec2{+00.0f, -10.0f},                            //21

      // Cannon left
      glm::vec2{-11.0f, +0.0f}, glm::vec2{-9.0f, +0.0f},    //22,23
      glm::vec2{-11.0f, -2.0f}, glm::vec2{-9.0f, -2.0f},    //24,25

      // Cannon right
      glm::vec2{+11.0f, +0.0f}, glm::vec2{+9.0f, +0.0f},    //26,27
      glm::vec2{+11.0f, -2.0f}, glm::vec2{+9.0f, -2.0f},    //28,29

      };

  // Normalize
  for (auto &position :positions) {
    position /= glm::vec2{15.5f, 15.5f};
  }

  std::array indices{
                    //Ship
                     0, 1, 16,
                     1, 2, 17,
                     15, 16, 17,
                     1, 16, 17,
                     2, 3, 14,
                     2, 14, 15,
                     3, 4, 18,
                     3, 14, 18,
                     13, 14, 18,
                     4, 5, 13,
                     12, 13, 4,
                     5, 6, 19,
                     19, 12, 11,
                     6, 19, 11,
                     5, 12, 13,
                     6, 7, 20,
                     6, 20, 11,
                     10, 11, 20,
                     7, 8, 10,
                     10, 9, 7,
                    //Canons
                     21, 22, 23,
                     24, 23, 22,
                     25, 26, 27,
                     27, 28, 26,
       
                   
                     };
  // clang-format on

  // Generate VBO
  glGenBuffers(1, &m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions.data(),
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate EBO
  glGenBuffers(1, &m_ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(),
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  GLint positionAttribute{glGetAttribLocation(m_program, "inPosition")};

  // Create VAO
  glGenVertexArrays(1, &m_vao);

  // Bind vertex attributes to current VAO
  glBindVertexArray(m_vao);

  glEnableVertexAttribArray(positionAttribute);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

  // End of binding to current VAO
  glBindVertexArray(0);
}

void Ship::paintGL(const GameData &gameData) {
  if (gameData.m_state != State::Playing) return;

  glUseProgram(m_program);

  glBindVertexArray(m_vao);

  glUniform1f(m_scaleLoc, m_scale);
  glUniform1f(m_rotationLoc, m_rotation);
  glUniform2fv(m_translationLoc, 1, &m_translation.x);

  // Restart thruster blink timer every 100 ms
  if (m_trailBlinkTimer.elapsed() > 100.0 / 1000.0) m_trailBlinkTimer.restart();

  if (gameData.m_input[static_cast<size_t>(Input::Up)]) {
    // Show thruster trail during 50 ms
    if (m_trailBlinkTimer.elapsed() < 50.0 / 1000.0) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      // 50% transparent
      glUniform4f(m_colorLoc, 1, 1, 1, 0.5f);

      glDrawElements(GL_TRIANGLES, 24 * 3, GL_UNSIGNED_INT, nullptr);

      glDisable(GL_BLEND);
    }
  }

  glUniform4fv(m_colorLoc, 1, &m_color.r);
  glDrawElements(GL_TRIANGLES, 24 * 3, GL_UNSIGNED_INT, nullptr);

  glBindVertexArray(0);

  glUseProgram(0);
}

void Ship::terminateGL() {
  glDeleteBuffers(1, &m_vbo);
  glDeleteBuffers(1, &m_ebo);
  glDeleteVertexArrays(1, &m_vao);
}

void Ship::update(const GameData &gameData, float deltaTime) {
  // Horizontal Translate 
  if (m_translation.x >= -0.87){ 
    if (gameData.m_input[static_cast<size_t>(Input::Left)])
     m_translation.x -= 1.5 * deltaTime;
  }

  if (m_translation.x <= +0.87){
    if (gameData.m_input[static_cast<size_t>(Input::Right)])
      m_translation.x += 1.5 * deltaTime;
  }
  // Apply thrust
  if (gameData.m_input[static_cast<size_t>(Input::Up)] &&
      gameData.m_state == State::Playing) {
    // Thrust in the forward vector
    glm::vec2 forward = glm::rotate(glm::vec2{0.0f, 1.0f}, m_rotation);
    m_velocity += forward * deltaTime;
  }
}
