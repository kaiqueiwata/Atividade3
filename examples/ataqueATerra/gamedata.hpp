#ifndef GAMEDATA_HPP_
#define GAMEDATA_HPP_

#include <bitset>

enum class Input { Right, Left, Down, Up, Fire };
enum class State { Playing, GameOver };


struct GameData {
  int PONTOS;
  float fator_vel_jogo = 0.2f;
  State m_state{State::Playing};
  std::bitset<5> m_input;  // [fire, up, down, left, right]
};

#endif