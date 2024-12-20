#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <functional>

class Chip8IO {
public:
    /* Everything on the 2d display array is displayed on the screen,
    all the buttons that are pressed are made = 1 in the 1d keyboard array,
    whenever a new key is pressed func function is called with the keycode of the
    key pressed*/
    Chip8IO(uint8_t** display, uint8_t* keyboard, std::function<void(uint8_t)> func);

    std::function<void(uint8_t)> func;
    uint8_t** display;
    uint8_t* keyboard;

    int mapKeyCodes(sf::Keyboard::Key keyCode);
    void startIO();
};
