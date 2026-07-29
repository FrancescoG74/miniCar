#include "game/WorldRenderer.h"

#include <cmath>

#include "Track.h"
#include "actor/Car.h"
#include "actor/Gate.h"
#include "actor/Rock.h"
#include "actor/StartLine.h"
#include "game/RaceSession.h"

namespace {

// Highlights player-driven cars with a white ring so they stand out from the AI pack.
void drawCircle(SDL_Renderer* renderer, float cx, float cy, float radius, SDL_Color color) {
    constexpr int kPoints = 24;
    SDL_FPoint points[kPoints + 1];
    for (int i = 0; i <= kPoints; ++i) {
        float t = static_cast<float>(i) / kPoints * 2.0f * static_cast<float>(M_PI);
        points[i] = { cx + radius * std::cos(t), cy + radius * std::sin(t) };
    }
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLinesF(renderer, points, kPoints + 1);
}

} // namespace

void WorldRenderer::render(SDL_Renderer* renderer, const Track& track, const StartLine& startLine,
                           const RaceSession& race, const CarSprite& carSprite) {
    track.render(renderer);
    startLine.render(renderer);

    for (const auto& rock : race.rocks()) rock.render(renderer);
    for (const auto& gate : race.gates()) gate.render(renderer);

    for (const auto& car : race.cars()) {
        TrackPoint p = track.sample(car.s); // sampled once for the rotation angle
        float cx = car.getPosition().x;
        float cy = car.getPosition().y;

        SDL_SetTextureColorMod(carSprite.texture, car.getColor().r, car.getColor().g, car.getColor().b);
        SDL_Rect dst{
            static_cast<int>(cx - carSprite.width / 2.0f),
            static_cast<int>(cy - carSprite.height / 2.0f),
            carSprite.width,
            carSprite.height
        };
        double angleDeg = p.angle * 180.0 / M_PI + 90.0;
        SDL_RenderCopyEx(renderer, carSprite.texture, nullptr, &dst, angleDeg, nullptr, SDL_FLIP_NONE);

        if (car.driver() != DriverKind::Ai) {
            drawCircle(renderer, cx, cy, carSprite.height * 0.75f, SDL_Color{ 255, 255, 255, 255 });
        }
    }
}
