#include "chip8IO.h"
    
Chip8IO::Chip8IO(uint8_t ** display, uint8_t* keyboard, std::function<void(uint8_t)> func) :
        display(display), keyboard(keyboard), func(func) {}

std::function<void(uint8_t)> func;

uint8_t** display;
uint8_t* keyboard;
    
std::unordered_map<sf::Keyboard::Key, int> keyMap = {
    {sf::Keyboard::Num0, 0}, {sf::Keyboard::Num1, 1}, {sf::Keyboard::Num2, 2}, {sf::Keyboard::Num3, 3},
    {sf::Keyboard::Num4, 4}, {sf::Keyboard::Num5, 5}, {sf::Keyboard::Num6, 6}, {sf::Keyboard::Num7, 7},
    {sf::Keyboard::Num8, 8}, {sf::Keyboard::Num9, 9}, {sf::Keyboard::A, 10}, {sf::Keyboard::B, 11},
    {sf::Keyboard::C, 12}, {sf::Keyboard::D, 13}, {sf::Keyboard::E, 14}, {sf::Keyboard::F, 15}
};

int Chip8IO::mapKeyCodes(sf::Keyboard::Key key) {
    auto it = keyMap.find(key);
    if (it != keyMap.end()) {
        return it->second;
    }
    else {
        return -1;
    }
}

void Chip8IO::startIO() {
    sf::RenderWindow window(sf::VideoMode(1000, 500), "CHIP-8");

    sf::RectangleShape rectangle;
    rectangle.setFillColor(sf::Color::Yellow);

    while (window.isOpen()) {
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased) {
                int key = mapKeyCodes(event.key.code);

                if (event.type == sf::Event::KeyReleased) {
                    keyboard[key] = 0;
                }
                else if (key != -1 && keyboard[key] == 0) {
                    keyboard[key] = 1;
                    func(key);
                }
            }
        }

        window.clear(sf::Color::Black);
        sf::Vector2u size = window.getSize();
         
        // drawing the contents of window array on the screen
        rectangle.setSize(sf::Vector2f(size.x / 64.0f, size.y / 32.0f));

        for (int i = 0; i < 64; ++i) {
            for (int j = 0; j < 32; ++j) {
                if (display[i][j] == 1) {
                    rectangle.setPosition(sf::Vector2f(i * size.x / 64.0f, j * size.y / 32.0f));
                    window.draw(rectangle);
                }
            }
        }
            
        window.display();
    }
}