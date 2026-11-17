#include "Game.h"

Level::Level(int number, Session *session)
    : Scene({ 1600.0f, 250.0f }, "#EFE4CC"), mSession(session), mNumber(number) {}
Level::~Level() { shutdown(); }

void Level::initialise()
{
    mLoaded = true;
    mGameState.nextSceneID = NO_SCENE;
    mGrace = 0.0f;
    mSession->papers = 0;
    mCheckpoint = { 125.0f, 360.0f };
    for (int i = 0; i < 640; i++) mTiles[i] = 0;

    for (int x = 0; x < 64; x++)
    {
        mTiles[8 * 64 + x] = 1;
        mTiles[9 * 64 + x] = 2;
    }
    for (int y = 0; y < 8; y++) mTiles[y * 64] = 2;

    int gapWidth = mNumber == 1 ? 2 : 3;
    for (int gap = 0; gap < mNumber + 1; gap++)
    {
        int start = 16 + gap * 14;
        int currentWidth = mNumber == 3 && gap >= 2 ? 4 : gapWidth;
        for (int x = start; x < start + currentWidth; x++)
        {
            mTiles[8 * 64 + x] = 0;
            mTiles[9 * 64 + x] = 0;
        }
    }

    for (int x = 8; x < 11; x++) mTiles[6 * 64 + x] = 1;
    for (int x = 24; x < 27; x++) mTiles[6 * 64 + x] = 1;
    for (int x = 38; x < 41; x++) mTiles[6 * 64 + x] = 1;
    for (int x = 54; x < 57; x++) mTiles[7 * 64 + x] = 1;

    if (mNumber >= 2)
        for (int x = 27; x < 29; x++) mTiles[4 * 64 + x] = 1;
    if (mNumber == 3)
    {
        for (int x = 41; x < 43; x++) mTiles[4 * 64 + x] = 1;
    }
    const char *tiles[] = { "assets/dorm_tiles.png", "assets/subway_tiles.png", "assets/library_tiles.png" };
    const char *walls[] = { "assets/dorm.png", "assets/subway.png", "assets/library.png" };
    mGameState.map = new Map(64, 10, mTiles, tiles[mNumber - 1], 50.0f, 2, 1, mOrigin);

    for (int i = 0; i < 4; i++)
        mBackground[i] = new Entity({400.0f + i * 800.0f, 300.0f}, {800, 600}, walls[mNumber - 1], BLOCK);
    Vector2 paperPositions[3] = { {475, 268}, {1275, 268}, {1975, 268} };
    if (mNumber >= 2) paperPositions[1] = {1400, 168};
    if (mNumber == 3) paperPositions[2] = {2100, 168};
    for (int i = 0; i < 3; i++)
    {
        mPapers[i] = new Entity(paperPositions[i], {30, 34}, "assets/homework.png", BLOCK);
        mPapers[i]->setAngle(i == 1 ? 7.0f : -7.0f);
    }

    std::map<Direction, std::vector<int>> atlas = {
        { DOWN, { 0, 1, 2, 3 } }, { LEFT, { 4, 5, 6, 7 } },
        { UP, { 8, 9, 10, 11 } }, { RIGHT, { 12, 13, 14, 15 } }
    };
    mGameState.xochitl = new Entity({ 125.0f, 360.0f }, { 44.0f, 48.0f },
        "assets/student.png", ATLAS, { 4, 4 }, atlas, PLAYER);
    mGameState.xochitl->setColliderDimensions({ 30.0f, 44.0f });
    mGameState.xochitl->setAcceleration({ 0.0f, 981.0f });
    mGameState.xochitl->setJumpingPower(540.0f);
    mGameState.xochitl->setSpeed(280);
    mGameState.xochitl->setFrameSpeed(8);
    mEnemyCount = mNumber == 1 ? 3 : (mNumber == 2 ? 6 : 7); 
    for (int i = 0; i < mEnemyCount; i++)
    {
        const float lanes[9] = {650, 1150, 1700, 2050, 2500, 2850, 1075, 2700, 3050};
        float x = lanes[i];
        AIType type = i % 3 == 0 ? WANDERER : (i % 3 == 1 ? FOLLOWER : FLYER);
        const char *image = type == WANDERER ? "assets/cart.png" :
            (type == FOLLOWER ? "assets/alarm.png" : "assets/plane.png");

        mEnemies[i] = new Entity({ x, type == FLYER ? 270.0f : 378.0f },
            { 42.0f, 42.0f }, image, NPC);
        mEnemies[i]->setColliderDimensions({ 34.0f, 38.0f });
        mEnemies[i]->setAIType(type);
        mEnemies[i]->setSpeed(mNumber == 3 ? 110 : 65 + mNumber * 15);
        mEnemies[i]->setPatrol(x - 45.0f, x + 45.0f);
        mEnemies[i]->setFlight(215.0f, 365.0f);
        if (type != FLYER) mEnemies[i]->setAcceleration({0, 981.0f});
    }
    mLockedExit = new Entity({ 3150.0f, 350.0f }, { 60.0f, 100.0f }, "assets/locked.png", BLOCK);
    mExit = new Entity({ 3150.0f, 350.0f }, { 60.0f, 100.0f }, "assets/exit.png", BLOCK);
}

void Level::update(float deltaTime)
{
    if (mGameState.nextSceneID != NO_SCENE) return;
    if (mGrace > 0.0f) mGrace -= deltaTime;
    mGameState.xochitl->update(deltaTime, nullptr, mGameState.map, nullptr, 0);
    bool hurt = false;
    for (int i = 0; i < mEnemyCount; i++)
    {
        Map *floor = mEnemies[i]->getAIType() == FLYER ? nullptr : mGameState.map;
        mEnemies[i]->update(deltaTime, mGameState.xochitl, floor, nullptr, 0);

        if (mEnemies[i]->getPosition().y > 650.0f) mEnemies[i]->deactivate();

        if (mGameState.xochitl->isColliding(mEnemies[i])) hurt = true;
    }
    bool fell = mGameState.xochitl->getPosition().y > 650.0f;
    if (fell || (hurt && mGrace <= 0.0f))
    {
        mSession->lives--;
        PlaySound(mSession->hurtSound);
        if (mSession->lives <= 0) mGameState.nextSceneID = LOSE;
        else
        {
            mGameState.xochitl->respawn(mCheckpoint);
            mGrace = 1.5f;
        }
        return;
    }

    for (int i = 0; i < 3; i++)
    {
        if (mPapers[i]->isActive() && mGameState.xochitl->isColliding(mPapers[i]))
        {
            mPapers[i]->deactivate();
            mSession->papers++;
            mCheckpoint = mPapers[i]->getPosition();
            mCheckpoint.y -= 12.0f;
            PlaySound(mSession->paperSound);
        }
    }

    Vector2 position = mGameState.xochitl->getPosition();
    if (position.x > 3170.0f) { position.x = 3170.0f; mGameState.xochitl->setPosition(position); }
    if (mSession->papers == 3 && mGameState.xochitl->isColliding(mExit))
    {
        PlaySound(mSession->exitSound);
        mGameState.nextSceneID = mNumber == 1 ? LEVEL_B : (mNumber == 2 ? LEVEL_C : WIN);
    }
}

void Level::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));
    for (int i = 0; i < 4; i++) mBackground[i]->render();
    mGameState.map->render();
    if (mSession->papers == 3) mExit->render();
    else mLockedExit->render();
    for (int i = 0; i < 3; i++) mPapers[i]->render();
    for (int i = 0; i < mEnemyCount; i++) mEnemies[i]->render();

    if (mGrace <= 0.0f || static_cast<int>(mGrace * 12) % 2 == 0)
        mGameState.xochitl->render();
}

void Level::shutdown()
{

    if (!mLoaded) return;
    delete mGameState.xochitl; mGameState.xochitl = nullptr;
    delete mGameState.map; mGameState.map = nullptr;
    delete mExit; mExit = nullptr;
    delete mLockedExit; mLockedExit = nullptr;
    for (int i = 0; i < 3; i++) { delete mPapers[i]; mPapers[i] = nullptr; }
    for (int i = 0; i < 4; i++) { delete mBackground[i]; mBackground[i] = nullptr; }
    for (int i = 0; i < mEnemyCount; i++) { delete mEnemies[i]; mEnemies[i] = nullptr; }
    mEnemyCount = 0;
    mLoaded = false;
}

MessageScene::MessageScene(const char *image) : Scene({ 500, 300 }, "#EFE4CC"), mImage(image) {}
MessageScene::~MessageScene() { shutdown(); }
void MessageScene::initialise()
{
    mGameState.nextSceneID = NO_SCENE;
    mIllustration = new Entity({500, 300}, {1000, 600}, "assets/campus_menu.png", BLOCK);
    mPanel = new Entity({ 500, 300 }, { 1000, 600 }, mImage, BLOCK);
}
void MessageScene::update(float) {} 
void MessageScene::render() { ClearBackground(ColorFromHex(mBGColourHexCode)); mIllustration->render(); mPanel->render(); }
void MessageScene::shutdown() { delete mPanel; mPanel = nullptr; delete mIllustration; mIllustration = nullptr; }
