#include <SFML/Graphics.hpp>
#include "Constants.h"
#include "Utils.h"
#include "Game.h"
#include <windows.h>

using namespace sf;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    bool isFullscreen = false;
    RenderWindow window(
        VideoMode(static_cast<unsigned>(WIN_W), static_cast<unsigned>(WIN_H)),
        "Guitar Hra");
    window.setFramerateLimit(60);
    window.setView(computeView(window.getSize().x, window.getSize().y));

    Font font;
    if (!font.loadFromFile("PressStart2P-Regular.ttf"))
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf"))
            return -1;

    Game  game(font);
    Clock clock;

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::Resized)
                window.setView(computeView(event.size.width, event.size.height));

            if (event.type == Event::KeyPressed)
            {
                if (event.key.code == Keyboard::F11)
                {
                    isFullscreen = !isFullscreen;
                    window.create(
                        isFullscreen ? VideoMode::getDesktopMode()
                                     : VideoMode(static_cast<unsigned>(WIN_W),
                                                 static_cast<unsigned>(WIN_H)),
                        "Guitar Hra",
                        isFullscreen ? Style::Fullscreen : Style::Default);
                    window.setFramerateLimit(60);
                    window.setView(computeView(window.getSize().x, window.getSize().y));
                }

                if (event.key.code == Keyboard::Escape)
                {
                    if (isFullscreen) {
                        isFullscreen = false;
                        window.create(VideoMode(static_cast<unsigned>(WIN_W),
                                                static_cast<unsigned>(WIN_H)), "Guitar Hra");
                        window.setFramerateLimit(60);
                        window.setView(computeView(window.getSize().x, window.getSize().y));
                    } else {
                        window.close();
                    }
                }
            }

            game.handleEvent(event);
        }

        game.update(dt);
        window.clear(Color(18, 18, 28));
        game.draw(window);
        window.display();
    }

    return 0;
}
