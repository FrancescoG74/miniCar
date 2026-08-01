#include "actor/StartLine.h"

#include <cmath>

#include "Track.h"

StartLine::StartLine(const Track& track, float trackWidth_, float s_)
    : Actor("StartLine", SDL_Color{ 235, 235, 235, 255 }),
       s(s_), trackWidth(trackWidth_) {
    TrackPoint p = track.sample(s);
    forwardX = std::cos(p.angle);
    forwardY = std::sin(p.angle);
    perpX = -std::sin(p.angle);
    perpY = std::cos(p.angle);
    setPosition({ p.x, p.y });
}

void StartLine::render(SDL_Renderer* renderer) const {
    if (!active) return;

    const float checkerHeight = trackWidth / static_cast<float>(kCheckerCount);
    const float halfThick = kThickness / 2.0f;
    const SDL_FPoint& pos = getPosition();

    // Each checker is a small rotated quad: ±halfThick along the forward axis and
    // ±checkerHeight/2 along the perpendicular. Drawing them with SDL_RenderGeometry
    // keeps the line correct even when placed on a curved section of track.
    for (int i = 0; i < kCheckerCount; ++i) {
        SDL_Color cByte = (i % 2 == 0) ? SDL_Color{ 20, 20, 20, 255 }
                                        : SDL_Color{ 235, 235, 235, 255 };
        SDL_FColor c{ cByte.r / 255.0f, cByte.g / 255.0f, cByte.b / 255.0f, cByte.a / 255.0f };

        float perpOffset = -trackWidth / 2.0f +
                           checkerHeight * (static_cast<float>(i) + 0.5f);
        float cx = pos.x + perpX * perpOffset;
        float cy = pos.y + perpY * perpOffset;

        float fx = forwardX * halfThick;
        float fy = forwardY * halfThick;
        float px = perpX * (checkerHeight / 2.0f);
        float py = perpY * (checkerHeight / 2.0f);

        SDL_Vertex verts[4];
        verts[0].position = { cx - fx - px, cy - fy - py };
        verts[1].position = { cx + fx - px, cy + fy - py };
        verts[2].position = { cx + fx + px, cy + fy + py };
        verts[3].position = { cx - fx + px, cy - fy + py };
        for (auto& v : verts) {
            v.color = c;
            v.tex_coord = { 0.0f, 0.0f };
        }

        int idx[6] = { 0, 1, 2, 0, 2, 3 };
        SDL_RenderGeometry(renderer, nullptr, verts, 4, idx, 6);
    }
}
