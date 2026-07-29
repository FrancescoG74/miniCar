#include "Actor.h"

int Actor::s_nextId = 0;

Actor::Actor() : id(++s_nextId) {}

Actor::Actor(const char* name_, SDL_Color color_)
    : id(++s_nextId), name(name_), color(color_) {}
