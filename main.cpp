#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>


sf::Sprite spawnEnemy(const std::vector<sf::Texture>& textures, const sf::RenderWindow& window) {
    int textureIndex = rand() % textures.size();
    sf::Sprite enemy;
    enemy.setTexture(textures[textureIndex]);

    float xPos = static_cast<float>(rand() % (window.getSize().x - static_cast<int>(enemy.getLocalBounds().width)));
    enemy.setPosition(xPos, -enemy.getLocalBounds().height);

    return enemy;
}


void resetGame(
    sf::Sprite& spaceship,
    std::vector<sf::Sprite>& enemies,
    std::vector<sf::Sprite>& lasers,
    bool& gameOver,
    int& score,
    sf::Clock& spawnClock,
    sf::Clock& shootClock,
    sf::Time spawnInterval,
    sf::Time shootInterval,
    const sf::RenderWindow& window
) {
    sf::FloatRect spaceshipBounds = spaceship.getLocalBounds();
    spaceship.setPosition(
        (window.getSize().x - spaceshipBounds.width * spaceship.getScale().x) / 2,
        window.getSize().y - spaceshipBounds.height * spaceship.getScale().y
    );

    enemies.clear();
    lasers.clear();

    gameOver = false;
    score = 0;

    spawnClock.restart();
    shootClock.restart();
}

int main() {
    srand(static_cast<unsigned int>(time(0)));

    sf::RenderWindow window(sf::VideoMode(500, 500), "SPACE BOOM");

    // Load and resize the icon
    sf::Image icon;
    if (!icon.loadFromFile("icon.png")) return -1;

    // Scale down the icon
    sf::Image resizedIcon;
    resizedIcon.create(32, 32);
    for (unsigned int y = 0; y < 32; ++y) {
        for (unsigned int x = 0; x < 32; ++x) {
            resizedIcon.setPixel(x, y, icon.getPixel(x * 16, y * 16));
        }
    }
    window.setIcon(resizedIcon.getSize().x, resizedIcon.getSize().y, resizedIcon.getPixelsPtr());

    sf::Texture spaceshipTexture;
    if (!spaceshipTexture.loadFromFile("spaceship.png")) return -1;

    std::vector<sf::Texture> enemyTextures(3);
    if (!enemyTextures[0].loadFromFile("enemy1.png") ||
        !enemyTextures[1].loadFromFile("enemy2.png") ||
        !enemyTextures[2].loadFromFile("enemy3.png")) return -1;

    sf::Texture laserTexture;
    if (!laserTexture.loadFromFile("laser.png")) return -1;

    sf::Sprite spaceship;
    spaceship.setTexture(spaceshipTexture);
    spaceship.setScale(1.5f, 1.5f);

    sf::FloatRect spaceshipBounds = spaceship.getLocalBounds();
    spaceship.setPosition(
        (window.getSize().x - spaceshipBounds.width * spaceship.getScale().x) / 2,
        window.getSize().y - spaceshipBounds.height * spaceship.getScale().y
    );

    // Speeds and timings
    const float movementSpeed = 0.6f;
    const float enemySpeed = 0.4f;
    const float laserSpeed = 0.6f;
    const sf::Time spawnInterval = sf::seconds(0.3f);
    const sf::Time shootInterval = sf::seconds(0.2f);

    // Game state
    std::vector<sf::Sprite> enemies;
    std::vector<sf::Sprite> lasers;
    sf::Clock spawnClock;
    sf::Clock shootClock;
    bool gameOver = false;
    bool gameStarted = false;
    int score = 0;

    // Text and fonts
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) return -1;

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(20);
    scoreText.setFillColor(sf::Color::Black);

    sf::Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(30);
    gameOverText.setFillColor(sf::Color::Black);

    sf::Text startText;
    startText.setFont(font);
    startText.setString("Press Enter to Start the Game");
    startText.setCharacterSize(30);
    startText.setFillColor(sf::Color::Black);
    startText.setPosition(
        (window.getSize().x - startText.getLocalBounds().width) / 2,
        (window.getSize().y - startText.getLocalBounds().height) / 2
    );

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
                if (!gameStarted) {
                    gameStarted = true;
                } else if (gameOver) {
                    resetGame(spaceship, enemies, lasers, gameOver, score, spawnClock, shootClock, spawnInterval, shootInterval, window);
                }
            }
        }

        if (gameStarted && !gameOver) {
            // Move spaceship
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) &&
                spaceship.getPosition().x > 0) {
                spaceship.move(-movementSpeed, 0.0f);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) &&
                spaceship.getPosition().x + spaceshipBounds.width * spaceship.getScale().x < window.getSize().x) {
                spaceship.move(movementSpeed, 0.0f);
            }

            // Shoot lasers
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && shootClock.getElapsedTime() > shootInterval) {
                sf::Sprite laser;
                laser.setTexture(laserTexture);
                laser.setPosition(
                    spaceship.getPosition().x + (spaceshipBounds.width * spaceship.getScale().x) / 2 - laser.getLocalBounds().width / 2,
                    spaceship.getPosition().y - laser.getLocalBounds().height
                );
                lasers.push_back(laser);
                shootClock.restart();
            }

            // Move lasers
            for (auto it = lasers.begin(); it != lasers.end();) {
                it->move(0.0f, -laserSpeed);
                if (it->getPosition().y + it->getLocalBounds().height < 0) {
                    it = lasers.erase(it);
                } else {
                    ++it;
                }
            }

            // Spawn enemies
            if (spawnClock.getElapsedTime() > spawnInterval) {
                enemies.push_back(spawnEnemy(enemyTextures, window));
                spawnClock.restart();
            }

            // Move and check collisions for enemies
            for (auto enemyIt = enemies.begin(); enemyIt != enemies.end();) {
                enemyIt->move(0.0f, enemySpeed);

                bool hit = false;
                for (auto laserIt = lasers.begin(); laserIt != lasers.end();) {
                    if (laserIt->getGlobalBounds().intersects(enemyIt->getGlobalBounds())) {
                        laserIt = lasers.erase(laserIt);
                        hit = true;
                        score += 10;
                        break;
                    } else {
                        ++laserIt;
                    }
                }

                if (hit) {
                    enemyIt = enemies.erase(enemyIt);
                } else if (enemyIt->getPosition().y > window.getSize().y) {
                    enemyIt = enemies.erase(enemyIt);
                } else {
                    ++enemyIt;
                }
            }

            // Check collisions between spaceship and enemies
            for (const auto& enemy : enemies) {
                if (enemy.getGlobalBounds().intersects(spaceship.getGlobalBounds())) {
                    gameOver = true;
                    gameOverText.setString("Game Over! Press Enter to Restart\nScore: " + std::to_string(score));
                    gameOverText.setPosition(
                        (window.getSize().x - gameOverText.getLocalBounds().width) / 2,
                        (window.getSize().y - gameOverText.getLocalBounds().height) / 2
                    );
                    break;
                }
            }

            window.clear(sf::Color(255, 192, 203));

            window.draw(spaceship);

            for (const auto& laser : lasers) {
                window.draw(laser);
            }

            for (const auto& enemy : enemies) {
                window.draw(enemy);
            }

            scoreText.setString("Score: " + std::to_string(score));
            window.draw(scoreText);
        } else if (!gameStarted) {
            window.clear(sf::Color(255, 192, 203));
            window.draw(startText);
        } else {
            window.clear(sf::Color(255, 192, 203));
            window.draw(gameOverText);
        }

        window.display();
    }

    return 0;
}
