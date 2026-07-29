#pragma once

#include <SDL2/SDL.h>

// A point on the track centerline: position plus direction of travel (radians, 0 = +x).
struct TrackPoint {
    float x;
    float y;
    float angle;
};

// A closed "stadium" shaped circuit: two straight sections connected by two
// semicircular turns. Cars move along it by advancing an arc-length parameter `s`.
class Track {
public:
    Track(float centerX, float centerY, float straightLength, float radius, float width);

    float totalLength() const { return m_totalLength; }

    // Returns the centerline position/heading at arc-length `s` (wraps automatically).
    TrackPoint sample(float s) const;

    void render(SDL_Renderer* renderer) const;

private:
    float m_centerX;
    float m_centerY;
    float m_straightLength;
    float m_radius;
    float m_width;
    float m_totalLength;
};
