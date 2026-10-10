#pragma once

#include <SDL3/SDL.h>
#include <unordered_map>

// On-screen virtual gamepad for touchscreens: steering left/right and
// accelerate/brake buttons that emulate Player 1's WASD keys. Enabled
// automatically when a touch device is detected (always true on Android);
// a harmless no-op otherwise. All coordinates are in the renderer's
// logical/design space (see SDL_SetRenderLogicalPresentation in Setup.cpp),
// so the buttons stay correctly placed regardless of the physical screen
// resolution.
class TouchControls {
public:
    // Positions the four buttons relative to the logical window size. Call
    // once at startup (Game::init already does this).
    void layout(int windowWidth, int windowHeight);

    // Feeds SDL_EVENT_FINGER_DOWN/MOTION/UP/CANCELED events into the button
    // hit-test state machine. Expects render (logical) coordinates -- call
    // SDL_ConvertEventToRenderCoordinates() on the event first.
    void handleEvent(const SDL_Event& event);

    // Draws the four translucent buttons. No-op if `enabled` is false.
    void render(SDL_Renderer* renderer) const;

    bool accelerate() const { return m_accelerate; }
    bool brake() const { return m_brake; }
    bool left() const { return m_left; }
    bool right() const { return m_right; }

    // True on touch-capable devices; false disables both rendering and
    // hit-testing so desktop keyboard-only play is unaffected.
    bool enabled = false;

private:
    enum class Button { None, Left, Right, Accelerate, Brake };

    Button hitTest(float x, float y) const;
    void setButton(Button button, bool pressed);

    SDL_FPoint m_leftCenter{ 0, 0 };
    SDL_FPoint m_rightCenter{ 0, 0 };
    SDL_FPoint m_accelCenter{ 0, 0 };
    SDL_FPoint m_brakeCenter{ 0, 0 };
    float m_radius = 0.0f;

    bool m_left = false;
    bool m_right = false;
    bool m_accelerate = false;
    bool m_brake = false;

    // Tracks which button each active finger currently holds down, so
    // lifting one finger doesn't cancel a button held by another finger.
    std::unordered_map<SDL_FingerID, Button> m_fingers;
};
