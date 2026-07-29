#include "Track.h"

#include <cmath>
#include <vector>

namespace {
constexpr int kSegments = 240;
}

Track::Track(float centerX, float centerY, float straightLength, float radius, float width)
    : m_centerX(centerX), m_centerY(centerY),
      m_straightLength(straightLength), m_radius(radius), m_width(width) {
    m_totalLength = 2.0f * m_straightLength + 2.0f * static_cast<float>(M_PI) * m_radius;
}

TrackPoint Track::sample(float s) const {
    while (s < 0.0f) s += m_totalLength;
    while (s >= m_totalLength) s -= m_totalLength;

    const float L = m_straightLength;
    const float R = m_radius;
    const float halfPi = static_cast<float>(M_PI) / 2.0f;
    const float pi = static_cast<float>(M_PI);

    const float topY = m_centerY - R;
    const float bottomY = m_centerY + R;
    const float leftX = m_centerX - L / 2.0f;
    const float rightX = m_centerX + L / 2.0f;

    if (s < L) {
        // Top straight, traveling left -> right.
        return { leftX + s, topY, 0.0f };
    }
    s -= L;

    const float arcLen = pi * R;
    if (s < arcLen) {
        // Right-hand semicircle, top -> bottom (clockwise).
        float theta = -halfPi + s / R;
        float x = rightX + R * std::cos(theta);
        float y = m_centerY + R * std::sin(theta);
        return { x, y, theta + halfPi };
    }
    s -= arcLen;

    if (s < L) {
        // Bottom straight, traveling right -> left.
        return { rightX - s, bottomY, pi };
    }
    s -= L;

    // Left-hand semicircle, bottom -> top (clockwise).
    float theta = halfPi + s / R;
    float x = leftX + R * std::cos(theta);
    float y = m_centerY + R * std::sin(theta);
    return { x, y, theta + halfPi };
}

void Track::render(SDL_Renderer* renderer) const {
    std::vector<SDL_Vertex> vertices;
    std::vector<int> indices;
    vertices.reserve(kSegments * 2);
    indices.reserve(kSegments * 6);

    std::vector<SDL_FPoint> outerPoints;
    std::vector<SDL_FPoint> innerPoints;
    outerPoints.reserve(kSegments + 1);
    innerPoints.reserve(kSegments + 1);

    const SDL_Color roadColor{ 60, 60, 65, 255 };

    for (int i = 0; i < kSegments; ++i) {
        float s = m_totalLength * static_cast<float>(i) / static_cast<float>(kSegments);
        TrackPoint p = sample(s);
        float perpX = -std::sin(p.angle);
        float perpY = std::cos(p.angle);

        SDL_FPoint outer{ p.x + perpX * (m_width / 2.0f), p.y + perpY * (m_width / 2.0f) };
        SDL_FPoint inner{ p.x - perpX * (m_width / 2.0f), p.y - perpY * (m_width / 2.0f) };

        outerPoints.push_back(outer);
        innerPoints.push_back(inner);

        vertices.push_back(SDL_Vertex{ outer, roadColor, SDL_FPoint{ 0, 0 } });
        vertices.push_back(SDL_Vertex{ inner, roadColor, SDL_FPoint{ 0, 0 } });
    }

    for (int i = 0; i < kSegments; ++i) {
        int next = (i + 1) % kSegments;
        int outerI = i * 2;
        int innerI = i * 2 + 1;
        int outerNext = next * 2;
        int innerNext = next * 2 + 1;

        indices.push_back(outerI);
        indices.push_back(innerI);
        indices.push_back(outerNext);

        indices.push_back(innerI);
        indices.push_back(innerNext);
        indices.push_back(outerNext);
    }

    SDL_RenderGeometry(renderer, nullptr, vertices.data(), static_cast<int>(vertices.size()),
                        indices.data(), static_cast<int>(indices.size()));

    // Close the loops for the boundary lines.
    outerPoints.push_back(outerPoints.front());
    innerPoints.push_back(innerPoints.front());

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawLinesF(renderer, outerPoints.data(), static_cast<int>(outerPoints.size()));
    SDL_RenderDrawLinesF(renderer, innerPoints.data(), static_cast<int>(innerPoints.size()));
}
