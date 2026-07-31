#include <catch2/catch_all.hpp>

#include "Sprite.h"

TEST_CASE("Sprite defaults to no texture and a single frame", "[sprite]") {
    Sprite sprite;
    REQUIRE(sprite.texture() == nullptr);
    REQUIRE_FALSE(sprite.isAnimated());
    REQUIRE(sprite.currentFrame() == 0);
}

TEST_CASE("Sprite stores the texture it's constructed or assigned with", "[sprite]") {
    auto* fakeTexture = reinterpret_cast<SDL_Texture*>(0x1);
    Sprite sprite(fakeTexture);
    REQUIRE(sprite.texture() == fakeTexture);

    auto* otherTexture = reinterpret_cast<SDL_Texture*>(0x2);
    sprite.setTexture(otherTexture);
    REQUIRE(sprite.texture() == otherTexture);
}

TEST_CASE("Sprite::setAnimation marks a multi-frame sprite as animated", "[sprite]") {
    Sprite sprite;
    sprite.setAnimation(16, 16, 4, 0.1f);
    REQUIRE(sprite.isAnimated());
    REQUIRE(sprite.currentFrame() == 0);
}

TEST_CASE("Sprite::advance steps to the next frame once its duration elapses", "[sprite]") {
    Sprite sprite;
    sprite.setAnimation(16, 16, 4, 0.1f);

    sprite.advance(0.05f);
    REQUIRE(sprite.currentFrame() == 0);

    sprite.advance(0.06f);
    REQUIRE(sprite.currentFrame() == 1);
}

TEST_CASE("Sprite::advance can skip multiple frames in one large step", "[sprite]") {
    Sprite sprite;
    sprite.setAnimation(16, 16, 4, 0.1f);

    sprite.advance(0.35f);
    REQUIRE(sprite.currentFrame() == 3);
}

TEST_CASE("Sprite::advance loops back to the first frame by default", "[sprite]") {
    Sprite sprite;
    sprite.setAnimation(16, 16, 4, 0.1f);

    sprite.advance(0.45f); // 4.5 frames: wraps past frame 3 back to frame 0
    REQUIRE(sprite.currentFrame() == 0);
}

TEST_CASE("Sprite::advance holds the last frame when looping is disabled", "[sprite]") {
    Sprite sprite;
    sprite.setAnimation(16, 16, 4, 0.1f, /*framesPerRow=*/0, /*loop=*/false);

    sprite.advance(10.0f);
    REQUIRE(sprite.currentFrame() == 3);
}

TEST_CASE("Sprite::resetAnimation returns to the first frame", "[sprite]") {
    Sprite sprite;
    sprite.setAnimation(16, 16, 4, 0.1f);
    sprite.advance(0.25f);
    REQUIRE(sprite.currentFrame() == 2);

    sprite.resetAnimation();
    REQUIRE(sprite.currentFrame() == 0);
}

TEST_CASE("Sprite::advance is a no-op without a configured animation", "[sprite]") {
    Sprite sprite;
    sprite.advance(5.0f);
    REQUIRE(sprite.currentFrame() == 0);
}
