#pragma once

#include <SDL3/SDL.h>
#include <vector>

// A point on the track centerline: position plus direction of travel (radians, 0 = +x).
struct TrackPoint {
    float x;
    float y;
    float angle;
};

// A closed circuit sampled by advancing an arc-length parameter `s`.
class Track {
public:
    Track(float centerX, float centerY, float straightLength, float radius, float width);
    Track(float centerX, float centerY, float width, std::vector<SDL_FPoint> controlPoints);

    float totalLength() const { return m_totalLength; }

    // Returns the centerline position/heading at arc-length `s` (wraps automatically).
    TrackPoint sample(float s) const;

    void render(SDL_Renderer* renderer) const;

private:
    struct PathSample {
        float distance;
        float x;
        float y;
        float angle;
    };

    float m_centerX;
    float m_centerY;
    float m_straightLength;
    float m_radius;
    float m_width;
    float m_totalLength;
    std::vector<PathSample> m_path;
};
