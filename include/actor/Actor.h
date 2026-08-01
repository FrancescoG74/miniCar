#pragma once

#include <SDL3/SDL.h>

#include "Sprite.h"

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

    // Optional image/animation attached to this actor (see Sprite). Unset by
    // default, in which case the default render() below stays a no-op.
    Sprite sprite;

    // Default render draws the attached sprite (no-op if it has no texture).
    // Derived classes with bespoke visuals (procedural polygons, etc.) override
    // this instead of using `sprite`.
    virtual void render(SDL_Renderer* renderer) const { sprite.render(renderer, position); }

private:
    static int s_nextId;
    int id = 0;                                 // unique per actor, auto-assigned
    const char* name = "";                      // human-readable identifier
    SDL_Color color{ 255, 255, 255, 255 };      // primary tint / display color
    SDL_FPoint position{ 0.0f, 0.0f };          // world-space center; derived classes set/update this
};
