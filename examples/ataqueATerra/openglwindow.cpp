#include "openglwindow.hpp"

#include <imgui.h>

#include "abcg.hpp"

void OpenGLWindow::handleEvent(SDL_Event &event) {
  // Keyboard events
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_SPACE)
      m_gameData.m_input.set(static_cast<size_t>(Input::Fire));
    if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
      m_gameData.m_input.set(static_cast<size_t>(Input::Up));
    if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
      m_gameData.m_input.set(static_cast<size_t>(Input::Down));
    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
      m_gameData.m_input.set(static_cast<size_t>(Input::Left));
    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
      m_gameData.m_input.set(static_cast<size_t>(Input::Right));
  }
  if (event.type == SDL_KEYUP) {
    if (event.key.keysym.sym == SDLK_SPACE)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Fire));
    if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Up));
    if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Down));
    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Left));
    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Right));
  }

  // Mouse events
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    if (event.button.button == SDL_BUTTON_LEFT)
      m_gameData.m_input.set(static_cast<size_t>(Input::Fire));
    if (event.button.button == SDL_BUTTON_RIGHT)
      m_gameData.m_input.set(static_cast<size_t>(Input::Up));
  }
  if (event.type == SDL_MOUSEBUTTONUP) {
    if (event.button.button == SDL_BUTTON_LEFT)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Fire));
    if (event.button.button == SDL_BUTTON_RIGHT)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Up));
  }
}

void OpenGLWindow::initializeGL() {
  // Load a new font
  ImGuiIO &io{ImGui::GetIO()};

  //letra da pontuação é menor
  auto filenameAquire{getAssetsPath() + "AquireBold-8Ma60.otf"};
  m_font_pts = io.Fonts->AddFontFromFileTTF(filenameAquire.c_str(), 30.0f);
  if (m_font_pts == nullptr) {
    throw abcg::Exception{abcg::Exception::Runtime("Cannot load font file")};
  }
 
  //letra do game over é maior
  m_font_game_over = io.Fonts->AddFontFromFileTTF(filenameAquire.c_str(), 45.0f);
  if (m_font_pts == nullptr) {
    throw abcg::Exception{abcg::Exception::Runtime("Cannot load font file")};
  }

  // Create program to render the stars
  m_starsProgram = createProgramFromFile(getAssetsPath() + "stars.vert",
                                         getAssetsPath() + "stars.frag");
  // Create program to render the other objects
  m_objectsProgram = createProgramFromFile(getAssetsPath() + "objects.vert",
                                           getAssetsPath() + "objects.frag");

  glClearColor(0, 0, 0, 1);

#if !defined(__EMSCRIPTEN__)
  glEnable(GL_PROGRAM_POINT_SIZE);
#endif

  // Start pseudo-random number generator
  auto seed{std::chrono::steady_clock::now().time_since_epoch().count()};
  m_randomEngine.seed(seed);

  restart();
}

void OpenGLWindow::restart() {
  m_gameData.m_state = State::Playing;

  m_starLayers.initializeGL(m_starsProgram, 25);
  m_ship.initializeGL(m_objectsProgram);
  m_asteroids.initializeGL(m_objectsProgram);
  m_bullets.initializeGL(m_objectsProgram);
}

void OpenGLWindow::update() {
  float deltaTime{static_cast<float>(getDeltaTime())};

  // Wait 5 seconds before restarting
  if (m_gameData.m_state != State::Playing &&
      m_restartWaitTimer.elapsed() > 5) {
    restart();
    return;
  }

  m_ship.update(m_gameData, deltaTime);
  m_starLayers.update(m_ship, deltaTime);
  m_asteroids.update(m_gameData, deltaTime);
  m_bullets.update(m_ship, m_gameData, deltaTime);

  if (m_gameData.m_state == State::Playing) {
    checkCollisions();
    checkWinCondition();
  }
}

void OpenGLWindow::paintGL() {
  update();

  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  m_starLayers.paintGL();
  m_asteroids.paintGL();
  m_bullets.paintGL();
  m_ship.paintGL(m_gameData);
}

void OpenGLWindow::paintUI() {
  abcg::OpenGLWindow::paintUI();

  {

    ImGuiWindowFlags flags{ImGuiWindowFlags_NoBackground |
                            ImGuiWindowFlags_NoTitleBar |
                            ImGuiWindowFlags_NoInputs};
    if (m_gameData.m_state != State::GameOver) {

      auto tamanhoDisplayPontuacao{ImVec2(150, 50)};
      auto posicaoDisplayPontuacao{
          ImVec2(m_viewportWidth - tamanhoDisplayPontuacao.x - 5,
                 m_viewportHeight - tamanhoDisplayPontuacao.y - 5)};

      ImGui::SetNextWindowPos(posicaoDisplayPontuacao);
      ImGui::SetNextWindowSize(tamanhoDisplayPontuacao);
      ImGui::Begin(" ", nullptr, flags);

      ImGui::PushFont(m_font_pts);
      ImGui::Text("%d pts", m_gameData.PONTOS);
      ImGui::PopFont();
      ImGui::End();
    } 
    else {

      auto size{ImVec2(400, 85)};
      auto position{ImVec2((m_viewportWidth - size.x) / 2.0f,
                           (m_viewportHeight - size.y) / 2.0f)};

      ImGui::SetNextWindowPos(position);
      ImGui::SetNextWindowSize(size);

      ImGui::Begin(" ", nullptr, flags);
      ImGui::PushFont(m_font_game_over);

      if (m_gameData.m_state == State::GameOver) {
        ImGui::Text("Fim de Jogo!");
      }

      ImGui::PopFont();
      ImGui::End();
    }
  }
}

void OpenGLWindow::resizeGL(int width, int height) {
  m_viewportWidth = width;
  m_viewportHeight = height;

  glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLWindow::terminateGL() {
  glDeleteProgram(m_starsProgram);
  glDeleteProgram(m_objectsProgram);

  m_asteroids.terminateGL();
  m_bullets.terminateGL();
  m_ship.terminateGL();
  m_starLayers.terminateGL();
}

void OpenGLWindow::checkCollisions() {
  // Check collision between ship and asteroids
  for (auto &asteroid : m_asteroids.m_enemies) {
    auto asteroidTranslation{asteroid.m_translation};
    auto distance{glm::distance(m_ship.m_translation, asteroidTranslation)};

    if (distance < m_ship.m_scale * 0.9f + asteroid.m_scale * 3.0f) {
      m_gameData.m_state = State::GameOver;
      m_gameData.PONTOS = 0;
      m_restartWaitTimer.restart();
    }
  }

  // Check collision between bullets and asteroids
  for (auto &bullet : m_bullets.m_bullets) {
    if (bullet.m_dead) continue;

    for (auto &asteroid : m_asteroids.m_enemies) {
      for (auto i : {-2, 0, 2}) {
        for (auto j : {-2, 0, 2}) {
          auto asteroidTranslation{asteroid.m_translation + glm::vec2(i, j)};
          auto distance{
              glm::distance(bullet.m_translation, asteroidTranslation)};

          if (distance < m_bullets.m_scale + asteroid.m_scale * 3.0f) {
            asteroid.m_hit = true;
            bullet.m_dead = true;
            m_gameData.PONTOS++;
          }
        }
      }
    }

    m_asteroids.m_enemies.remove_if(
        [](const Enemies::Enemy &a) { return a.m_hit; });
  }
}

void OpenGLWindow::checkWinCondition() {
  if (m_asteroids.m_enemies.empty()) {
    m_gameData.fator_vel_jogo += 0.1f;
    m_asteroids.initializeGL(m_objectsProgram);
    m_restartWaitTimer.restart();
  }
}