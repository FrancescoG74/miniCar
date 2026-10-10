#include "game/TouchControls.h"

#include <cmath>
#include <vector>

namespace {

constexpr int kCircleSegments = 24;
// Button radius as a fraction of the logical window height, kept large
// enough for comfortable thumbs on a phone screen.
constexpr float kButtonRadiusFraction = 0.11f;

SDL_FColor toFColor(SDL_Color c) {
    return { c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f };
}

void fillCircle(SDL_Renderer* renderer, SDL_FPoint center, float radius, SDL_Color color) {
    SDL_FColor fc = toFColor(color);
    std::vector<SDL_Vertex> verts;
    verts.reserve(kCircleSegments + 2);
    verts.push_back(SDL_Vertex{ center, fc, { 0, 0 } });
    for (int i = 0; i <= kCircleSegments; ++i) {
        float t = static_cast<float>(i) / kCircleSegments * 2.0f * static_cast<float>(M_PI);
        SDL_FPoint p{ center.x + radius * std::cos(t), center.y + radius * std::sin(t) };
        verts.push_back(SDL_Vertex{ p, fc, { 0, 0 } });
    }
    std::vector<int> indices;
    indices.reserve(static_cast<std::size_t>(kCircleSegments) * 3);
    for (int i = 1; i <= kCircleSegments; ++i) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }
    SDL_RenderGeometry(renderer, nullptr, verts.data(), static_cast<int>(verts.size()),
                       indices.data(), static_cast<int>(indices.size()));
}

void fillTriangle(SDL_Renderer* renderer, SDL_FPoint a, SDL_FPoint b, SDL_FPoint c, SDL_Color color) {
    SDL_FColor fc = toFColor(color);
    SDL_Vertex verts[3] = {
        { a, fc, { 0, 0 } },
        { b, fc, { 0, 0 } },
        { c, fc, { 0, 0 } },
    };
    int indices[3] = { 0, 1, 2 };
    SDL_RenderGeometry(renderer, nullptr, verts, 3, indices, 3);
}

void strokeCircle(SDL_Renderer* renderer, SDL_FPoint center, float radius, SDL_Color color) {
    SDL_FPoint pts[kCircleSegments + 1];
    for (int i = 0; i <= kCircleSegments; ++i) {
        float t = static_cast<float>(i) / kCircleSegments * 2.0f * static_cast<float>(M_PI);
        pts[i] = { center.x + radius * std::cos(t), center.y + radius * std::sin(t) };
    }
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderLines(renderer, pts, kCircleSegments + 1);
}

} // namespace

void TouchControls::layout(int windowWidth, int windowHeight) {
    m_radius = static_cast<float>(windowHeight) * kButtonRadiusFraction;
    const float margin = m_radius * 1.5f;
    const float gap = m_radius * 2.3f;
    const float y = static_cast<float>(windowHeight) - margin;

    m_leftCenter = { margin, y };
    m_rightCenter = { margin + gap, y };

    m_brakeCenter = { static_cast<float>(windowWidth) - margin - gap, y };
    m_accelCenter = { static_cast<float>(windowWidth) - margin, y };
}

TouchControls::Button TouchControls::hitTest(float x, float y) const {
    auto within = [&](SDL_FPoint c) {
        float dx = x - c.x;
        float dy = y - c.y;
        return dx * dx + dy * dy <= m_radius * m_radius;
    };
    if (within(m_leftCenter)) return Button::Left;
    if (within(m_rightCenter)) return Button::Right;
    if (within(m_accelCenter)) return Button::Accelerate;
    if (within(m_brakeCenter)) return Button::Brake;
    return Button::None;
}

void TouchControls::setButton(Button button, bool pressed) {
    switch (button) {
    case Button::Left: m_left = pressed; break;
    case Button::Right: m_right = pressed; break;
    case Button::Accelerate: m_accelerate = pressed; break;
    case Button::Brake: m_brake = pressed; break;
    default: break;
    }
}

void TouchControls::handleEvent(const SDL_Event& event) {
    if (!enabled) return;

    switch (event.type) {
    case SDL_EVENT_FINGER_DOWN: {
        Button button = hitTest(event.tfinger.x, event.tfinger.y);
        if (button != Button::None) {
            m_fingers[event.tfinger.fingerID] = button;
            setButton(button, true);
        }
        break;
    }
    case SDL_EVENT_FINGER_MOTION: {
        // Lets a finger slide from one pedal/steer button to another without
        // lifting, which is how real touch controllers are expected to feel.
        Button newButton = hitTest(event.tfinger.x, event.tfinger.y);
        auto it = m_fingers.find(event.tfinger.fingerID);
        if (it != m_fingers.end()) {
            if (it->second != newButton) {
                setButton(it->second, false);
                if (newButton != Button::None) {
                    it->second = newButton;
                    setButton(newButton, true);
                } else {
                    m_fingers.erase(it);
                }
            }
        } else if (newButton != Button::None) {
            m_fingers[event.tfinger.fingerID] = newButton;
            setButton(newButton, true);
        }
        break;
    }
    case SDL_EVENT_FINGER_UP:
    case SDL_EVENT_FINGER_CANCELED: {
        auto it = m_fingers.find(event.tfinger.fingerID);
        if (it != m_fingers.end()) {
            setButton(it->second, false);
            m_fingers.erase(it);
        }
        break;
    }
    default:
        break;
    }
}

void TouchControls::render(SDL_Renderer* renderer) const {
    if (!enabled) return;

    SDL_BlendMode prevMode = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(renderer, &prevMode);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    auto drawButton = [&](SDL_FPoint center, bool pressed, SDL_Color base) {
        // Dark backing disc first so the button reads clearly against the
        // bright green grass, regardless of `base`'s own brightness.
        fillCircle(renderer, center, m_radius * 1.08f, SDL_Color{ 0, 0, 0, 110 });
        SDL_Color fill = base;
        fill.a = pressed ? 235 : 170;
        fillCircle(renderer, center, m_radius, fill);
        strokeCircle(renderer, center, m_radius, SDL_Color{ 255, 255, 255, 230 });
    };

    drawButton(m_leftCenter, m_left, SDL_Color{ 60, 60, 60, 255 });
    drawButton(m_rightCenter, m_right, SDL_Color{ 60, 60, 60, 255 });
    drawButton(m_accelCenter, m_accelerate, SDL_Color{ 40, 150, 40, 255 });
    drawButton(m_brakeCenter, m_brake, SDL_Color{ 170, 40, 40, 255 });

    const SDL_Color arrow{ 255, 255, 255, 230 };
    const float r = m_radius * 0.45f;

    fillTriangle(renderer,
                 { m_leftCenter.x - r, m_leftCenter.y },
                 { m_leftCenter.x + r * 0.6f, m_leftCenter.y - r * 0.8f },
                 { m_leftCenter.x + r * 0.6f, m_leftCenter.y + r * 0.8f },
                 arrow);
    fillTriangle(renderer,
                 { m_rightCenter.x + r, m_rightCenter.y },
                 { m_rightCenter.x - r * 0.6f, m_rightCenter.y - r * 0.8f },
                 { m_rightCenter.x - r * 0.6f, m_rightCenter.y + r * 0.8f },
                 arrow);
    fillTriangle(renderer,
                 { m_accelCenter.x, m_accelCenter.y - r },
                 { m_accelCenter.x - r * 0.8f, m_accelCenter.y + r * 0.6f },
                 { m_accelCenter.x + r * 0.8f, m_accelCenter.y + r * 0.6f },
                 arrow);
    fillTriangle(renderer,
                 { m_brakeCenter.x, m_brakeCenter.y + r },
                 { m_brakeCenter.x - r * 0.8f, m_brakeCenter.y - r * 0.6f },
                 { m_brakeCenter.x + r * 0.8f, m_brakeCenter.y - r * 0.6f },
                 arrow);

    SDL_SetRenderDrawBlendMode(renderer, prevMode);
}
