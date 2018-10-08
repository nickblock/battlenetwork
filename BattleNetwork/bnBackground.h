#pragma once

#include "bnShaderResourceManager.h"
#include "bnTextureResourceManager.h"
#include "bnSmartShader.h"

#include <cmath>
/*
Backgrounds must fill the entire screen by repeating the texture
Some are animated and need to support texture offsets while filling
the screen
*/

class Background : public sf::Drawable, public sf::Transformable
{
protected:
  void Wrap(sf::Vector2f _amount) {
    offset = _amount;

    offset.x = std::fmod(offset.x, 1.f);
    offset.y = std::fmod(offset.y, 1.f);

  }

  void TextureOffset(sf::Vector2f _offset) {
    textureRect.left = (int)_offset.x;
    textureRect.top  = (int)_offset.y;
  }

  void FillScreen(sf::Vector2u textureSize) {
    // How many times can the texture fit in (width,height)?
    unsigned occuranceX = (unsigned)std::ceil(((float)width / (float)textureSize.x));
    unsigned occuranceY = (unsigned)std::ceil(((float)height / (float)textureSize.y));

    occuranceX = std::max(occuranceX, (unsigned)1);
    occuranceY = std::max(occuranceY, (unsigned)1);

    vertices.resize(occuranceX * occuranceY * (unsigned)4);

    textureRect = sf::IntRect(0, 0, textureSize.x, textureSize.y);

    for (unsigned int i = 0; i < occuranceX; ++i) {
      for (unsigned int j = 0; j < occuranceY; ++j) {
        // get a pointer to the current tile's quad
        sf::Vertex* quad = &vertices[(i + j * occuranceX) * 4];

        // define its 4 corners
        quad[0].position = sf::Vector2f((float)(i * textureSize.x * 2), (float)(j * textureSize.y * 2));
        quad[1].position = sf::Vector2f((float)((i + 1) * textureSize.x * 2), (float)(j * textureSize.y * 2));
        quad[2].position = sf::Vector2f((float)((i + 1) * textureSize.x * 2), (float)((j + 1) * textureSize.y * 2));
        quad[3].position = sf::Vector2f((float)(i * textureSize.x * 2), (float)((j + 1) * textureSize.y * 2));

        // define its 4 texture coordinates
        quad[0].texCoords = sf::Vector2f(0, 0);
        quad[1].texCoords = sf::Vector2f((float)textureSize.x, 0);
        quad[2].texCoords = sf::Vector2f((float)textureSize.x, (float)textureSize.y);
        quad[3].texCoords = sf::Vector2f(0, (float)textureSize.y);
      }

      textureRect = sf::IntRect(0, 0, textureSize.x, textureSize.y);
    }
  }

public:
  Background(sf::Texture& ref, int width, int height) : offset(0,0), textureRect(0, 0, width, height), width(width), height(height), texture(ref) {
      texture = ref;
      texture.setRepeated(true);
      vertices.setPrimitiveType(sf::Quads);

      sf::Vector2u textureSize = ref.getSize();

      FillScreen(textureSize);

      textureWrap = SHADERS.GetShader(ShaderType::TEXEL_TEXTURE_WRAP);
  }

  ~Background() { ;  }

  virtual void Update(float _elapsed) = 0;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
  {
    // apply the transform
    states.transform *= getTransform();

    // apply the tileset texture
    states.texture = &texture;

    sf::Vector2u size = texture.getSize();

    textureWrap->setUniform("x", (float)textureRect.left / (float)size.x);
    textureWrap->setUniform("y", (float)textureRect.top / (float)size.y);
    textureWrap->setUniform("w", (float)textureRect.width / (float)size.x);
    textureWrap->setUniform("h", (float)textureRect.height / (float)size.y);
    textureWrap->setUniform("offsetx", (float)(offset.x));
    textureWrap->setUniform("offsety", (float)(offset.y));

    states.shader = textureWrap;

    // draw the vertex array
    target.draw(vertices, states);
  }

  sf::VertexArray vertices;
  sf::Texture& texture;
  sf::IntRect textureRect;
  sf::Vector2f offset;
  int width, height;
  sf::Shader* textureWrap;
};

