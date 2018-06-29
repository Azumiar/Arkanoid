#include <SFML/Graphics.hpp>
#include <sstream>
#include <map>
#include <cmath>

using namespace std;
using namespace sf;

string int_to_string(int i){
    stringstream ss;
    ss<<i;
    return ss.str();
}

int randomRange(int min, int max){
    return min + (rand() % static_cast<int>(max - min + 1));
}

class InputController{
public:
    map<Keyboard::Key, bool> keys;

    void keyPress(Keyboard::Key key){
        keys[key] = true;
    }

    void keyRelease(Keyboard::Key key){
        keys[key] = false;
    }

    bool isPressed(Keyboard::Key key){
        return keys[key] ? keys[key] : false;
    }
};

class Ball{
public:
    float x, y, dx, dy;
    int score = 0;
    bool alive = true;
    RectangleShape body;
    Ball(int _x, int _y){
        x = _x;
        y = _y;
        body = RectangleShape(Vector2f(10, 10));
        body.setPosition(x, y);
        body.setFillColor(Color::White);
        int angle = randomRange(20, 45);
        dx = cos(angle * 0.01745329252);
        dy = sin(angle * 0.01745329252);
        dx *= randomRange(0, 2) < 1 ? -1 : 1;
        dy *= -1;
    }

    void reset(){
        int angle = randomRange(20, 45);
        dx = cos(angle * 0.01745329252);
        dy = sin(angle * 0.01745329252);
        dy *= -1;
        alive = true;
        x = 200;
        y = 540;
        body.setPosition(x, y);
    }

    void tick(Time deltaTime){
        if(!alive){
            return;
        }
        dx = dx > 1.5f ? 1.5f : dx;
        dx = dx < -1.5f ? -1.5f : dx;
        dy = dy > 1.5f ? 1.5f : dy;
        dy = dy < -1.5f ? -1.5f : dy;
        x += dx * deltaTime.asSeconds() * 300.f;
        y += dy * deltaTime.asSeconds() * 300.f;
        if(x < 0 || x + 10 > 400){
            dx *= -1;
        }
        if(y > 610) {
            alive = false;
        }else if(y < 0){
            dy *= -1;
        }
        body.setPosition(x, y);
    }

    void draw(RenderWindow* window){
        window->draw(body);
    }
};

class Paddle{
public:
    float x, y, lastX;
    RectangleShape body;
    InputController* ic;
    Keyboard::Key upKey;
    Keyboard::Key downKey;
    Paddle(int _x, int _y, InputController* _ic, Keyboard::Key _upKey, Keyboard::Key _downKey){
        x = _x;
        y = _y;
        ic = _ic;
        upKey = _upKey;
        downKey = _downKey;
        body = RectangleShape(Vector2f(40, 10));
        body.setPosition(x, y);
        body.setFillColor(Color::White);
    }

    void draw(RenderWindow* window){
        window->draw(body);
    }

    void tick(Time deltaTime, Ball* ball){
        lastX = x;
        if(ic->isPressed(upKey) && x > 0){
            x -= deltaTime.asSeconds() * 500.0f;
            body.setPosition(x, y);
        }else if(ic->isPressed(downKey) && x + 40 < 400){
            x += deltaTime.asSeconds() * 500.0f;
            body.setPosition(x, y);
        }

        if(ball->y + 10 > y && ball->y < y + 10 && ball->x > x - 10 &&  ball->x < x + 40){
            if(x - lastX > 0){
                ball->dx += 0.5f;
            } else if(x - lastX < 0){
                ball->dx -= 0.5f;
            }

            ball->dy *= -1;
        }
    }
};

class Brick{
public:
    int x, y, score;
    RectangleShape body;
    Brick(int _x, int _y, int _score, Color color){
        x = _x;
        y = _y;
        score = _score;
        body = RectangleShape(Vector2f(40, 20));
        body.setFillColor(color);
        body.setPosition(x, y);
    }

    void draw(RenderWindow* window){
        window->draw(body);
    }

    int intersects(Ball* ball){
        if(abs((ball->x + 5) - (x + 20)) * 2 < 50 && abs((ball->y + 5) - (y + 10)) * 2 < 30){
            if(ball->x + 10 > x && ball->x < x + 40 && ball->y < y && ball->y + 10 > y){
                return 1;
            }else if(ball->x + 10 > x && ball->x < x + 40 && ball->y < y + 20 && ball->y + 10 > y + 20){
                return 3;
            }else if(ball->y + 10 > y && ball->y < y + 20 && ball->x < x + 40 && ball->x + 10 > x + 40){
                return 2;
            }else{
                return 4;
            }
        }
        return 0;
    }
};

int main()
{
    srand(time(0));

    RenderWindow window(VideoMode(400, 600), "Break Out");

    InputController ic;
    Paddle player(180, 550, &ic, Keyboard::A, Keyboard::D);
    Ball ball(200, 540);
    Clock deltaTimer;

    vector<Brick> bricks;

    for(int i = 0; i < 10; ++i){
        Brick b(40 * i, 50, 40, Color::Red);
        bricks.push_back(b);
    }

    for(int i = 0; i < 10; ++i){
        Brick b(40 * i, 70, 30, Color(255, 110, 0));
        bricks.push_back(b);
    }

    for(int i = 0; i < 10; ++i){
        Brick b(40 * i, 90, 20, Color::Green);
        bricks.push_back(b);
    }

    for(int i = 0; i < 10; ++i){
        Brick b(40 * i, 110, 10, Color::Yellow);
        bricks.push_back(b);
    }

    bool gameEnded = false;

    Font font;
    font.loadFromFile("fff.ttf");

    Text playerScore;
    playerScore.setFont(font);
    playerScore.setFillColor(Color::White);

    Text playerWinText;
    playerWinText.setFont(font);
    playerWinText.setFillColor(Color::White);

    Text restartText;
    restartText.setFont(font);
    restartText.setFillColor(Color::White);
    restartText.setCharacterSize(16);
    restartText.setString("           YOU LOST\nPRESS 'R' TO RESTART");
    FloatRect textRect = restartText.getLocalBounds();
    restartText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top  + textRect.height / 2.0f);
    restartText.setPosition(Vector2f(200, 300));

    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();

            if(event.type == Event::KeyPressed){
                if(!ball.alive && event.key.code == Keyboard::R){
                    ball.score = 0;
                    ball.alive = false;
                    bricks.clear();
                    for(int i = 0; i < 10; ++i){
                        Brick b(40 * i, 50, 40, Color::Red);
                        bricks.push_back(b);
                    }

                    for(int i = 0; i < 10; ++i){
                        Brick b(40 * i, 70, 30, Color(255, 110, 0));
                        bricks.push_back(b);
                    }

                    for(int i = 0; i < 10; ++i){
                        Brick b(40 * i, 90, 20, Color::Green);
                        bricks.push_back(b);
                    }

                    for(int i = 0; i < 10; ++i){
                        Brick b(40 * i, 110, 10, Color::Yellow);
                        bricks.push_back(b);
                    }

                    restartText.setString("           YOU LOST\nPRESS 'R' TO RESTART");
                    FloatRect textRect = restartText.getLocalBounds();
                    restartText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top  + textRect.height / 2.0f);
                    restartText.setPosition(Vector2f(200, 300));

                    ball.reset();
                }
                ic.keyPress(event.key.code);
            }

            if(event.type == Event::KeyReleased){
                ic.keyRelease(event.key.code);
            }
        }

        Time deltaTime = deltaTimer.restart();

        if(ball.alive){
            ball.tick(deltaTime);
            for(int i = 0; i < bricks.size(); ++i){
                Brick b = bricks.at(i);
                int a = b.intersects(&ball);
                if(a){
                    ball.score += b.score;
                    if(ball.score == 1000){
                        ball.alive = false;
                        restartText.setString("           YOU WON\nPRESS 'R' TO RESTART");
                        FloatRect textRect = restartText.getLocalBounds();
                        restartText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top  + textRect.height / 2.0f);
                        restartText.setPosition(Vector2f(200, 300));
                    }
                    bricks.erase(bricks.begin() + i);
                    if(a == 1 || a == 3){
                        ball.dy *= -1;
                    }else{
                        ball.dx *= -1;
                    }
                    break;
                }
            }
            player.tick(deltaTime, &ball);
        }

        playerScore.setString(int_to_string(ball.score));
        FloatRect textRect = playerScore.getLocalBounds();
        playerScore.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top  + textRect.height / 2.0f);
        playerScore.setPosition(Vector2f(50, 40));

        window.clear();
        player.draw(&window);
        ball.draw(&window);
        for(int i = 0; i < bricks.size(); ++i){
            bricks.at(i).draw(&window);
        }
        window.draw(playerScore);
        if(!ball.alive){
            window.draw(playerWinText);
            window.draw(restartText);
        }
        window.display();
    }

    return 0;
}
