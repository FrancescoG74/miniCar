#include "actor/Rock.h"

#include <algorithm>
#include <cmath>
#include <random>

#include "Track.h"

namespace {

constexpr int kSides = 8;
// Radius jitter per vertex so the rock silhouette looks irregular instead of a
// perfect polygon. Values chosen to stay near 1.0 (roughly the nominal size).
constexpr float kRadiusJitter[kSides] = {
    1.00f, 0.78f, 1.05f, 0.85f, 1.10f, 0.80f, 1.02f, 0.90f,
};

} // namespace

Rock::Rock(float s_, float laneOffset_, const Track& track, float size_)
    : Actor("Rock", SDL_Color{ 110, 100, 92, 255 }),
       s(s_), laneOffset(laneOffset_), size(size_) {
    TrackPoint p = track.sample(s);
    float perpX = -std::sin(p.angle);
    float perpY = std::cos(p.angle);
    setPosition({ p.x + perpX * laneOffset, p.y + perpY * laneOffset });
}

void Rock::render(SDL_Renderer* renderer) const {
    if (!active) return;

    const SDL_FPoint& pos = getPosition();
    SDL_Vertex verts[kSides + 1];
    verts[0].position = pos;
    verts[0].color = getColor();
    verts[0].tex_coord = { 0.0f, 0.0f };

    for (int i = 0; i < kSides; ++i) {
        float t = static_cast<float>(i) / kSides * 2.0f * static_cast<float>(M_PI);
        float r = size * kRadiusJitter[i];
        verts[i + 1].position = { pos.x + r * std::cos(t), pos.y + r * std::sin(t) };
        verts[i + 1].color = getColor();
        verts[i + 1].tex_coord = { 0.0f, 0.0f };
    }

    int indices[kSides * 3];
    for (int i = 0; i < kSides; ++i) {
        indices[i * 3 + 0] = 0;
        indices[i * 3 + 1] = 1 + i;
        indices[i * 3 + 2] = 1 + ((i + 1) % kSides);
    }
    SDL_RenderGeometry(renderer, nullptr, verts, kSides + 1, indices, kSides * 3);

    // Darker outline: draw the perimeter as a line loop.
    SDL_SetRenderDrawColor(renderer, 55, 50, 45, 255);
    SDL_FPoint outline[kSides + 1];
    for (int i = 0; i <= kSides; ++i) {
        outline[i] = verts[1 + (i % kSides)].position;
    }
    SDL_RenderDrawLinesF(renderer, outline, kSides + 1);
}

std::vector<Rock> Rock::createInitialRocks(const Track& track) {
    std::mt19937 rng{ std::random_device{}() };
    return createInitialRocks(track, rng);
}

std::vector<Rock> Rock::createInitialRocks(const Track& track, std::mt19937& rng) {
    const float L = track.totalLength();
    // Rocks are placed randomly around the circuit at |laneOffset| >= 45 with size
    // <= 22 so their (size*0.55) collision hitboxes are always outside the AI cars'
    // fixed +/-25 lanes. Only players who steer wide can hit them; AI cars can't
    // change lanes and would otherwise get stuck against an in-lane rock forever.
    std::uniform_int_distribution<int> countDist(5, 8);
    std::uniform_real_distribution<float> sizeDist(12.0f, 22.0f);
    std::uniform_real_distribution<float> lateralDist(45.0f, 50.0f);
    std::uniform_int_distribution<int> sideDist(0, 1); // 0 = left of centerline, 1 = right
    std::uniform_real_distribution<float> jitterDist(-0.03f, 0.03f);

    const int count = countDist(rng);
    // Space rocks roughly evenly around the circuit with a small random jitter so
    // they never end up right on top of each other or right on the start/finish line.
    std::vector<Rock> rocks;
    rocks.reserve(count);
    for (int i = 0; i < count; ++i) {
        float baseFrac = 0.10f + static_cast<float>(i) / static_cast<float>(count) * 0.85f;
        float frac = baseFrac + jitterDist(rng);
        frac = std::clamp(frac, 0.05f, 0.95f);

        float lateral = lateralDist(rng);
        if (sideDist(rng) == 0) lateral = -lateral;

        rocks.emplace_back(L * frac, lateral, track, sizeDist(rng));
    }
    return rocks;
}
