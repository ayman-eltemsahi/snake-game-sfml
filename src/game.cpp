#include "game.hpp"
#include "timer.hpp"
#include <unordered_map>
#include <ctime>
#include <format>
#include <utility>
#include "config.hpp"
#include "log.hpp"

namespace snake_game {

namespace cfg = snake_game::config;
using pii = std::pair<int, int>;
using sprite_map = std::unordered_map<Direction, pii>;

std::unordered_map<Direction, Position> moves = {{UP, Position(0, -1)},
                                                 {DOWN, Position(0, 1)},
                                                 {LEFT, Position(-1, 0)},
                                                 {RIGHT, Position(1, 0)}};

sprite_map snake_head_map = {
    {UP, pii(0, 0)}, {DOWN, pii(2, 0)}, {LEFT, pii(3, 0)}, {RIGHT, pii(1, 0)}};
sprite_map snake_body_map = {
    {UP, pii(0, 3)}, {DOWN, pii(0, 3)}, {LEFT, pii(1, 3)}, {RIGHT, pii(1, 3)}};
sprite_map snake_tail_map = {
    {UP, pii(0, 1)}, {DOWN, pii(2, 1)}, {LEFT, pii(3, 1)}, {RIGHT, pii(1, 1)}};

struct Hash {
  size_t operator()(const std::pair<Position, Position> &pos) const {
    return std::hash<float>{}(pos.first.x) ^ std::hash<float>{}(pos.first.y) ^
           std::hash<float>{}(pos.second.x) ^ std::hash<float>{}(pos.second.y);
  }
};

std::unordered_map<std::pair<Position, Position>, pii, Hash> snake_turns_map = {
    {{Position(0, -1), Position(1, 0)}, pii(0, 2)},
    {{Position(1, 0), Position(0, -1)}, pii(0, 2)},
    {{Position(1, 0), Position(0, 1)}, pii(1, 2)},
    {{Position(0, 1), Position(1, 0)}, pii(1, 2)},
    {{Position(-1, 0), Position(0, 1)}, pii(2, 2)},
    {{Position(0, 1), Position(-1, 0)}, pii(2, 2)},
    {{Position(-1, 0), Position(0, -1)}, pii(3, 2)},
    {{Position(0, -1), Position(-1, 0)}, pii(3, 2)},
};

std::unordered_map<Direction, float> rotation = {{DOWN, 0}, {LEFT, 90}, {UP, 180}, {RIGHT, 270}};

bool collides(const Position &a, float a_size, const Position &b, float b_size) {
  return a.x < b.x + b_size && a.x + a_size > b.x && a.y < b.y + b_size && a.y + a_size > b.y;
}

bool isOutOfBounds(int width, int height, const Position &pos, float size) {
  return pos.x < 0 || pos.y < 0 || pos.x + size > width || pos.y + size > height;
}

Position snapPosition(const Position &pos) {
  return Position(pos.x - (int)pos.x % (int)cfg::step_size,
                  pos.y - (int)pos.y % (int)cfg::step_size);
}

Direction getDirection(sf::Keyboard::Key key, Direction cur) {
  if (key == sf::Keyboard::Up) {
    return cur == DOWN ? cur : UP;
  }

  if (key == sf::Keyboard::Down) {
    return cur == UP ? cur : DOWN;
  }

  if (key == sf::Keyboard::Left) {
    return cur == RIGHT ? cur : LEFT;
  }

  if (key == sf::Keyboard::Right) {
    return cur == LEFT ? cur : RIGHT;
  }

  return cur;
}

Game::Game(const std::string &title)
    : m_window(title),
      m_frame(0),
      m_lastRenderTime(0),
      m_target(200.0, 200.0),
      m_state(PLAYING),
      m_direction(RIGHT),
      m_lastStepTimer(Timer{}) {
  srand(time(0));
  if (!m_font.loadFromFile("../fonts/OpenSans.ttf")) {
    LOG("Error loading font");
    exit(1);
  }
  if (!m_snakeTexture.loadFromFile("../images/snake.png")) {
    LOG("Error loading snake texture");
    exit(1);
  }
  if (!m_grassTexture.loadFromFile("../images/grass.png")) {
    LOG("Error loading grass texture");
    exit(1);
  }
  if (!m_appleTexture.loadFromFile("../images/apple.png")) {
    LOG("Error loading apple texture");
    exit(1);
  }

  for (float i = 0; i < 13.0; i++) {
    m_snakeBody.push_back(Position(100.0 - i * cfg::step_size, 100.0));
  }
  randomizeTarget();
}

void Game::run() {
  while (m_window.isOpen()) {
    m_frame++;
    Timer timer;

    sf::Event event;
    sf::Keyboard::Key key = sf::Keyboard::Key::Unknown;
    while (m_window.pollEvent(event)) {
      if (event.type == sf::Event::KeyPressed) {
        key = event.key.code;
      } else if (event.type == sf::Event::Resized) {
        randomizeTarget();
      }
    }

    m_window.clear(sf::Color::Black);

    switch (m_state) {
      case PLAYING: {
        const auto step_time =
            cfg::snake_step_time - cfg::snake_step_time * m_snakeBody.size() * 0.02;

        if (m_lastStepTimer.elapsed_millis() > step_time) {
          handleKeyPress(event.key.code);
          moveSnake();
          m_lastStepTimer.reset();
        }
        drawGrass();
        drawSnake();
        drawTarget();

        break;
      }

      case LOSE: {
        drawGrass();
        drawSnake();
        drawYouLost();
        break;
      }

      default:
        break;
    }

    drawFrameTime(timer);
    m_window.display();
  }
}

void Game::handleKeyPress(sf::Keyboard::Key key) { m_direction = getDirection(key, m_direction); }

void Game::moveSnake() {
  if (m_state != PLAYING) {
    return;
  }

  Position move = moves.at(m_direction);

  Position newHead = m_snakeBody.front();
  newHead.x += move.x * cfg::step_size;
  newHead.y += move.y * cfg::step_size;

  for (const auto &bodyPart : m_snakeBody) {
    if (collides(bodyPart, cfg::step_size, newHead, cfg::step_size)) {
      m_snakeBody.push_front(newHead);
      changeState(LOSE);
      return;
    }
  }

  const auto sz = m_window.getSize();
  if (isOutOfBounds(sz.x, sz.y, newHead, cfg::step_size)) {
    changeState(LOSE);
    return;
  }

  m_snakeBody.push_front(newHead);
  if (collides(newHead, cfg::step_size, m_target, cfg::step_size)) {
    randomizeTarget();
  } else {
    m_snakeBody.pop_back();
  }
}

void Game::drawTarget() {
  m_window.texture(m_appleTexture, sf::Vector2f(m_target.x, m_target.y), true);
}

void Game::randomizeTarget() {
  const auto vp = m_window.getSize();

  m_target.x = rand() % vp.x;
  m_target.y = rand() % vp.y;

  m_target = snapPosition(m_target);
}

void Game::changeState(GameState state) {
  m_state = state;
  LOG2("Game state changed to: ", state);
}

sf::IntRect getSnakeSpritePart(const sprite_map &map, Direction dir) {
  const auto loc = map.at(dir);
  const auto sz = cfg::snake_sprite_size;
  return sf::IntRect(loc.first * sz, loc.second * sz, sz, sz);
}

sf::IntRect getSnakeSpritePart(const pii &loc) {
  const auto sz = cfg::snake_sprite_size;
  return sf::IntRect(loc.first * sz, loc.second * sz, sz, sz);
}

Direction calcDirection(const Position &cur, const Position &prev) {
  if (cur.x == prev.x) {
    return cur.y < prev.y ? DOWN : UP;
  } else {
    return cur.x < prev.x ? RIGHT : LEFT;
  }
}

Position calcDirection2(const Position &cur, const Position &prev) { return cur - prev; }

void Game::drawSnake() {
  if (m_snakeBody.empty()) {
    return;
  }

  const auto sz = cfg::snake_sprite_size;
  const auto &head = m_snakeBody.front();

  for (int i = (int)m_snakeBody.size() - 1; i >= 0; i--) {
    const auto &bodyPart = m_snakeBody[i];
    const auto &map = i == 0                        ? snake_head_map
                      : i == m_snakeBody.size() - 1 ? snake_tail_map
                                                    : snake_body_map;

    Position k1 = {0, 0};
    Position k2 = {0, 0};
    bool is_turn = false;
    if (i != 0 && i != m_snakeBody.size() - 1) {
      k1 = m_snakeBody[i + 1] - bodyPart;
      k2 = m_snakeBody[i - 1] - bodyPart;

      k1 = Position(k1.x >= 1 ? 1 : k1.x <= -1 ? -1 : 0, k1.y >= 1 ? 1 : k1.y <= -1 ? -1 : 0);
      k2 = Position(k2.x >= 1 ? 1 : k2.x <= -1 ? -1 : 0, k2.y >= 1 ? 1 : k2.y <= -1 ? -1 : 0);

      is_turn = snake_turns_map.find({k1, k2}) != snake_turns_map.end();
    }

    const sf::IntRect spritePart =
        i == 0    ? getSnakeSpritePart(map, m_direction)
        : is_turn ? getSnakeSpritePart(snake_turns_map.at({k1, k2}))
                  : getSnakeSpritePart(map, calcDirection(bodyPart, m_snakeBody[i - 1]));

    m_window.texture(m_snakeTexture, sf::Vector2f(bodyPart.x, bodyPart.y), spritePart);
  }
}

void Game::drawGrass() {
  const auto sz = m_grassTexture.getSize();
  for (int i = 0; i < m_window.getSize().x; i += sz.x) {
    for (int j = 0; j < m_window.getSize().y; j += sz.y) {
      m_window.texture(m_grassTexture, sf::Vector2f(i, j), false);
    }
  }
}

void Game::drawFrameTime(const Timer &timer) {
  if (!config::showDebugInfo) {
    return;
  }

  if (m_frame % 20 == 0) {
    m_lastRenderTime = timer.elapsed_micros();
  }

  const auto value = "frame time:  " + std::to_string(int(m_lastRenderTime)) + " micros";
  const auto fontSize = m_window.getSize().x / 40;
  const auto pos = sf::Vector2f(10.0, m_window.getSize().y - fontSize - 10);
  m_window.text(m_font, value, pos, fontSize, sf::Color::White);
}

void Game::drawYouLost() {
  const auto sz = m_window.getSize();
  m_window.text(m_font, "You Lost", sf::Vector2f(sz.x * 0.41, sz.y * 0.45), sz.x / 20,
                sf::Color::White);
}

}  // namespace snake_game
