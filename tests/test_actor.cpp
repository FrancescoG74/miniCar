#include <catch2/catch_all.hpp>
#include <string>

#include "actor/Actor.h"

TEST_CASE("Actor default constructor is active with an empty name", "[actor]") {
    Actor a;
    REQUIRE(a.active);
    REQUIRE(std::string(a.getName()).empty());
}

TEST_CASE("Actor constructed with name/color stores both", "[actor]") {
    SDL_Color color{ 10, 20, 30, 255 };
    Actor a("TestActor", color);
    REQUIRE(std::string(a.getName()) == "TestActor");
    REQUIRE(a.getColor().r == 10);
    REQUIRE(a.getColor().g == 20);
    REQUIRE(a.getColor().b == 30);
}

TEST_CASE("Actor ids are unique and increase with each construction", "[actor]") {
    Actor a;
    Actor b;
    REQUIRE(b.getId() > a.getId());
}

TEST_CASE("Actor position defaults to origin and is settable", "[actor]") {
    Actor a;
    REQUIRE(a.getPosition().x == Catch::Approx(0.0f));
    REQUIRE(a.getPosition().y == Catch::Approx(0.0f));

    a.setPosition({ 5.0f, -3.0f });
    REQUIRE(a.getPosition().x == Catch::Approx(5.0f));
    REQUIRE(a.getPosition().y == Catch::Approx(-3.0f));
}

TEST_CASE("Actor name and color are mutable after construction", "[actor]") {
    Actor a;
    a.setName("Renamed");
    a.setColor(SDL_Color{ 1, 2, 3, 4 });
    REQUIRE(std::string(a.getName()) == "Renamed");
    REQUIRE(a.getColor().r == 1);
    REQUIRE(a.getColor().a == 4);
}
