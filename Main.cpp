#include <SFML/Graphics.hpp>
#include<SFML/Audio.hpp>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <cmath>
#include <string>

using namespace std;

// ---------------- CONSTANTS ----------------
const int ROW_COUNT = 5;
const int COL_COUNT = 9;
const int CELL_SIZE = 120;
const int MENU_HEIGHT = 120;
const int MAX_MENU_ITEMS = 5;
const int MAX_PEAS = 100;
const int MAX_LEADER = 100;
const int MAX_ZOMBIES = 20;
const int MAX_SUNS = 50;

bool lawnMowerActive[ROW_COUNT];
bool lawnMowerUsed[ROW_COUNT];
float lawnMowerX[ROW_COUNT];

// Plant types
#define NONE 0
#define PEASHOOTER 1
#define SUNFLOWER 2
#define CHERRY_BOMB 3
#define WALNUT 4
#define FROZEN_PEA 5

bool cherryUnlocked = false;
bool walnutUnlocked = false;
bool frozenPeaUnlocked = false;

// Zombie types
#define NORMAL_ZOMBIE 1
#define FOOTBALL_ZOMBIE 2
#define DANCING_ZOMBIE 3
#define FLYING_ZOMBIE 4

// Zombie health
const int ZOMBIE_HEALTH_NORMAL = 3;
const int ZOMBIE_HEALTH_FOOTBALL = 10;
const int ZOMBIE_HEALTH_DANCER = 7;
const int ZOMBIE_HEALTH_FLYING = 6;

// Plant health
const int PLANT_HEALTH = 3;
const int WALNUT_HEALTH = 15;
bool soundOn = true;
sf::Music menuMusic;

// Freeze tracking
bool zombieFrozen[ROW_COUNT][COL_COUNT];
sf::Clock freezeTimer[ROW_COUNT][COL_COUNT];

// ---------------- GLOBAL VARIABLES ----------------
int currentLevel = 1;
int zombiesSpawned = 0;
int zombiesKilled = 0;
int zombiesPerLevel = 10;
float zombieSpawnTime = 2.5f;
float zombieMoveTime = 1.0f;
float baseZombieMoveTime = 1.0f;
bool gameRunning = true;

// ---------------- GAME STATE ----------------
int gameState = 0; // 0=Theme, 1=Login, 2=Menu, 3=Game, 4=Pause
int screen = 0;

// ---------------- PLAYER ----------------
char playerName[50] = "";
int playerNameLength = 0;

// ---------------- GAME DATA ----------------
int fieldPlants[ROW_COUNT][COL_COUNT];
int plantHealth[ROW_COUNT][COL_COUNT];
int fieldZombies[ROW_COUNT][COL_COUNT];
int zombieHealth[ROW_COUNT][COL_COUNT];
int zombieType[ROW_COUNT][COL_COUNT];
int sunX[MAX_SUNS], sunY[MAX_SUNS];
bool sunActive[MAX_SUNS];
int activeSuns = 0;
int score = 150;
int selectedPlant = -1;

// ---------------- MENU ----------------
sf::Sprite menuSprite[MAX_MENU_ITEMS];
sf::Texture menuNormal[MAX_MENU_ITEMS];
sf::Texture menuClicked[MAX_MENU_ITEMS];
sf::RectangleShape menuBox[MAX_MENU_ITEMS];
int menuCost[MAX_MENU_ITEMS];

// ---------------- PEAS ----------------
float peaX[MAX_PEAS], peaY[MAX_PEAS];
int peaRow[MAX_PEAS];
bool peaActive[MAX_PEAS];
int peaDamage[MAX_PEAS];
bool peaIsFrozen[MAX_PEAS];

// ---------------- TIMERS ----------------
sf::Clock zombieSpawnClock;
sf::Clock zombieMoveClock;
sf::Clock sunSpawnClock;
sf::Clock zombieAttackClock;
sf::Clock sunflowerClock;
sf::Clock peaShootClock;
sf::Clock cherryBombClock;

// ---------------- STRING FUNCTIONS ----------------
int strLen(char str[])
{
    int len = 0;
    while (str[len] != 0) len++;
    return len;
}

void strCopy(char dest[], char src[])
{
    int i = 0;
    while (src[i] != 0)
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = 0;
}

void intToStr(int num, char str[])
{
    int n = num;
    int i = 0;
    if (num == 0)
    {
        str[0] = '0';
        str[1] = 0;
        return;
    }

    bool neg = false;
    if (num < 0)
    {
        neg = true;
        n = -num;
    }

    char temp[20];
    while (n > 0)
    {
        temp[i++] = (n % 10) + '0';
        n /= 10;
    }

    int k = 0;
    if (neg)
    {
        str[k++] = '-';
    }
    for (int j = i - 1; j >= 0; j--)
    {
        str[k++] = temp[j];
    }
    str[k] = 0;
}

// ---------------- RESET ----------------
void resetBoard()
{
    for (int r = 0; r < ROW_COUNT; r++)
    {
        for (int c = 0; c < COL_COUNT; c++)
        {
            fieldPlants[r][c] = NONE;
            plantHealth[r][c] = 0;
            fieldZombies[r][c] = 0;
            zombieHealth[r][c] = 0;
            zombieType[r][c] = 0;
            zombieFrozen[r][c] = false;
        }
    }

    for (int i = 0; i < MAX_PEAS; i++)
    {
        peaActive[i] = false;
        peaDamage[i] = 1;
        peaIsFrozen[i] = false;
    }

    for (int i = 0; i < MAX_SUNS; i++)
    {
        sunActive[i] = false;
    }

    for (int r = 0; r < ROW_COUNT; r++)
    {
        lawnMowerActive[r] = false;
        lawnMowerUsed[r] = false;
        lawnMowerX[r] = 0;
    }

    zombiesSpawned = 0;
    zombiesKilled = 0;
    activeSuns = 0;
    selectedPlant = -1;
    gameRunning = true;
    score = 150 + (currentLevel - 1) * 25;

    zombieSpawnClock.restart();
    zombieMoveClock.restart();
    sunSpawnClock.restart();
    zombieAttackClock.restart();
    sunflowerClock.restart();
    peaShootClock.restart();
    cherryBombClock.restart();
}

void setupLevel(int level)
{
    currentLevel = level;

    cherryUnlocked = false;
    walnutUnlocked = false;
    frozenPeaUnlocked = false;

    if (level == 1)
    {
        zombiesPerLevel = 5;
        zombieSpawnTime = 3.5f;
        baseZombieMoveTime = 1.5f;
        zombieMoveTime = baseZombieMoveTime;
    }
    else if (level == 2)
    {
        zombiesPerLevel = 12;
        zombieSpawnTime = 2.8f;
        baseZombieMoveTime = 1.2f;
        zombieMoveTime = baseZombieMoveTime;
        cherryUnlocked = true;
    }
    else if (level >= 3)
    {
        zombiesPerLevel = 15;
        zombieSpawnTime = 2.2f;
        baseZombieMoveTime = 0.9f;
        zombieMoveTime = baseZombieMoveTime;
        cherryUnlocked = true;
        walnutUnlocked = true;
        frozenPeaUnlocked = true;
    }

    resetBoard();
}

// ---------------- FILE HANDLING ----------------
void loadLeaderboard(char names[][50], int scores[], int& count)
{
    count = 0;
    ifstream fin("leaderboard.txt");
    if (!fin)
    {
        return;
    }

    char line[100];
    while (fin.getline(line, 100) && count < MAX_LEADER)
    {
        int i = 0, j = 0;
        while (line[i] != ' ' && line[i] != 0)
        {
            names[count][j++] = line[i++];
        }
        names[count][j] = 0;
        if (line[i] == ' ')
        {
            i++;
        }
        int s = 0;
        while (line[i] >= '0' && line[i] <= '9')
        {
            s = s * 10 + (line[i] - '0');
            i++;
        }
        scores[count] = s;
        count++;
    }
    fin.close();
}

void saveLeaderboard(char names[][50], int scores[], int count)
{
    ofstream fout("leaderboard.txt");
    for (int i = 0; i < count; i++)
    {
        fout << names[i] << " " << scores[i] << endl;
    }
    fout.close();
}

void addScore(char name[], int s)
{
    char names[MAX_LEADER][50];
    int scores[MAX_LEADER];
    int count = 0;

    loadLeaderboard(names, scores, count);

    if (count < MAX_LEADER)
    {
        strCopy(names[count], name);
        scores[count] = s;
        count++;
    }

    for (int i = 0; i < count - 1; i++)
    {
        for (int j = 0; j < count - i - 1; j++)
        {
            if (scores[j] < scores[j + 1])
            {
                int tempScore = scores[j];
                scores[j] = scores[j + 1];
                scores[j + 1] = tempScore;

                char tempName[50];
                strCopy(tempName, names[j]);
                strCopy(names[j], names[j + 1]);
                strCopy(names[j + 1], tempName);
            }
        }
    }
    saveLeaderboard(names, scores, count);
}

// ---------------- DRAW LEADERBOARD ----------------
void drawLeaderboard(sf::RenderWindow& w, sf::Font& font)
{
    char names[MAX_LEADER][50];
    int scores[MAX_LEADER];
    int count = 0;
    loadLeaderboard(names, scores, count);

    sf::Text lbText;
    lbText.setFont(font);
    lbText.setCharacterSize(40);
    lbText.setFillColor(sf::Color::Black);

    char title[] = "LEADERBOARD";
    lbText.setString(title);
    lbText.setPosition(400, 20);
    w.draw(lbText);

    for (int i = 0; i < count && i < 10; i++)
    {
        char line[100], num[10];
        intToStr(i + 1, num);
        int k = 0;
        int j = 0;
        while (num[j] != 0)
        {
            line[k++] = num[j++];
        }
        line[k++] = '.';
        line[k++] = ' ';

        j = 0;
        while (names[i][j] != 0)
        {
            line[k++] = names[i][j++];
        }
        line[k++] = ' ';
        line[k++] = '-';
        line[k++] = ' ';

        intToStr(scores[i], num);
        j = 0;
        while (num[j] != 0)
        {
            line[k++] = num[j++];
        }
        line[k] = 0;
        lbText.setString(line);
        lbText.setPosition(300, 100 + i * 50);
        w.draw(lbText);
    }
}

bool levelCleared()
{
    if (zombiesSpawned < zombiesPerLevel)
        return false;

    for (int r = 0; r < ROW_COUNT; r++)
    {
        for (int c = 0; c < COL_COUNT; c++)
        {
            if (fieldZombies[r][c])
                return false;
        }
    }

    return true;
}

bool gameOver()
{
    for (int r = 0; r < ROW_COUNT; r++)
    {
        if (fieldZombies[r][0] && !lawnMowerUsed[r])
        {
            lawnMowerActive[r] = true;
            lawnMowerUsed[r] = true;
            lawnMowerX[r] = 0;
            return false;
        }

        if (fieldZombies[r][0] && lawnMowerUsed[r])
        {
            return true;
        }
    }
    return false;
}

// ---------------- SPAWN SUN ----------------
void spawnSun()
{
    if (activeSuns >= MAX_SUNS)
        return;

    for (int i = 0; i < MAX_SUNS; i++)
    {
        if (!sunActive[i])
        {
            sunActive[i] = true;
            sunX[i] = 50 + rand() % (COL_COUNT * CELL_SIZE - 100);
            sunY[i] = MENU_HEIGHT + 50 + rand() % (ROW_COUNT * CELL_SIZE - 100);
            activeSuns++;
            break;
        }
    }
}

// ---------------- SUNFLOWER LOGIC ----------------
void sunflowerLogic()
{
    if (sunflowerClock.getElapsedTime().asSeconds() > 6.0f) // every 6 seconds
    {
        for (int r = 0; r < ROW_COUNT; r++)
        {
            for (int c = 0; c < COL_COUNT; c++)
            {
                if (fieldPlants[r][c] == SUNFLOWER)
                {
                    // find free sun slot
                    for (int i = 0; i < MAX_SUNS; i++)
                    {
                        if (!sunActive[i])
                        {
                            sunActive[i] = true;

                            // place sun on sunflower tile
                            sunX[i] = c * CELL_SIZE + 35;
                            sunY[i] = r * CELL_SIZE + MENU_HEIGHT + 35;

                            activeSuns++;
                            break;
                        }
                    }
                }
            }
        }
        sunflowerClock.restart();
    }
}


// ---------------- CHERRY BOMB LOGIC ----------------
void cherryBombLogic()
{
    if (cherryBombClock.getElapsedTime().asSeconds() > 1.5f)
    {
        for (int r = 0; r < ROW_COUNT; r++)
        {
            for (int c = 0; c < COL_COUNT; c++)
            {
                if (fieldPlants[r][c] == CHERRY_BOMB)
                {
                    for (int i = r - 1; i <= r + 1; i++)
                    {
                        for (int j = c - 1; j <= c + 1; j++)
                        {
                            if (i >= 0 && i < ROW_COUNT && j >= 0 && j < COL_COUNT)
                            {
                                if (fieldZombies[i][j])
                                {
                                    fieldZombies[i][j] = 0;
                                    zombieHealth[i][j] = 0;
                                    zombieType[i][j] = 0;
                                    zombieFrozen[i][j] = false;
                                    zombiesKilled++;
                                    score += 50;
                                }
                            }
                        }
                    }
                    fieldPlants[r][c] = NONE;
                    plantHealth[r][c] = 0;
                }
            }
        }
        cherryBombClock.restart();
    }
}

// ---------------- ZOMBIE SPAWNING ----------------
void spawnZombie()
{
    if (zombiesSpawned >= zombiesPerLevel || !gameRunning)
        return;

    if (zombieSpawnClock.getElapsedTime().asSeconds() > zombieSpawnTime)
    {
        int row = rand() % ROW_COUNT;
        int type = NORMAL_ZOMBIE;

        // ⬇️ THIS SECTION CONTROLS WHICH ZOMBIES APPEAR AT EACH LEVEL ⬇️
        if (currentLevel == 1)
            type = NORMAL_ZOMBIE;
        else if (currentLevel == 2)
        {
            type = (rand() % 2 == 0) ? NORMAL_ZOMBIE : DANCING_ZOMBIE;
        }
        else if (currentLevel >= 3)
        {
            int r = rand() % 4;
            if (r == 0) type = NORMAL_ZOMBIE;
            else if (r == 1) type = DANCING_ZOMBIE;
            else if (r == 2) type = FOOTBALL_ZOMBIE;
            else type = FLYING_ZOMBIE;
        }
        // ⬆️ END OF ZOMBIE TYPE SELECTION ⬆️

        if (fieldZombies[row][COL_COUNT - 1] == 0)
        {
            fieldZombies[row][COL_COUNT - 1] = 1;
            zombieType[row][COL_COUNT - 1] = type;
            zombieFrozen[row][COL_COUNT - 1] = false;

            if (type == NORMAL_ZOMBIE) zombieHealth[row][COL_COUNT - 1] = ZOMBIE_HEALTH_NORMAL;
            if (type == DANCING_ZOMBIE) zombieHealth[row][COL_COUNT - 1] = ZOMBIE_HEALTH_DANCER;
            if (type == FOOTBALL_ZOMBIE) zombieHealth[row][COL_COUNT - 1] = ZOMBIE_HEALTH_FOOTBALL;
            if (type == FLYING_ZOMBIE) zombieHealth[row][COL_COUNT - 1] = ZOMBIE_HEALTH_FLYING;

            zombiesSpawned++;
            zombieSpawnClock.restart();
        }
    }
}

// ---------------- MOVE ZOMBIES ----------------
void moveZombies()
{
    if (!gameRunning) return;

    if (zombieMoveClock.getElapsedTime().asSeconds() > zombieMoveTime)
    {
        for (int r = 0; r < ROW_COUNT; r++)
        {
            for (int c = 1; c < COL_COUNT; c++)
            {
                if (fieldZombies[r][c])
                {
                    if (zombieFrozen[r][c])
                    {
                        if (freezeTimer[r][c].getElapsedTime().asSeconds() > 5.0f)
                        {
                            zombieFrozen[r][c] = false;
                        }
                        else
                        {
                            continue;
                        }
                    }

                    if (fieldPlants[r][c - 1] != NONE || fieldZombies[r][c - 1])
                        continue;

                    fieldZombies[r][c - 1] = 1;
                    fieldZombies[r][c] = 0;

                    zombieHealth[r][c - 1] = zombieHealth[r][c];
                    zombieHealth[r][c] = 0;

                    zombieType[r][c - 1] = zombieType[r][c];
                    zombieType[r][c] = 0;

                    zombieFrozen[r][c - 1] = zombieFrozen[r][c];
                    zombieFrozen[r][c] = false;

                    if (zombieFrozen[r][c - 1])
                    {
                        freezeTimer[r][c - 1] = freezeTimer[r][c];
                    }
                }
            }
        }
        zombieMoveClock.restart();
    }
}

// ---------------- ZOMBIE ATTACK PLANTS ----------------
void zombieAttackPlants()
{
    if (!gameRunning) return;

    if (zombieAttackClock.getElapsedTime().asSeconds() > 1.0f)
    {
        for (int r = 0; r < ROW_COUNT; r++)
        {
            for (int c = 0; c < COL_COUNT; c++)
            {
                if (fieldZombies[r][c] && c > 0 && fieldPlants[r][c - 1] != NONE)
                {
                    plantHealth[r][c - 1]--;
                    if (plantHealth[r][c - 1] <= 0)
                    {
                        fieldPlants[r][c - 1] = NONE;
                        plantHealth[r][c - 1] = 0;
                    }
                }
            }
        }
        zombieAttackClock.restart();
    }
}

// ---------------- SHOOT PEAS ----------------
void shootPeas()
{
    if (!gameRunning) return;

    if (peaShootClock.getElapsedTime().asSeconds() > 1.5f)
    {
        for (int r = 0; r < ROW_COUNT; r++)
        {
            for (int c = 0; c < COL_COUNT; c++)
            {
                if (fieldPlants[r][c] == PEASHOOTER || fieldPlants[r][c] == FROZEN_PEA)
                {
                    bool zombieInLane = false;
                    for (int zc = c + 1; zc < COL_COUNT; zc++)
                    {
                        if (fieldZombies[r][zc])
                        {
                            zombieInLane = true;
                            break;
                        }
                    }

                    if (zombieInLane)
                    {
                        for (int i = 0; i < MAX_PEAS; i++)
                        {
                            if (!peaActive[i])
                            {
                                peaActive[i] = true;
                                peaX[i] = c * CELL_SIZE + 80;
                                peaY[i] = r * CELL_SIZE + MENU_HEIGHT + 50;
                                peaRow[i] = r;
                                peaDamage[i] = 1;
                                peaIsFrozen[i] = (fieldPlants[r][c] == FROZEN_PEA);
                                break;
                            }
                        }
                    }
                }
            }
        }
        peaShootClock.restart();
    }
}

// ---------------- UPDATE PEAS ----------------
void updatePeas(float dt)
{
    if (!gameRunning) return;

    for (int i = 0; i < MAX_PEAS; i++)
    {
        if (peaActive[i])
        {
            peaX[i] += 400 * dt;

            for (int c = 0; c < COL_COUNT; c++)
            {
                if (fieldZombies[peaRow[i]][c])
                {
                    float zombieX = c * CELL_SIZE;
                    if (peaX[i] >= zombieX && peaX[i] <= zombieX + CELL_SIZE)
                    {
                        if (peaIsFrozen[i])
                        {
                            zombieFrozen[peaRow[i]][c] = true;
                            freezeTimer[peaRow[i]][c].restart();
                        }

                        zombieHealth[peaRow[i]][c] -= peaDamage[i];

                        if (zombieHealth[peaRow[i]][c] <= 0)
                        {
                            fieldZombies[peaRow[i]][c] = 0;
                            zombieHealth[peaRow[i]][c] = 0;
                            zombieType[peaRow[i]][c] = 0;
                            zombieFrozen[peaRow[i]][c] = false;
                            zombiesKilled++;
                            score += 50;
                        }

                        peaActive[i] = false;
                        break;
                    }
                }
            }

            if (peaX[i] > COL_COUNT * CELL_SIZE)
            {
                peaActive[i] = false;
            }
        }
    }
}

// ---------------- UPDATE SUNS ----------------
void updateSuns()
{
    if (sunSpawnClock.getElapsedTime().asSeconds() > 10.0f)
    {
        spawnSun();
        sunSpawnClock.restart();
    }
}

// ---------------- CHECK COLLECT SUN ----------------
void checkSunCollection(sf::Vector2i mousePos)
{
    for (int i = 0; i < MAX_SUNS; i++)
    {
        if (sunActive[i])
        {
            if (mousePos.x >= sunX[i] && mousePos.x <= sunX[i] + 50 &&
                mousePos.y >= sunY[i] && mousePos.y <= sunY[i] + 50)
            {
                sunActive[i] = false;
                activeSuns--;
                score += 25;
            }
        }
    }
}

// ---------------- UPDATE LAWN MOWERS (SINGLE DEFINITION) ----------------
void updateLawnMowers(float dt)
{
    for (int r = 0; r < ROW_COUNT; r++)
    {
        if (lawnMowerActive[r])
        {
            lawnMowerX[r] += 500 * dt;

            for (int c = 0; c < COL_COUNT; c++)
            {
                float zx = c * CELL_SIZE;
                if (fieldZombies[r][c] && lawnMowerX[r] >= zx)
                {
                    fieldZombies[r][c] = 0;
                    zombieHealth[r][c] = 0;
                    zombieType[r][c] = 0;
                    zombiesKilled++;
                }
            }

            if (lawnMowerX[r] > COL_COUNT * CELL_SIZE)
            {
                lawnMowerActive[r] = false;
            }
        }
    }
}

// ---------------- EVENTS ----------------
void handleEvents(sf::RenderWindow& window, sf::Text& startText, sf::Text& nameText,
    sf::Text menuText[], sf::Text& backText, sf::Text& levelText,
    sf::Text& scoreText)
{
    sf::Event e;
    sf::FloatRect soundBtn(300, 300, 300, 70);
    sf::FloatRect exitBtn(300, 400, 300, 70);

    while (window.pollEvent(e))
    {
        if (e.type == sf::Event::Closed)
        {
            window.close();
        }

        if (gameState == 0 && e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2i p = sf::Mouse::getPosition(window);
            if (startText.getGlobalBounds().contains(p.x, p.y))
            {
                gameState = 1;
            }
        }

        if (gameState == 1)
        {
            if (e.type == sf::Event::TextEntered)
            {
                if (e.text.unicode == 8 && playerNameLength > 0)
                {
                    playerName[--playerNameLength] = 0;
                }
                else if (e.text.unicode >= 32 && e.text.unicode <= 126 && playerNameLength < 49)
                {
                    playerName[playerNameLength++] = (char)e.text.unicode;
                    playerName[playerNameLength] = 0;
                }
                nameText.setString(playerName);
            }

            if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Enter && playerNameLength > 0)
            {
                gameState = 2;
                screen = 0;
            }
        }

        if (gameState == 2 && e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2i p = sf::Mouse::getPosition(window);
            if (menuText[0].getGlobalBounds().contains(p.x, p.y))
            {
                gameState = 3;
                setupLevel(1);
            }
            else if (menuText[1].getGlobalBounds().contains(p.x, p.y))
            {
                screen = 1;
            }
            else if (menuText[2].getGlobalBounds().contains(p.x, p.y))
            {
                screen = 2;
            }
            else if (menuText[3].getGlobalBounds().contains(p.x, p.y))
            {
                screen = 3;
            }
            else if (menuText[4].getGlobalBounds().contains(p.x, p.y))
            {
                window.close();
            }
        }

        if (gameState == 2 && screen != 0)
        {
            if ((e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left))
            {
                sf::Vector2i p = sf::Mouse::getPosition(window);
                if (backText.getGlobalBounds().contains(p.x, p.y))
                {
                    screen = 0;
                }
            }
            else if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape)
            {
                screen = 0;
            }
        }

        if (gameState == 3 && e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2i p = sf::Mouse::getPosition(window);

            checkSunCollection(p);

            if (p.y < MENU_HEIGHT)
            {
                for (int i = 0; i < MAX_MENU_ITEMS; i++)
                {
                    if (menuBox[i].getGlobalBounds().contains(p.x, p.y))
                    {
                        bool canSelect = false;

                        // Check if plant is unlocked
                        if (i == 0 || i == 1)
                            canSelect = true;  // Always available
                        else if (i == 2 && cherryUnlocked)
                            canSelect = true;
                        else if (i == 3 && walnutUnlocked)
                            canSelect = true;
                        else if (i == 4 && frozenPeaUnlocked)
                            canSelect = true;

                        // Only select if unlocked AND affordable
                        if (canSelect && score >= menuCost[i])
                        {
                            selectedPlant = i + 1;
                        }
                    }
                }
            }
            else
            {
                int col = p.x / CELL_SIZE;
                int row = (p.y - MENU_HEIGHT) / CELL_SIZE;

                if (row >= 0 && row < ROW_COUNT && col >= 0 && col < COL_COUNT)
                {
                    if (selectedPlant != -1 && fieldPlants[row][col] == NONE && score >= menuCost[selectedPlant - 1])
                    {
                        // FIXED: Complete plant placement logic
                        fieldPlants[row][col] = selectedPlant;

                        // Set plant health
                        if (selectedPlant == WALNUT)
                        {
                            plantHealth[row][col] = WALNUT_HEALTH;
                        }
                        else
                        {
                            plantHealth[row][col] = PLANT_HEALTH;
                        }

                        score -= menuCost[selectedPlant - 1];
                        selectedPlant = -1;
                    }
                }
            }
        }

        if (gameState == 3 && e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::P)
        {
            gameState = 4; // Pause
        }
        else if (gameState == 4 && e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::P)
        {
            // Resume
            zombieSpawnClock.restart();
            zombieMoveClock.restart();
            zombieAttackClock.restart();
            sunflowerClock.restart();
            peaShootClock.restart();
            sunSpawnClock.restart();

            gameState = 3;
        }

        if (gameState == 4 && e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2i p = sf::Mouse::getPosition(window);

            // SOUND TOGGLE
            if (soundBtn.contains(p.x, p.y))
            {
                soundOn = !soundOn;
            }

            // EXIT GAME
            else if (exitBtn.contains(p.x, p.y))
            {
                addScore(playerName, score);
                gameState = 2;
                screen = 0;
                gameRunning = false;
            }
        }
    }
}

// ---------------- DRAW GAME ----------------
void drawGame(
    sf::RenderWindow& w,
    sf::Font& font,
    sf::Sprite& grid,
    sf::Sprite& peaPlant,
    sf::Sprite& sunflower,
    sf::Sprite& sun,
    sf::Sprite& lawnMower,
    sf::Sprite& pea,
    sf::Text& levelText,
    sf::Text& scoreText,
    sf::Sprite& normalZombie,
    sf::Sprite& footballZombie,
    sf::Sprite& dancingZombie,
    sf::Sprite& flyingZombie,
    sf::Sprite& cherry,
    sf::Sprite& walnut,
    sf::Sprite& frozenPea,
    sf::Text& backText
)
{
    // Draw menu background
    sf::RectangleShape menuBg(sf::Vector2f(w.getSize().x, MENU_HEIGHT));
    menuBg.setFillColor(sf::Color(100, 150, 100));
    w.draw(menuBg);

    // Draw menu items
    for (int i = 0; i < MAX_MENU_ITEMS; i++)
    {
        menuSprite[i].setTexture(selectedPlant == (i + 1) ? menuClicked[i] : menuNormal[i]);
        menuSprite[i].setPosition(menuBox[i].getPosition());
        w.draw(menuBox[i]);
        w.draw(menuSprite[i]);

        // Draw cost
        sf::Text costText;
        costText.setFont(font);
        costText.setCharacterSize(20);
        costText.setFillColor(sf::Color::Yellow);
        costText.setString(to_string(menuCost[i]));
        costText.setPosition(menuBox[i].getPosition().x + 5, menuBox[i].getPosition().y + 5);
        w.draw(costText);
    }

    // Draw game grid
    for (int r = 0; r < ROW_COUNT; r++)
    {
        for (int c = 0; c < COL_COUNT; c++)
        {
            float x = c * CELL_SIZE;
            float y = r * CELL_SIZE + MENU_HEIGHT;
            grid.setPosition(x, y);
            w.draw(grid);

            // Draw plants - FIXED: Added all plant types
            if (fieldPlants[r][c] == PEASHOOTER)
            {
                peaPlant.setPosition(x, y);
                w.draw(peaPlant);
            }
            else if (fieldPlants[r][c] == SUNFLOWER)
            {
                sunflower.setPosition(x, y);
                w.draw(sunflower);
            }
            else if (fieldPlants[r][c] == CHERRY_BOMB)
            {
                cherry.setPosition(x, y);
                w.draw(cherry);
            }
            else if (fieldPlants[r][c] == WALNUT)
            {
                walnut.setPosition(x, y);
                w.draw(walnut);
            }
            else if (fieldPlants[r][c] == FROZEN_PEA)
            {
                frozenPea.setPosition(x, y);
                w.draw(frozenPea);
            }

            // Draw zombies
            if (fieldZombies[r][c])
            {
                switch (zombieType[r][c])
                {
                case NORMAL_ZOMBIE:
                    normalZombie.setPosition(x, y);
                    w.draw(normalZombie);
                    break;
                case FOOTBALL_ZOMBIE:
                    footballZombie.setPosition(x, y);
                    w.draw(footballZombie);
                    break;
                case DANCING_ZOMBIE:
                    dancingZombie.setPosition(x, y);
                    w.draw(dancingZombie);
                    break;
                case FLYING_ZOMBIE:
                    flyingZombie.setPosition(x, y);
                    w.draw(flyingZombie);
                    break;
                default:
                    normalZombie.setPosition(x, y);
                    w.draw(normalZombie);
                    break;
                }
            }
        }
    }

    // Draw lawn mowers
    for (int r = 0; r < ROW_COUNT; r++)
    {
        if (!lawnMowerUsed[r] || lawnMowerActive[r])
        {
            lawnMower.setPosition(
                lawnMowerActive[r] ? lawnMowerX[r] : 0,
                r * CELL_SIZE + MENU_HEIGHT + 20
            );
            w.draw(lawnMower);
        }
    }

    // Draw peas
    for (int i = 0; i < MAX_PEAS; i++)
    {
        if (peaActive[i])
        {
            pea.setPosition(peaX[i], peaY[i]);
            w.draw(pea);
        }
    }

    // Draw suns
    for (int i = 0; i < MAX_SUNS; i++)
    {
        if (sunActive[i])
        {
            sun.setPosition(sunX[i], sunY[i]);
            w.draw(sun);
        }
    }

    // Draw UI text
    char levelStr[50];
    intToStr(currentLevel, levelStr);
    char levelFull[50] = "Level: ";
    strCopy(levelFull + 7, levelStr);
    levelText.setString(levelFull);
    levelText.setPosition(10, 10);
    w.draw(levelText);

    char scoreStr[50];
    intToStr(score, scoreStr);
    char scoreFull[50] = "Score: ";
    strCopy(scoreFull + 7, scoreStr);
    scoreText.setString(scoreFull);
    scoreText.setPosition(10, 50);
    w.draw(scoreText);

    // Draw sun count
    char sunStr[50];
    intToStr(score, sunStr);
    char sunFull[50] = "Suns: ";
    strCopy(sunFull + 6, sunStr);

    sf::Text sunText;
    sunText.setFont(font);
    sunText.setString(sunFull);
    sunText.setCharacterSize(30);
    sunText.setFillColor(sf::Color::Yellow);
    sunText.setPosition(10, 90);
    w.draw(sunText);

    // Draw back button
    w.draw(backText);
}

// ---------------- MAIN ----------------
int main()
{
    srand(time(0));
    sf::RenderWindow window(sf::VideoMode(COL_COUNT * CELL_SIZE, ROW_COUNT * CELL_SIZE + MENU_HEIGHT), "Plants vs Zombies");

    float winW = COL_COUNT * CELL_SIZE;
    float winH = ROW_COUNT * CELL_SIZE + MENU_HEIGHT;

    // Theme screen
    sf::Texture themeT;
    if (!themeT.loadFromFile("Images/mainMenu.png"))
    {
        themeT.create(900, 700);
    }
    sf::Sprite theme(themeT);
    float sX = winW / themeT.getSize().x;
    float sY = winH / themeT.getSize().y;
    float s = (sX < sY) ? sX : sY;
    theme.setScale(s, s);
    theme.setPosition((winW - themeT.getSize().x * s) / 2, (winH - themeT.getSize().y * s) / 2);

    // Login screen
    sf::Texture loginT;
    if (!loginT.loadFromFile("Images/login_screen.jpg"))
    {
        loginT.create(900, 700);
    }
    sf::Sprite loginBg(loginT);
    loginBg.setScale(s, s);
    loginBg.setPosition(theme.getPosition());

    sf::Font font;
    if (!font.loadFromFile("Poppins-Bold.otf"))
    {
        if (!font.loadFromFile("C:/Windows/Fonts/Poppins-Bold.otf"))
        {
            return 0;
        }
    }

    sf::Text startText;
    startText.setFont(font);
    startText.setString("START GAME");
    startText.setCharacterSize(50);
    startText.setFillColor(sf::Color::Black);
    startText.setOrigin(startText.getLocalBounds().width / 2, startText.getLocalBounds().height / 2);
    startText.setPosition(winW / 2, winH * 0.75f);

    sf::Text nameText;
    nameText.setFont(font);
    nameText.setString("");
    nameText.setCharacterSize(40);
    nameText.setFillColor(sf::Color::Black);
    nameText.setPosition(310,390);

    // Game textures
    sf::Texture gridT, peaT, sunT, sunflowerT, peaProjT, menuT, leaderT, instT, optionsT;
    sf::Texture normalZT, footballZT, cherryT, walnutT, frozenPeaT, flyingZT, dancingZT;

    // Load textures with fallbacks
    if (!gridT.loadFromFile("Images/lawn_day.png")) 
        gridT.create(CELL_SIZE, CELL_SIZE);
    if (!peaT.loadFromFile("Images/peashooter.png"))
        peaT.create(CELL_SIZE, CELL_SIZE);
    if (!sunflowerT.loadFromFile("Images/sunflower.png"))
        sunflowerT.create(CELL_SIZE, CELL_SIZE);
    if (!sunT.loadFromFile("Images/sunlight.png")) 
        sunT.create(50, 50);
    if (!peaProjT.loadFromFile("Images/pea.png")) 
        peaProjT.create(20, 20);
    if (!menuT.loadFromFile("Images/menu.jpg")) 
        menuT.create(900, 700);
    if (!leaderT.loadFromFile("Images/Leaderboard.png")) 
        leaderT.create(900, 700);
    if (!instT.loadFromFile("Images/instructions.png")) 
        instT.create(900, 700);
    if (!optionsT.loadFromFile("Images/options.jpg")) 
        optionsT.create(900, 700);
    if (!normalZT.loadFromFile("Images/simplezombie.png"))
        normalZT.create(CELL_SIZE, CELL_SIZE);
    if (!footballZT.loadFromFile("Images/footballzombie.png")) 
        footballZT.create(CELL_SIZE, CELL_SIZE);
    if (!flyingZT.loadFromFile("Images/flyingzombie.png"))
        flyingZT.create(CELL_SIZE, CELL_SIZE);
    if (!flyingZT.loadFromFile("Images/dancingzombie.png"))
        dancingZT.create(CELL_SIZE, CELL_SIZE);
    if (!cherryT.loadFromFile("Images/cherrybomb.png")) 
        cherryT.create(CELL_SIZE, CELL_SIZE);
    if (!walnutT.loadFromFile("Images/walnut.png"))
        walnutT.create(CELL_SIZE, CELL_SIZE);
    if (!frozenPeaT.loadFromFile("Images/frozenpea.png")) 
        frozenPeaT.create(CELL_SIZE, CELL_SIZE);

    sf::Sprite grid(gridT), peaPlant(peaT), sunflower(sunflowerT), menu(menuT),
        sun(sunT), pea(peaProjT), normalZombie(normalZT), footballZombie(footballZT),
        dancingZombie(dancingZT), cherry(cherryT), walnut(walnutT), frozenPea(frozenPeaT), flyingZombie(flyingZT);
    sf::Sprite leaderboard(leaderT), instructions(instT), options(optionsT);

    // Scale sprites
    float lbS = (winW / leaderT.getSize().x < winH / leaderT.getSize().y) ? (winW / leaderT.getSize().x) : (winH / leaderT.getSize().y);
    leaderboard.setScale(lbS, lbS);
    leaderboard.setPosition((winW - leaderT.getSize().x * lbS) / 2, (winH - leaderT.getSize().y * lbS) / 2);

    float inS = (winW / instT.getSize().x < winH / instT.getSize().y) ? (winW / instT.getSize().x) : (winH / instT.getSize().y);
    instructions.setScale(inS, inS);
    instructions.setPosition((winW - instT.getSize().x * inS) / 2, (winH - instT.getSize().y * inS) / 2);

    float opS = (winW / optionsT.getSize().x < winH / optionsT.getSize().y) ? (winW / optionsT.getSize().x) : (winH / optionsT.getSize().y);
    options.setScale(opS, opS);
    options.setPosition((winW - optionsT.getSize().x * opS) / 2, (winH - optionsT.getSize().y * opS) / 2);

    sf::Texture lawnMowerT;
    if (!lawnMowerT.loadFromFile("Images/lawnmower.png"))
    {
        lawnMowerT.create(100, 80);
    }
    sf::Sprite lawnMower(lawnMowerT);

    sf::Texture pauseT;
    if (!pauseT.loadFromFile("Images/pausemenu.png"))
    {
        pauseT.create(800, 600);
    }
    sf::Sprite pauseMenu(pauseT);

    //if (!menuMusic.openFromFile("menu.ogg"))
    //{
    //    // failed to load, silently ignore
    //}
    //menuMusic.setLoop(true);


    float pSx = winW / pauseT.getSize().x;
    float pSy = winH / pauseT.getSize().y;
    float pS = (pSx < pSy) ? pSx : pSy;
    pauseMenu.setScale(pS, pS);
    pauseMenu.setPosition((winW - pauseT.getSize().x * pS) / 2, (winH - pauseT.getSize().y * pS) / 2);

    // FIXED: Load all menu item textures
    if (!menuNormal[0].loadFromFile("Images/peashooterpack.png")) 
        menuNormal[0].create(CELL_SIZE, CELL_SIZE);
    if (!menuClicked[0].loadFromFile("Images/peashooterpack_clicked.png"))
        menuClicked[0].create(CELL_SIZE, CELL_SIZE);
    if (!menuNormal[1].loadFromFile("Images/sunflowerpack.png")) 
        menuNormal[1].create(CELL_SIZE, CELL_SIZE);
    if (!menuClicked[1].loadFromFile("Images/sunflowerpack_clicked.png"))
        menuClicked[1].create(CELL_SIZE, CELL_SIZE);
    if (!menuNormal[2].loadFromFile("Images/cherrybombpack.png")) 
        menuNormal[2].create(CELL_SIZE, CELL_SIZE);
    if (!menuClicked[2].loadFromFile("Images/cherrybombpack_clicked.png")) 
        menuClicked[2].create(CELL_SIZE, CELL_SIZE);
    if (!menuNormal[3].loadFromFile("Images/walnutpack.png")) 
        menuNormal[3].create(CELL_SIZE, CELL_SIZE);
    if (!menuClicked[3].loadFromFile("Images/walnutpack_clicked.png"))
        menuClicked[3].create(CELL_SIZE, CELL_SIZE);
    if (!menuNormal[4].loadFromFile("Images/snowpeapack.png"))
        menuNormal[4].create(CELL_SIZE, CELL_SIZE);
    if (!menuClicked[4].loadFromFile("Images/snowpeapack_clicked.png")) 
        menuClicked[4].create(CELL_SIZE, CELL_SIZE);

    // Back text
    sf::Text backText;
    backText.setFont(font);
    backText.setString("BACK (ESC)");
    backText.setCharacterSize(30);
    backText.setFillColor(sf::Color::Black);
    backText.setPosition(winW - 150, 10);

    // Level and score text
    sf::Text levelText;
    levelText.setFont(font);
    levelText.setCharacterSize(30);
    levelText.setFillColor(sf::Color::Black);

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(30);
    scoreText.setFillColor(sf::Color::Black);

    // Menu items
    char menuLabels[5][20] = { "START GAME", "LEADERBOARD", "INSTRUCTIONS", "OPTIONS", "EXIT" };
    sf::Text menuText[5];
    for (int i = 0; i < 5; i++)
    {
        menuText[i].setFont(font);
        menuText[i].setString(menuLabels[i]);
        menuText[i].setCharacterSize(40);
        menuText[i].setFillColor(sf::Color::Black);
        menuText[i].setPosition(winW / 2 - 50, 200 + i * 80);
    }

    // Menu boxes for plant selection
    for (int i = 0; i < MAX_MENU_ITEMS; i++)
    {
        menuBox[i].setSize(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        menuBox[i].setPosition(i * (CELL_SIZE + 10), 0);
        menuBox[i].setFillColor(sf::Color(150, 150, 150, 100));
    }

    // FIXED: Set costs for all plants
    menuCost[0] = 100;  // Peashooter
    menuCost[1] = 50;   // Sunflower
    menuCost[2] = 150;  // Cherry Bomb
    menuCost[3] = 50;   // Walnut
    menuCost[4] = 175;  // Frozen Pea

    // Game clock
    sf::Clock gameClock;

    while (window.isOpen())
    {
        float deltaTime = gameClock.restart().asSeconds();

        handleEvents(window, startText, nameText, menuText, backText, levelText, scoreText);

        window.clear(sf::Color::Black);

        if (gameState == 0)
        {
            window.draw(theme);
            window.draw(startText);
        }
        else if (gameState == 1)
        {
            window.draw(loginBg);
            sf::Text instruction;
            instruction.setFont(font);
            instruction.setCharacterSize(30);
            instruction.setFillColor(sf::Color::Black);
            instruction.setPosition(900, 540);
            window.draw(instruction);
            window.draw(nameText);
        }
        else if (gameState == 2)
        {
            window.draw(menu);

            if (screen == 0)
            {
                for (int i = 0; i < 5; i++)
                {
                    window.draw(menuText[i]);
                }
            }
            else if (screen == 1)
            {
                window.draw(leaderboard);
                drawLeaderboard(window, font);
                window.draw(backText);
            }
            else if (screen == 2)
            {
                window.draw(instructions);
                window.draw(backText);
            }
            else if (screen == 3)
            {
                window.draw(options);
                window.draw(backText);
            }
        }
        else if (gameState == 3)
        {
            if (gameRunning)
            {
                // FIXED: Added cherry bomb logic
                spawnZombie();
                moveZombies();
                zombieAttackPlants();
                shootPeas();
                updateLawnMowers(deltaTime);
                updatePeas(deltaTime);
                updateSuns();
                sunflowerLogic();
                cherryBombLogic();  // ADDED

                // Check game conditions
                if (levelCleared())
                {
                    if (currentLevel < 3)
                    {
                        currentLevel++;
                        setupLevel(currentLevel);
                    }
                    else
                    {
                        addScore(playerName, score);
                        gameState = 2;
                        screen = 0;
                    }
                }

                if (gameOver())
                {
                    addScore(playerName, score);
                    gameRunning = false;
                    gameState = 2;
                    screen = 0;
                }
            }

            // FIXED: Updated drawGame call with new sprites
            drawGame(window, font, grid, peaPlant, sunflower, sun, lawnMower, pea,
                levelText, scoreText, normalZombie, footballZombie,flyingZombie,
                dancingZombie, cherry, walnut, frozenPea, backText);
        }
        else if (gameState == 4)
        {
            window.draw(pauseMenu);

            sf::Text soundText;
            soundText.setFont(font);
            soundText.setCharacterSize(40);
            soundText.setFillColor(sf::Color::Black);
            soundText.setString(soundOn ? "Sound: ON" : "Sound: OFF");
            soundText.setPosition(330, 310);
            window.draw(soundText);

            sf::Text exitText;
            exitText.setFont(font);
            exitText.setCharacterSize(40);
            exitText.setFillColor(sf::Color::Black);
            exitText.setString("EXIT GAME");
            exitText.setPosition(350, 410);
            window.draw(exitText);
        }

        // -------- MENU MUSIC CONTROL --------
        if (soundOn && (gameState == 0 || gameState == 1 || gameState == 2))
        {
            if (menuMusic.getStatus() != sf::Music::Playing)
                menuMusic.play();
        }
        else
        {
            if (menuMusic.getStatus() == sf::Music::Playing)
                menuMusic.stop();
        }


        window.display();
    }

    return 0;
}