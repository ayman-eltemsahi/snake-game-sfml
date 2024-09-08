#pragma once
#include <SFML/Graphics.hpp>
#include "timer.hpp"
#include "window-manager.hpp"
#include <deque>

namespace snake_game {

enum Direction { UP, DOWN, LEFT, RIGHT };
enum GameState { PLAYING, WIN, LOSE };

struct Position {
  Position(int x, int y) : x(x), y(y) {}
  int x;
  int y;

  Position(const Position &other) : x(other.x), y(other.y) {}
  Position(Position &&other) noexcept : x(other.x), y(other.y) {
    other.x = 0;
    other.y = 0;
  }

  bool operator==(const Position &other) const { return x == other.x && y == other.y; }

  Position &operator=(const Position &other) {
    if (this == &other) {
      return *this;
    }
    x = other.x;
    y = other.y;
    return *this;
  }

  friend Position operator+(const Position &a, const Position &b) {
    return Position{a.x + b.x, a.y + b.y};
  }
  friend Position operator-(const Position &a, const Position &b) {
    return Position{a.x - b.x, a.y - b.y};
  }
};

class Game {
 public:
  Game(const std::string &title);
  void run();

 private:
  void handleKeyPress(sf::Keyboard::Key key);
  void moveSnake();
  void randomizeTarget();
  void changeState(GameState state);

  void drawSnake();
  void drawGrass();
  void drawTarget();
  void drawYouLost();
  void drawFrameTime(const Timer &timer);

 private:
  GameState m_state;
  Direction m_direction;
  Position m_target;
  std::deque<Position> m_snakeBody;
  WindowManager m_window;
  int64_t m_frame;
  sf::Font m_font;
  sf::Texture m_snakeTexture;
  sf::Texture m_grassTexture;
  sf::Texture m_appleTexture;

  Timer m_lastStepTimer;

  float m_lastRenderTime;
};

}  // namespace snake_game
