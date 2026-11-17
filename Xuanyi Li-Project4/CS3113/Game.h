#ifndef GAME_H
#define GAME_H
#include "Scene.h"

struct Session
{
    int lives = 5;
    Sound jumpSound{}, hurtSound{}, exitSound{}, paperSound{};
    int papers = 0; 
};

class Level : public Scene
{
    Session *mSession;
    int mNumber;
    unsigned int mTiles[64 * 10] = {};
    Entity *mEnemies[9] = {};
    int mEnemyCount = 0;
    Entity *mExit = nullptr;
    Entity *mLockedExit = nullptr;
    Entity *mPapers[3] = {};
    Entity *mBackground[4] = {};
    Vector2 mCheckpoint = { 125.0f, 360.0f }; 
    float mGrace = 0.0f;
    bool mLoaded = false;
public:
    Level(int number, Session *session);
    ~Level();
    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

class MessageScene : public Scene
{
    const char *mImage;
    Entity *mPanel = nullptr;
    Entity *mIllustration = nullptr;
public:
    MessageScene(const char *image);
    ~MessageScene();
    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};
#endif
