#pragma once

#include "game/GameState.h"

// The pre-race countdown. Freezes the world, ticks a digit from 3 down to "Go",
// cues audio (voice if available, beep otherwise), and transitions to the
// RacingState the moment the timer hits zero.
class CountdownState : public GameState {
public:
    void onEnter(Game& game) override;
    void update(Game& game, float dt) override;
    void renderOverlay(Game& game) override;
};

// The race is running: cars/gates tick, collisions resolve, engines whine,
// and the moment any car crosses the winning lap the state transitions to
// FinishedState.
class RacingState : public GameState {
public:
    void update(Game& game, float dt) override;
};

// Race is over: engines silenced, a semi-transparent overlay + winner banner
// on top of the still-rendered world. Restart via 'R' (handled by Game).
class FinishedState : public GameState {
public:
    void onEnter(Game& game) override;
    void update(Game& game, float dt) override;
    void renderOverlay(Game& game) override;
};
