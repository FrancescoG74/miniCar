#include "Track.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace {
constexpr int kSegments = 240;
constexpr int kSplineStepsPerSegment = 32;
constexpr float kPi = static_cast<float>(M_PI);

SDL_FPoint catmullRomPoint(SDL_FPoint p0, SDL_FPoint p1, SDL_FPoint p2, SDL_FPoint p3,
               float t) {
    const float t2 = t * t;
    const float t3 = t2 * t;
    return {
    0.5f * ((2.0f * p1.x) + (-p0.x + p2.x) * t +
        (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * t2 +
        (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * t3),
    0.5f * ((2.0f * p1.y) + (-p0.y + p2.y) * t +
        (2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * t2 +
        (-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * t3),
    };
}

SDL_FPoint catmullRomTangent(SDL_FPoint p0, SDL_FPoint p1, SDL_FPoint p2, SDL_FPoint p3,
                 float t) {
    const float t2 = t * t;
    return {
    0.5f * ((-p0.x + p2.x) +
        2.0f * (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * t +
        3.0f * (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * t2),
    0.5f * ((-p0.y + p2.y) +
        2.0f * (2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * t +
        3.0f * (-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * t2),
    };
}
}

Track::Track(float centerX, float centerY, float straightLength, float radius, float width)
    : m_centerX(centerX), m_centerY(centerY),
      m_straightLength(straightLength), m_radius(radius), m_width(width) {
    m_totalLength = 2.0f * m_straightLength + 2.0f * static_cast<float>(M_PI) * m_radius;
}

Track::Track(float centerX, float centerY, float width, std::vector<SDL_FPoint> controlPoints)
    : m_centerX(centerX), m_centerY(centerY), m_straightLength(0.0f), m_radius(0.0f),
      m_width(width), m_totalLength(0.0f) {
    if (controlPoints.size() < 4) {
        throw std::invalid_argument("A spline track requires at least four control points");
    }

    const std::size_t pointCount = controlPoints.size();
    m_path.reserve(pointCount * kSplineStepsPerSegment + 1);
    SDL_FPoint previous{};
    for (std::size_t index = 0; index <= pointCount * kSplineStepsPerSegment; ++index) {
        const std::size_t segment = index / kSplineStepsPerSegment;
        const float t = static_cast<float>(index % kSplineStepsPerSegment) /
                        static_cast<float>(kSplineStepsPerSegment);
        const SDL_FPoint p0 = controlPoints[(segment + pointCount - 1) % pointCount];
        const SDL_FPoint p1 = controlPoints[segment % pointCount];
        const SDL_FPoint p2 = controlPoints[(segment + 1) % pointCount];
        const SDL_FPoint p3 = controlPoints[(segment + 2) % pointCount];
        const SDL_FPoint position = catmullRomPoint(p0, p1, p2, p3, t);
        const SDL_FPoint tangent = catmullRomTangent(p0, p1, p2, p3, t);

        if (index > 0) {
            const float dx = position.x - previous.x;
            const float dy = position.y - previous.y;
            m_totalLength += std::sqrt(dx * dx + dy * dy);
        }
        m_path.push_back({ m_totalLength, position.x, position.y,
                           std::atan2(tangent.y, tangent.x) });
        previous = position;
    }
}

TrackPoint Track::sample(float s) const {
    while (s < 0.0f) s += m_totalLength;
    while (s >= m_totalLength) s -= m_totalLength;

    if (!m_path.empty()) {
        const auto next = std::lower_bound(
            m_path.begin(), m_path.end(), s,
            [](const PathSample& sample, float distance) { return sample.distance < distance; });
        if (next == m_path.begin()) return { next->x, next->y, next->angle };

        const PathSample& before = *(next - 1);
        const float fraction = (s - before.distance) / (next->distance - before.distance);
        float angleDelta = next->angle - before.angle;
        if (angleDelta > kPi) angleDelta -= 2.0f * kPi;
        if (angleDelta < -kPi) angleDelta += 2.0f * kPi;
        return {
            before.x + (next->x - before.x) * fraction,
            before.y + (next->y - before.y) * fraction,
            before.angle + angleDelta * fraction,
        };
    }

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

    const SDL_Color roadColorByte{ 60, 60, 65, 255 };
    const SDL_FColor roadColor{ roadColorByte.r / 255.0f, roadColorByte.g / 255.0f, 
                                roadColorByte.b / 255.0f, roadColorByte.a / 255.0f };

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
    SDL_RenderLines(renderer, outerPoints.data(), static_cast<int>(outerPoints.size()));
    SDL_RenderLines(renderer, innerPoints.data(), static_cast<int>(innerPoints.size()));
}
