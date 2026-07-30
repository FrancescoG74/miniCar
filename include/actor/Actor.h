#pragma once

#include <SDL2/SDL.h>

// Base class for anything that lives in the game world (cars for now; obstacles,
// power-ups, checkpoints, etc. can plug in later). Keeps the shared identity /
// state fields common to all game objects and gives them a polymorphic render hook.
class Actor {
public:
    bool active = true;                         // false = actor should be ignored

    Actor();
    Actor(const char* name, SDL_Color color);
    virtual ~Actor() = default;

    // Identity accessors. `id` is auto-assigned at construction and immutable.
    int getId() const { return id; }
    const char* getName() const { return name; }
    void setName(const char* n) { name = n; }
    const SDL_Color& getColor() const { return color; }
    void setColor(SDL_Color c) { color = c; }

    // World-space center accessors. Derived classes (or systems) mutate the
    // position through setPosition; readers use getPosition.
    const SDL_FPoint& getPosition() const { return position; }
    void setPosition(SDL_FPoint p) { position = p; }

    // Default no-op render; derived classes may override to draw themselves.
    // Kept virtual (rather than pure) so a caller can hold generic Actor*s and
    // safely call render() even on actors without a bespoke visual.
    virtual void render(SDL_Renderer*) const {}

private:
    static int s_nextId;
    int id = 0;                                 // unique per actor, auto-assigned
    const char* name = "";                      // human-readable identifier
    SDL_Color color{ 255, 255, 255, 255 };      // primary tint / display color
    SDL_FPoint position{ 0.0f, 0.0f };          // world-space center; derived classes set/update this
};
