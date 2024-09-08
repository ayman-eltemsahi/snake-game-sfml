#pragma once
#include <SFML/Graphics.hpp>
#include "config.hpp"
#include "log.hpp"

namespace snake_game {

class WindowManager {
 public:
  WindowManager(const std::string& title)
      : m_window(
            sf::RenderWindow(sf::VideoMode(config::window_width, config::window_height), title)) {
    m_window.setFramerateLimit(120);
  }

  void clear(const sf::Color& color) { m_window.clear(color); }
  bool isOpen() const { return m_window.isOpen(); }
  void display() { m_window.display(); }
  void close() { m_window.close(); }
  bool pollEvent(sf::Event& event) {
    const auto value = m_window.pollEvent(event);

    if (value) {
      if (event.type == sf::Event::Closed) {
        m_window.close();
      } else if (event.type == sf::Event::Resized) {
        m_window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
      }
    }

    return value;
  }

  sf::Vector2u getSize() const { return m_window.getSize(); }
  void draw(const sf::Drawable& drawable) { m_window.draw(drawable); }

  void texture(const sf::Texture& texture, const sf::Vector2f& pos, bool scale) {
    sf::Sprite sprite;
    sprite.setTexture(texture);
    sprite.setPosition(pos);

    if (scale) {
      sprite.setScale(config::step_size / texture.getSize().x,
                      config::step_size / texture.getSize().y);
    }
    m_window.draw(sprite);
  }

  void texture(const sf::Texture& texture, const sf::Vector2f& pos, const sf::IntRect& rectangle) {
    sf::Sprite sprite;
    sprite.setTexture(texture);
    sprite.setTextureRect(rectangle);
    sprite.setPosition(pos);
    sprite.setScale(config::step_size / rectangle.width, config::step_size / rectangle.height);
    m_window.draw(sprite);
  }

  void text(const sf::Font& font, const std::string& value, const sf::Vector2f& pos, float size,
            const sf::Color& color) {
    sf::Text text;
    text.setFont(font);
    text.setString(value);
    text.setCharacterSize(size);
    text.setFillColor(color);
    text.setPosition(pos);

    m_window.draw(text);
  }

 private:
  sf::RenderWindow m_window;
};

}  // namespace snake_game
