#pragma once

#include <SDL2/SDL.h>

// Attaches a (non-owning) texture and optional frame animation to an Actor.
// The texture is owned elsewhere (e.g. Game), matching the existing convention
// of sharing one tinted texture across many actor instances (see Car::carTexture).
// Frames are equal-size regions of a single sprite sheet, addressed left-to-right,
// top-to-bottom by index; if no animation is configured the whole texture is
// drawn as a single image.
class Sprite {
public:
    Sprite() = default;
    explicit Sprite(SDL_Texture* texture) : m_texture(texture) {}

    void setTexture(SDL_Texture* texture) { m_texture = texture; }
    SDL_Texture* texture() const { return m_texture; }

    // Configures a strip of `frameCount` equal-size frames starting at (0,0),
    // wrapping to a new row after `framesPerRow` frames (0 = single row).
    void setAnimation(int frameWidth, int frameHeight, int frameCount,
                       float secondsPerFrame, int framesPerRow = 0, bool loop = true);

    // Steps the animation by dt seconds; no-op if no animation is configured.
    void advance(float dt);
    void resetAnimation();

    bool isAnimated() const { return m_frameCount > 1; }
    int currentFrame() const { return m_currentFrame; }

    // Draws the current frame centered at `center`, rotated by `angleDegrees`
    // and tinted by `colorMod`. No-op if no texture is attached.
    void render(SDL_Renderer* renderer, SDL_FPoint center, double angleDegrees = 0.0,
                SDL_Color colorMod = SDL_Color{ 255, 255, 255, 255 },
                SDL_RendererFlip flip = SDL_FLIP_NONE) const;

private:
    SDL_Texture* m_texture = nullptr;
    int m_frameWidth = 0;
    int m_frameHeight = 0;
    int m_frameCount = 1;
    int m_framesPerRow = 1;
    float m_secondsPerFrame = 0.0f;
    bool m_loop = true;
    int m_currentFrame = 0;
    float m_elapsed = 0.0f;
};
