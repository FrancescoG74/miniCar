#include "Sprite.h"

#include <algorithm>

void Sprite::setAnimation(int frameWidth, int frameHeight, int frameCount,
                          float secondsPerFrame, int framesPerRow, bool loop) {
    m_frameWidth = frameWidth;
    m_frameHeight = frameHeight;
    m_frameCount = std::max(1, frameCount);
    m_framesPerRow = framesPerRow > 0 ? framesPerRow : m_frameCount;
    m_secondsPerFrame = secondsPerFrame;
    m_loop = loop;
    m_currentFrame = 0;
    m_elapsed = 0.0f;
}

void Sprite::advance(float dt) {
    if (m_frameCount <= 1 || m_secondsPerFrame <= 0.0f) return;

    m_elapsed += dt;
    while (m_elapsed >= m_secondsPerFrame) {
        if (m_currentFrame + 1 < m_frameCount) {
            ++m_currentFrame;
            m_elapsed -= m_secondsPerFrame;
        } else if (m_loop) {
            m_currentFrame = 0;
            m_elapsed -= m_secondsPerFrame;
        } else {
            m_elapsed = 0.0f; // clamp: hold on the last frame
        }
    }
}

void Sprite::resetAnimation() {
    m_currentFrame = 0;
    m_elapsed = 0.0f;
}

void Sprite::render(SDL_Renderer* renderer, SDL_FPoint center, double angleDegrees,
                    SDL_Color colorMod, SDL_FlipMode flip) const {
    if (!renderer || !m_texture) return;

    SDL_FRect srcRect;
    const SDL_FRect* src = nullptr;
    float width = static_cast<float>(m_frameWidth);
    float height = static_cast<float>(m_frameHeight);
    if (m_frameWidth > 0 && m_frameHeight > 0) {
        srcRect = {
            static_cast<float>((m_currentFrame % m_framesPerRow) * m_frameWidth),
            static_cast<float>((m_currentFrame / m_framesPerRow) * m_frameHeight),
            static_cast<float>(m_frameWidth),
            static_cast<float>(m_frameHeight)
        };
        src = &srcRect;
    } else {
        SDL_GetTextureSize(m_texture, &width, &height);
    }

    SDL_FRect dst{
        center.x - width / 2.0f,
        center.y - height / 2.0f,
        width, height
    };

    SDL_SetTextureColorMod(m_texture, colorMod.r, colorMod.g, colorMod.b);
    SDL_SetTextureAlphaMod(m_texture, colorMod.a);
    SDL_RenderTextureRotated(renderer, m_texture, src, &dst, angleDegrees, nullptr, flip);
}
