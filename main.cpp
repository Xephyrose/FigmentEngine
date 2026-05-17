#include <iostream>
#include "Node.h"
#include <SDL3/SDL.h>

int main() {

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        return -1;
    }
    SDL_Window *window = SDL_CreateWindow("title", 600, 400, 0);

    Node node;
    node.name = "Test";

    std::cout << "Node's name is " << node.name << std::endl;
    SDL_Quit();
}