#include "actor/Gate.h"

#include <cmath>
#include <random>

#include "Track.h"

namespace {

float randomRange(float lo, float hi) {
    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<float> dist(lo, hi);
    return dist(rng);
}

} // namespace

Gate::Gate(float s_, const Track& track, float trackHalfWidth_)
    : Actor("Gate", SDL_Color{ 220, 200, 40, 255 }),
       s(s_), trackHalfWidth(trackHalfWidth_) {
    TrackPoint p = track.sample(s);
    forwardX = std::cos(p.angle);
    forwardY = std::sin(p.angle);
    perpX = -std::sin(p.angle);
    perpY = std::cos(p.angle);
    setPosition({ p.x, p.y });
    postA = { p.x + perpX * trackHalfWidth, p.y + perpY * trackHalfWidth };
    postB = { p.x - perpX * trackHalfWidth, p.y - perpY * trackHalfWidth };

    // Start each gate in a random phase (open or closed) with a random amount of
    // time left in that phase, so multiple gates don't begin in lockstep.
    m_closed = randomRange(0.0f, 1.0f) < 0.5f;
    m_phaseTimer = m_closed ? randomRange(kMinClosedDuration, kMaxClosedDuration)
                             : randomRange(kMinOpenDuration, kMaxOpenDuration);
}

void Gate::update(float dt) {
    m_phaseTimer -= dt;
    // Loop (rather than a single if) in case dt ever exceeds the rolled duration.
    while (m_phaseTimer <= 0.0f) {
        m_closed = !m_closed;
        float duration = m_closed ? randomRange(kMinClosedDuration, kMaxClosedDuration)
                                    : randomRange(kMinOpenDuration, kMaxOpenDuration);
        m_phaseTimer += duration;
    }
}

void Gate::render(SDL_Renderer* renderer) const {
    if (!active) return;

    const bool closed = isClosed();
    const SDL_Color barColor = closed
        ? SDL_Color{ 220, 40, 40, 255 }
        : SDL_Color{ 60, 200, 80, 255 };

    // Two dark posts anchoring the gate on either side of the track.
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_FRect postARect{ postA.x - 4.0f, postA.y - 4.0f, 8.0f, 8.0f };
    SDL_FRect postBRect{ postB.x - 4.0f, postB.y - 4.0f, 8.0f, 8.0f };
    SDL_RenderFillRectF(renderer, &postARect);
    SDL_RenderFillRectF(renderer, &postBRect);

    // The bar itself is a rectangle spanning postA -> postB with `thickness` extent
    // along the track's forward direction. Built as two triangles via RenderGeometry
    // so it draws correctly at any track angle (straights + curves).
    const float halfThick = thickness / 2.0f;
    SDL_Vertex verts[4];
    verts[0].position = { postA.x + forwardX * halfThick, postA.y + forwardY * halfThick };
    verts[1].position = { postB.x + forwardX * halfThick, postB.y + forwardY * halfThick };
    verts[2].position = { postB.x - forwardX * halfThick, postB.y - forwardY * halfThick };
    verts[3].position = { postA.x - forwardX * halfThick, postA.y - forwardY * halfThick };
    for (int i = 0; i < 4; ++i) {
        verts[i].color = barColor;
        verts[i].tex_coord = { 0.0f, 0.0f };
    }
    int indices[6] = { 0, 1, 2, 0, 2, 3 };

    if (closed) {
        // Solid red bar blocks the track.
        SDL_RenderGeometry(renderer, nullptr, verts, 4, indices, 6);
    } else {
        // Open: only draw the bar outline in green so the gate stays visible but
        // clearly signals "passable".
        SDL_SetRenderDrawColor(renderer, barColor.r, barColor.g, barColor.b, barColor.a);
        SDL_FPoint outline[5] = {
            verts[0].position, verts[1].position, verts[2].position, verts[3].position, verts[0].position
        };
        SDL_RenderDrawLinesF(renderer, outline, 5);
    }
}

std::vector<Gate> Gate::createInitialGates(const Track& track, float trackWidth) {
    const float L = track.totalLength();
    const float halfWidth = trackWidth / 2.0f;
    // Placed on the two curved sections of the stadium track. Each gate's
    // open/close timing is independently randomized (see Gate::update), so they
    // no longer follow a fixed synchronized pattern.
    return {
        Gate(L * 0.25f, track, halfWidth),
        Gate(L * 0.75f, track, halfWidth),
    };
}
