#pragma once

// Immutable gameplay values shared by race simulation, collision resolution and
// controller strategies. Keeping related numbers together prevents the physics,
// AI and driver handoff code from silently drifting apart.
struct RaceTuning {
    float maxCarSpeed = 130.0f;
    float playerAcceleration = 60.0f;
    float playerBrake = 90.0f;
    float playerSteerRate = 70.0f;
    float aiAcceleration = 50.0f;
    float recoveryBoost = 2.5f;

    float laneLimit = 53.0f;
    float carLength = 30.0f;
    float carWidth = 16.0f;
    int lapsToWin = 5;
};

struct AiTuning {
    float homeLaneOffset = 25.0f;
    float laneMatchTolerance = 15.0f;
    float overtakeLookAhead = 90.0f;
    float overtakeSpeedMargin = 5.0f;
    float safeGapFront = 100.0f;
    float safeGapBack = 45.0f;
    float steerRate = 45.0f;
    float followingLaneTolerance = 35.0f;
    float followingLookAhead = 60.0f;
    float obstacleLookAhead = 120.0f;
};
