#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include "pathfinder.h"
#include "ResourcePath.hpp"

const unsigned int VSX { 500 };         // view size x
const unsigned int VSY { 500 };         // view size y

const unsigned int WSX { 800 };         // window size x
const unsigned int WSY { 600 };         // window size y

const unsigned int BSX { 250 };          // board size x
const unsigned int BSY { 250 };          // you guessed it

bool            gMouseOnScreen { true };
bool            gMouseLeftPressed { false };
bool            gMouseRightPressed { false };
sf::Vector2i    gMouseStart;
sf::Vector2i    gMouseEnd;
sf::Font        gFont;
sf::Text        tFPS;
sf::Text        tPath;
sf::Text        tMethod;

bool            gJPS { false };

sf::Clock       gPathCalcClock;
sf::Time        gPathCalcTime;

int             gPathTimeAvg { 0 };

bool            gCornerCutting { false };
sf::Text        tCorner;

bool            gSmoothing { false };
sf::Text        tSmooth;


//////////////////////////////////////////////////
//                                              //
//    This is where the magic happens:          //
                                                //
cPathFinder     p { BSX, BSY };                 //
                                                //
//////////////////////////////////////////////////

std::string i2s(int x)
{
    std::string s { };
    while ( x > 0 )
    {
        auto i = x % 10;
        s = char(i + 48) + s;
        x /= 10;
    }
    return s;
}

void processEvents(sf::RenderWindow& window, sf::View& v)
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            window.close();
        }
        
        if ( event.type == sf::Event::MouseLeft )
        {
            gMouseOnScreen = false;
            gMouseLeftPressed = false;
            gMouseRightPressed = false;
        }
        
        if ( event.type == sf::Event::MouseEntered)
            gMouseOnScreen = true;
        
        if ( event.type == sf::Event::MouseButtonPressed && gMouseOnScreen )
        {
            gMouseStart = sf::Mouse::getPosition(window);
            if ( event.mouseButton.button == sf::Mouse::Left)
            {
                if ( gMouseStart.x < 500 && gMouseStart.y < 500)
                {
                    gMouseLeftPressed = true;
                    p.startMarking(gMouseStart);
                }
                else
                {
                    if ( gMouseStart.x > 520 && gMouseStart.x < 700 &&
                          gMouseStart.y > 80 && gMouseStart.y < 100)
                    {
                        if ( gCornerCutting == false )
                        {
                            gCornerCutting = true;
                            tCorner.setString("Corner cutting: allowed");
                        }
                        else
                        {
                            gCornerCutting = false;
                            tCorner.setString("Corner cutting: not allowed");
                        }
                    }
                    if ( gMouseStart.x > 520 && gMouseStart.x < 700 &&
                        gMouseStart.y > 110 && gMouseStart.y < 130)
                    {
                        if ( gSmoothing == false )
                        {
                            gSmoothing = true;
                            tSmooth.setString("Path smoothing: on");
                        }
                        else
                        {
                            gSmoothing = false;
                            tSmooth.setString("Path smoothing: off");
                        }
                    }
                    if ( gMouseStart.x > 520 && gMouseStart.x < 700 &&
                         gMouseStart.y > 140 && gMouseStart.y < 160)
                    {
                        if ( gJPS == false )
                        {
                            gJPS = true;
                            tMethod.setString("Method: jump point search");
                            p.mJPS = true;
                        }
                        else
                        {
                            gJPS = false;
                            tMethod.setString("Method: standard A*");
                            p.mJPS = false;
                        }
                    }
                }
            }
            if ( event.mouseButton.button == sf::Mouse::Right )
                gMouseRightPressed = true;
            
        }
        
        if ( event.type == sf::Event::MouseButtonReleased )
        {
            if ( event.mouseButton.button == sf::Mouse::Left && gMouseLeftPressed )
            {
                gMouseLeftPressed = false;
                gMouseEnd = sf::Mouse::getPosition(window);
                p.toggleMarkedOnes();
            }
            if ( event.mouseButton.button == sf::Mouse::Right && gMouseRightPressed )
            {
                gMouseRightPressed = false;
                gMouseEnd = sf::Mouse::getPosition(window);
            }
        }
    }
    
    if ( sf::Mouse::isButtonPressed(sf::Mouse::Left) && gMouseOnScreen
        && gMouseLeftPressed)
        p.keepMarking(sf::Mouse::getPosition(window));
    
    gPathCalcClock.restart();
    bool tmp { false };
    if ( gMouseOnScreen && !gMouseLeftPressed && !gMouseRightPressed )
    {
        tmp = p.walk(sf::Vector2i(40,40), sf::Mouse::getPosition(window), gCornerCutting, gSmoothing);
    }
    gPathCalcTime = gPathCalcClock.restart();
    
    if (tmp)    // only measure succesful pathing
    {
        if ( gPathTimeAvg == 0 )
        {
            gPathTimeAvg = 2 * static_cast<int>(gPathCalcTime.asMicroseconds());
        }
        else
            gPathTimeAvg += gPathCalcTime.asMicroseconds();
        gPathTimeAvg /= 2;
    }
}

int main()
{
    sf::RenderWindow    window;
    window.create(sf::VideoMode(WSX, WSY), " A* test ");
    
    window.setVerticalSyncEnabled(true);
    
    sf::View    mainView { window.getDefaultView() };
    mainView.setSize(VSX, VSY);
    mainView.setCenter(static_cast<float>(VSX) / 2.0,
                       static_cast<float>(VSY) / 2.0);
    
    mainView.setViewport(sf::FloatRect(0,0, 5.0 / 8.0, 5.0 / 6.0));
    
    sf::View        guiView { window.getDefaultView() };
    
    sf::Time        timeSinceLastRender { sf::Time::Zero };
    sf::Time        timeSinceLastUpdate { sf::Time::Zero };
    sf::Clock       clock;
    sf::Clock       fpsclock;
    short int       currentFPS { 0 };
    short int       pastFPS { 0 };
    
    sf::Mouse::setPosition(sf::Vector2i(100, 100), window);
    
    gFont.loadFromFile(resourcePath() + "sansation.ttf");
    tFPS.setFont(gFont);
    tFPS.setCharacterSize(16);
    tFPS.setColor(sf::Color::White);
    tFPS.setPosition(520, 20);
    
    tPath.setFont(gFont);
    tPath.setCharacterSize(16);
    tPath.setColor(sf::Color::White);
    tPath.setPosition(520, 50);
    
    tCorner.setFont(gFont);
    tCorner.setCharacterSize(16);
    tCorner.setColor(sf::Color::White);
    tCorner.setPosition(520, 80);
    tCorner.setString("Corner cutting: not allowed");
    
    tSmooth.setFont(gFont);
    tSmooth.setCharacterSize(16);
    tSmooth.setColor(sf::Color::White);
    tSmooth.setPosition(520, 110);
    tSmooth.setString("Path smoothing: off");
    
    tMethod.setFont(gFont);
    tMethod.setCharacterSize(16);
    tMethod.setColor(sf::Color::White);
    tMethod.setPosition(520, 140);
    tMethod.setString("Method: standard A*");

    // OK now the view won't change, so we can set it once and
    // then forget about it, but later an eye should be kept on
    // updating it as necessary.
    
    p.setView(mainView);
    fpsclock.restart();
    
    while ( window.isOpen())
    {
        processEvents(window, mainView);
        timeSinceLastUpdate = clock.restart();
        
        /*
         
        // This makes sense to have if the bottleneck is
        // _rendering_, not event processing or path calculation.
        // Otherwise it's counterproductive.
         
        while ( timeSinceLastUpdate > sf::seconds(1.0 / 60.0))
        {
            processEvents(window, mainView);
            timeSinceLastUpdate -= sf::seconds(1.0 / 60.0);
        }
         */
        
        window.clear();
        window.setView(mainView);
        
        p.render(window);
        ++currentFPS;
        timeSinceLastRender += fpsclock.restart();
        if ( timeSinceLastRender >= sf::seconds(1.0) )
        {
            pastFPS = currentFPS;
            currentFPS = 0;
            tFPS.setString("FPS: " + i2s(pastFPS));
            timeSinceLastRender -= sf::seconds(1.0);
            tPath.setString("Avg. pathing time (microsec.): " + i2s(gPathTimeAvg));
        }
        
        window.setView(guiView);
        window.draw(tFPS);
        window.draw(tPath);
        window.draw(tCorner);
        window.draw(tSmooth);
        window.draw(tMethod);
        window.display();
    }

    return 0;
}

