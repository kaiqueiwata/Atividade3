#include "openglwindow.hpp"

#include <imgui.h>

#include <cppitertools/itertools.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "imfilebrowser.h"

void OpenGLWindow::handleEvent(SDL_Event& event) {
  if (event.type == SDL_KEYUP) {
    if(event.key.keysym.sym == SDLK_SPACE){
      if(!isJumping){
        isJumping = true;
        m_jumpSpeed = 2.0f;
      }
    }
  }
}

void OpenGLWindow::initializeGL() {
  glClearColor(0, 0, 0, 1);
  glEnable(GL_DEPTH_TEST);

  ImGuiIO& io{ImGui::GetIO()};

  houveColisao = false;
  pontos = 0;

  // configuracoes de fontes
  {
    // Fontes do titulo e mensagem de gameover/titulo
    auto ttf_gameover{getAssetsPath() + "fonts/HardsignLayered-eZ1MB.ttf"};
    auto ttf_message{getAssetsPath() +
                     "fonts/PlayfairDisplaySemibold-lg9nd.ttf"};

    // Titulo e GameOver sao as mesmas com tamanho diferente
    m_font_game_over =
        io.Fonts->AddFontFromFileTTF(ttf_gameover.c_str(), 45.0f);
    m_font_title = io.Fonts->AddFontFromFileTTF(ttf_gameover.c_str(), 65.0f);

    // A fonte de mensagem, usada pra mostrar os pontos e avisar
    // e que o jogo ta acelerando, eh uma fonte mais simples
    m_font_message = io.Fonts->AddFontFromFileTTF(ttf_message.c_str(), 25.0f);

    if (m_font_game_over == nullptr || m_font_message == nullptr ||
        m_font_title == nullptr) {
      throw abcg::Exception{abcg::Exception::Runtime("Cannot load font file")};
    }
  }

  // Create programs
  for (const auto& name : m_shaderNames) {
    auto path{getAssetsPath() + "shaders/" + name};
    auto program{createProgramFromFile(path + ".vert", path + ".frag")};
    m_programs.push_back(program);
  }

  // Load default model
  loadModel(getAssetsPath() + "/Tree_Log/one_log.obj");

  // Load cubemap
  m_model.loadCubeTexture(getAssetsPath() + "maps/cube/");

  // Initial trackball spin
  m_trackBallModel.setAxis(glm::normalize(glm::vec3(1, 1, 1)));

  // Aqui esta definindo a posicao inicial do tronco
  m_modelMatrix = glm::translate(m_modelMatrix, glm::vec3(0, 0, -1));
  m_camera.dolly(0.0f);
  initializeSkybox();
}

void OpenGLWindow::initializeSkybox() {
  // Create skybox program
  auto path{getAssetsPath() + "shaders/" + m_skyShaderName};
  m_skyProgram = createProgramFromFile(path + ".vert", path + ".frag");

  // Generate VBO
  glGenBuffers(1, &m_skyVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_skyVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(m_skyPositions), m_skyPositions.data(),
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  GLint positionAttribute{glGetAttribLocation(m_skyProgram, "inPosition")};

  // Create VAO
  glGenVertexArrays(1, &m_skyVAO);

  // Bind vertex attributes to current VAO
  glBindVertexArray(m_skyVAO);

  glBindBuffer(GL_ARRAY_BUFFER, m_skyVBO);
  glEnableVertexAttribArray(positionAttribute);
  glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // End of binding to current VAO
  glBindVertexArray(0);
}

void OpenGLWindow::loadModel(std::string_view path) {
  m_model.loadDiffuseTexture(getAssetsPath() + "maps/pattern.png");
  m_model.loadNormalTexture(getAssetsPath() + "maps/pattern_normal.png");
  m_model.loadFromFile(path);
  m_model.setupVAO(m_programs.at(m_currentProgramIndex));
  m_trianglesToDraw = m_model.getNumTriangles();

  // Use material properties from the loaded model
  m_Ka = m_model.getKa();
  m_Kd = m_model.getKd();
  m_Ks = m_model.getKs();
  m_shininess = m_model.getShininess();
}

void OpenGLWindow::paintGL() {
  update();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  // Use currently selected program
  const auto program{m_programs.at(m_currentProgramIndex)};
  glUseProgram(program);

  // Get location of uniform variables
  GLint viewMatrixLoc{glGetUniformLocation(program, "viewMatrix")};
  GLint projMatrixLoc{glGetUniformLocation(program, "projMatrix")};
  GLint modelMatrixLoc{glGetUniformLocation(program, "modelMatrix")};
  GLint normalMatrixLoc{glGetUniformLocation(program, "normalMatrix")};
  GLint lightDirLoc{glGetUniformLocation(program, "lightDirWorldSpace")};
  GLint shininessLoc{glGetUniformLocation(program, "shininess")};
  GLint IaLoc{glGetUniformLocation(program, "Ia")};
  GLint IdLoc{glGetUniformLocation(program, "Id")};
  GLint IsLoc{glGetUniformLocation(program, "Is")};
  GLint KaLoc{glGetUniformLocation(program, "Ka")};
  GLint KdLoc{glGetUniformLocation(program, "Kd")};
  GLint KsLoc{glGetUniformLocation(program, "Ks")};
  GLint diffuseTexLoc{glGetUniformLocation(program, "diffuseTex")};
  GLint normalTexLoc{glGetUniformLocation(program, "normalTex")};
  GLint cubeTexLoc{glGetUniformLocation(program, "cubeTex")};
  GLint mappingModeLoc{glGetUniformLocation(program, "mappingMode")};
  GLint texMatrixLoc{glGetUniformLocation(program, "texMatrix")};

  // Set uniform variables used by every scene object
  glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_camera.m_viewMatrix[0][0]);
  glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_camera.m_projMatrix[0][0]);
  glUniform1i(diffuseTexLoc, 0);
  glUniform1i(normalTexLoc, 1);
  glUniform1i(cubeTexLoc, 2);
  glUniform1i(mappingModeLoc, m_mappingMode);

  glm::mat3 texMatrix{m_trackBallLight.getRotation()};
  glUniformMatrix3fv(texMatrixLoc, 1, GL_TRUE, &texMatrix[0][0]);

  auto lightDirRotated{m_trackBallLight.getRotation() * m_lightDir};
  glUniform4fv(lightDirLoc, 1, &lightDirRotated.x);
  glUniform4fv(IaLoc, 1, &m_Ia.x);
  glUniform4fv(IdLoc, 1, &m_Id.x);
  glUniform4fv(IsLoc, 1, &m_Is.x);

  // Set uniform variables of the current object
  glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &m_modelMatrix[0][0]);

  auto modelViewMatrix{glm::mat3(m_camera.m_viewMatrix * m_modelMatrix)};
  glm::mat3 normalMatrix{glm::inverseTranspose(modelViewMatrix)};
  glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);

  glUniform1f(shininessLoc, m_shininess);
  glUniform4fv(KaLoc, 1, &m_Ka.x);
  glUniform4fv(KdLoc, 1, &m_Kd.x);
  glUniform4fv(KsLoc, 1, &m_Ks.x);
  m_model.render(m_trianglesToDraw);

  if (m_currentProgramIndex == 0 || m_currentProgramIndex == 1) {
    renderSkybox();
  }
}

void OpenGLWindow::renderSkybox() {
  glUseProgram(m_skyProgram);

  // Get location of uniform variables
  GLint viewMatrixLoc{glGetUniformLocation(m_skyProgram, "viewMatrix")};
  GLint projMatrixLoc{glGetUniformLocation(m_skyProgram, "projMatrix")};
  GLint skyTexLoc{glGetUniformLocation(m_skyProgram, "skyTex")};

  // Set uniform variables
  auto viewMatrix{m_trackBallLight.getRotation()};
  glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &viewMatrix[0][0]);
  glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_projMatrix[0][0]);
  glUniform1i(skyTexLoc, 0);

  glBindVertexArray(m_skyVAO);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_model.getCubeTexture());

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CW);
  glDepthFunc(GL_LEQUAL);
  glDrawArrays(GL_TRIANGLES, 0, m_skyPositions.size());
  glDepthFunc(GL_LESS);

  glBindVertexArray(0);
  glUseProgram(0);
}

void OpenGLWindow::paintUI() {
  // File browser for models
  

  





  // Create main window widget
  {
    auto widgetSize{ImVec2(222, 190)};

    if (!m_model.isUVMapped()) {
      // Add extra space for static text
      widgetSize.y += 26;
    }

    auto flagsDisplayText{
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration};
    ImGui::Begin("Widget window", nullptr, flagsDisplayText);

 

    // Aviso de gameover

  

    if (houveColisao) {
      auto game_over_size{ImVec2(400, 200)};
      // auto game_over_position{ImVec2((m_viewportWidth - game_over_size.x)
      // / 2.0f,
      //                       (m_viewportHeight - game_over_size.y) / 2.0f)};
      auto game_over_position{ImVec2(0, 200)};

      ImGui::SetNextWindowPos(game_over_position);
      ImGui::SetNextWindowSize(game_over_size);
      ImGui::Begin(" ", nullptr, flagsDisplayText);
      ImGui::PushFont(m_font_game_over);

      ImGui::Text("Fim de Jogo!");
      ImGui::Text("   %d pts", pontos);

      ImGui::PopFont();
      ImGui::End();
    } else {
        // Titulo do jogo

    auto title_size{ImVec2(600, 200)};
    auto title_position{ImVec2(0, 0)};

    ImGui::SetNextWindowPos(title_position);
    ImGui::SetNextWindowSize(title_size);
    ImGui::Begin(" ", nullptr, flagsDisplayText);
    ImGui::PushFont(m_font_title);

    ImGui::Text("The Tree Log Challenge!");

    ImGui::PopFont();
    ImGui::PushFont(m_font_message);

    ImGui::Text("Jump [SPACE]");

    ImGui::PopFont();
    ImGui::End();

    


    

    if (acelerar) {
      auto size{ImVec2(340, 180)};
      auto position{ImVec2(0, 0)};

      ImGui::SetNextWindowPos(position);
      ImGui::SetNextWindowSize(size);
      ImGui::Begin(" ", nullptr, flagsDisplayText);
      ImGui::PushFont(m_font_message);

      ImGui::Text("%d pontos!\nAcelerando 20%%", pontos);

      ImGui::PopFont();
      ImGui::End();
      }
    }

    // A cada cinco pontos acumulados ele exibe a mensagem de que esta
    // acelerando
    
    // Slider will be stretched horizontally


    // CW/CCW combo box
    {
      static std::size_t currentIndex{};
     

      ImGui::PushItemWidth(120);
    
      ImGui::PopItemWidth();

      if (currentIndex == 0) {
        glFrontFace(GL_CCW);
      } else {
        glFrontFace(GL_CW);
      }
    }

    // Projection combo box
   

    // Shader combo box
 
   

    ImGui::End();
  }

  



 

 
}

void OpenGLWindow::resizeGL(int width, int height) {
  m_viewportWidth = width;
  m_viewportHeight = height;

  // m_trackBallModel.resizeViewport(width, height);
  // m_trackBallLight.resizeViewport(width, height);

  m_camera.computeProjectionMatrix(width, height);
}

void OpenGLWindow::terminateGL() {
  for (const auto& program : m_programs) {
    glDeleteProgram(program);
  }
  terminateSkybox();
}

void OpenGLWindow::terminateSkybox() {
  glDeleteProgram(m_skyProgram);
  glDeleteBuffers(1, &m_skyVBO);
  glDeleteVertexArrays(1, &m_skyVAO);
}

void OpenGLWindow::translateModel(float speed) {
  glm::vec3 m_forward{glm::vec3(0.0f, 0.0f, 1.0f)};

  // Vetor unitario da direcao X velocidade X fator de aceleracao
  m_forward = m_forward * speed * m_logSpeedFactor;
  m_modelMatrix = glm::translate(m_modelMatrix, m_forward);
}

//Leva o modelo de volta a posicao inicial
void OpenGLWindow::resetModelPosition(){
  m_modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -2.5f));
}

void OpenGLWindow::update() {
  float deltaTime{static_cast<float>(getDeltaTime())};

  elapsedTime += deltaTime;
  
  if(isJumping){
    m_camera.jump(m_jumpSpeed * deltaTime * m_jumpSpeedFactor);  

    if (m_camera.m_at.y > 0.49f) {
      m_jumpSpeed -= 3 * deltaTime;
    }
    else {
      isJumping = false;
      m_jumpSpeed = 0;
      m_camera.m_at.y = 0.5f;
    }
  }

  if (getZPos(m_modelMatrix) < 2.5f) {
    translateModel(m_LogSpeed * deltaTime);
  } else {
    if (!houveColisao) {
      pontos++;
    }
    resetModelPosition();
    elapsedTime = 0.0f;
  }

  if (pontos > 0 && pontos % 5 == 0 && !acelerar) {
    // incrementa a velocidade em 20% a cada 5 troncos pulados
    m_logSpeedFactor += 0.2f;
    m_jumpSpeedFactor += 0.2f;

    acelerar = true;
  }

  if (acelerar) {
    elapsedMsgTimer += deltaTime;

    if (elapsedMsgTimer >= displayMsgTimer) {
      acelerar = false;
      elapsedMsgTimer = 0.0f;
    }
  }

  checkCollisions();

  if (houveColisao) {
    restartTimer -= deltaTime;
    if (restartTimer <= 0) {
      restart();
    }
  }
}

void OpenGLWindow::checkCollisions() {
  float zModel = getZPos(m_modelMatrix);
  // houve colisao: o tronco da arvore esta proximo e o personagem nao esta
  // pulando
  houveColisao =
      houveColisao || ((zModel >= +2.1f && zModel <= 2.5f) && !isJumping);
}

float OpenGLWindow::getZPos(glm::mat4 matrix) { return matrix[3][2]; }

void OpenGLWindow::restart() {
  pontos = 0;
  houveColisao = false;
  resetModelPosition();
  restartTimer = 3.0f;
  elapsedMsgTimer = 0.0f;
  m_logSpeedFactor = 1.0f;
  m_jumpSpeedFactor = 1.0f;
}