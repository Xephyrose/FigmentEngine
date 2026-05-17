#ifndef SDL_TEST_NODE_H
#define SDL_TEST_NODE_H
#include <string>

#include "Vector2.h"


class Node {
public:
    std::string name;
    Vector2 position;
    float radians;
};


#endif //SDL_TEST_NODE_H
