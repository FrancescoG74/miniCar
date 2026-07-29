#include "actor/Gate.h"

#include <cmath>

#include "actor/Car.h"
#include "Track.h"

Gate::Gate(float s_, float phaseOffset_, const Track& track, float trackHalfWidth_)
    : Actor("Gate", SDL_Color{ 220, 200, 40, 255 }),
       s(s_), trackHalfWidth(trackHalfWidth_), phaseOffset(phaseOffset_) {
    TrackPoint p = track.sample(s);
    forwardX = std::cos(p.angle);
    forwardY = std::sin(p.angle);
    perpX = -std::sin(p.angle);
    perpY = std::cos(p.angle);
    setPosition({ p.x, p.y });
    postA = { p.x + perpX * trackHalfWidth, p.y + perpY * trackHalfWidth };
    postB = { p.x - perpX * trackHalfWidth, p.y - perpY * trackHalfWidth };
}

void Gate::update(float dt) {
    elapsed += dt;
}

bool Gate::isClosed() const {
    // Phase in [0, kCycleDuration). The first kClosedDuration seconds of the cycle
    // are "closed", the remainder is "open".
    float phase = std::fmod(elapsed + phaseOffset, kCycleDuration);
    if (phase < 0.0f) phase += kCycleDuration;
    return phase < kClosedDuration;
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
    // Placed on the two curved sections of the stadium track, with phases half a
    // cycle apart so one is typically open while the other is closed.
    return {
        Gate(L * 0.25f, 0.0f,                 track, halfWidth),
        Gate(L * 0.75f, kCycleDuration / 2.0f, track, halfWidth),
    };
}

void Gate::resolveCarCollisions(std::vector<Car>& cars,
                                  const std::vector<Gate>& gates,
                                  float totalLength,
                                  float carLength) {
    for (auto& car : cars) {
        for (const auto& gate : gates) {
            if (!gate.active || !gate.isClosed()) continue;

            // Wrap-aware forward gap in track-local coords.
            float gap = gate.s - car.s;
            while (gap > totalLength / 2.0f) gap -= totalLength;
            while (gap < -totalLength / 2.0f) gap += totalLength;

            const float sExtent = carLength / 2.0f + gate.thickness / 2.0f;
            if (std::abs(gap) >= sExtent) continue;

            // Gates span the full track width, so any car in the s-overlap collides.
            float push = sExtent - std::abs(gap) + 1.0f;
            if (gap >= 0.0f) {
                car.s = std::fmod(car.s - push + totalLength, totalLength);
            } else {
                car.s = std::fmod(car.s + push + totalLength, totalLength);
            }
            car.speed = 0.0f;
            car.recoveryTimer = Car::kRecoveryDuration;
        }
    }
}
