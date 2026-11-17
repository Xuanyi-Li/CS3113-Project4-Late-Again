/**
 * Author: Xuanyi Li
 * Assignment: Rise of the AI
 * Date due: 11/16/2026
 * I pledge that I have completed this assignment without
 * collaborating with anyone else, in conformance with the
 * NYU School of Engineering Policies and Procedures on
 * Academic Misconduct.
 */

#include "CS3113/Game.h"

constexpr int SCREEN_WIDTH = 1000, SCREEN_HEIGHT = 600, FPS = 120;
constexpr Vector2 ORIGIN = { 500.0f, 300.0f };
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;
AppStatus gAppStatus = RUNNING;
float gPreviousTicks = 0.0f, gTimeAccumulator = 0.0f;
Camera2D gCamera = {};
Session gSession;
Music gMusic = {};
Scene *gCurrentScene = nullptr;
SceneID gSceneID = MENU;
std::map<SceneID, Scene *> gLevels;
Entity *gLives[6] = {};
Entity *gLabels[3] = {};
Entity *gPaperHUD[4] = {};

bool playing() { return gSceneID == LEVEL_A || gSceneID == LEVEL_B || gSceneID == LEVEL_C; }

void switchToScene(SceneID id)
{
    if (gCurrentScene) gCurrentScene->shutdown();
    gSceneID = id;
    if (id == MENU) gSession.lives = 5;
    gCurrentScene = gLevels[id];
    gCurrentScene->initialise();
    gCamera.target = ORIGIN;
    gTimeAccumulator = 0.0f;
    gPreviousTicks = static_cast<float>(GetTime());
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "LATE AGAIN! - Rise of the AI");
    InitAudioDevice();
    gMusic = LoadMusicStream("assets/morning.wav");
    SetMusicVolume(gMusic, 0.3f);
    PlayMusicStream(gMusic);
    gSession.jumpSound = LoadSound("assets/jump.wav");
    gSession.hurtSound = LoadSound("assets/hurt.wav");
    gSession.exitSound = LoadSound("assets/exit.wav");
    gSession.paperSound = LoadSound("assets/paper.wav");
    gLevels[MENU] = new MessageScene("assets/menu.png");
    gLevels[WIN] = new MessageScene("assets/win.png");
    gLevels[LOSE] = new MessageScene("assets/lose.png");
    gLevels[LEVEL_A] = new Level(1, &gSession);
    gLevels[LEVEL_B] = new Level(2, &gSession);
    gLevels[LEVEL_C] = new Level(3, &gSession);
    const char *livesImages[] = { "assets/lives0.png", "assets/lives1.png", "assets/lives2.png",
        "assets/lives3.png", "assets/lives4.png", "assets/lives5.png" };
    const char *labels[] = { "assets/level1.png", "assets/level2.png", "assets/level3.png" };
    for (int i = 0; i < 6; i++) gLives[i] = new Entity({ 835, 40 }, { 270, 40 }, livesImages[i], BLOCK);
    for (int i = 0; i < 3; i++) gLabels[i] = new Entity({ 330, 40 }, { 600, 40 }, labels[i], BLOCK);
    const char *paperImages[] = {"assets/papers0.png", "assets/papers1.png", "assets/papers2.png", "assets/papers3.png"};
    for (int i = 0; i < 4; i++) gPaperHUD[i] = new Entity({500, 560}, {940, 50}, paperImages[i], BLOCK);
    gCamera.offset = ORIGIN;
    gCamera.zoom = 1.0f;
    switchToScene(MENU);
    SetTargetFPS(FPS);
}

void processInput()
{
    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) { gAppStatus = TERMINATED; return; }
    if (!playing())
    {
        if (IsKeyPressed(KEY_ENTER)) switchToScene(gSceneID == MENU ? LEVEL_A : MENU);
        return;
    }
    if (IsKeyPressed(KEY_ONE)) switchToScene(LEVEL_A);
    else if (IsKeyPressed(KEY_TWO)) switchToScene(LEVEL_B);
    else if (IsKeyPressed(KEY_THREE)) switchToScene(LEVEL_C);
    Entity *player = gCurrentScene->getState().xochitl;
    player->resetMovement(); 
    if (IsKeyDown(KEY_A)) player->moveLeft();
    else if (IsKeyDown(KEY_D)) player->moveRight();
    if ((IsKeyPressed(KEY_W) || IsKeyPressed(KEY_SPACE)) && player->isCollidingBottom())
    {
        player->jump();
        PlaySound(gSession.jumpSound);
    }
}

void update()
{
    UpdateMusicStream(gMusic);
    float ticks = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    if (deltaTime > 0.25f) deltaTime = 0.25f;
    deltaTime += gTimeAccumulator; 
    while (deltaTime >= FIXED_TIMESTEP)
    {
        gCurrentScene->update(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;
        if (gCurrentScene->getState().nextSceneID != NO_SCENE)
        {
            switchToScene(gCurrentScene->getState().nextSceneID);
            return;
        }
        if (playing())
        {
            Vector2 target = { gCurrentScene->getState().xochitl->getPosition().x, ORIGIN.y };
            if (target.x < 500.0f) target.x = 500.0f;
            if (target.x > 2700.0f) target.x = 2700.0f;
            panCamera(&gCamera, &target);
        }
    }
    gTimeAccumulator = deltaTime; 
}

void render()
{
    BeginDrawing();
    BeginMode2D(gCamera);
    gCurrentScene->render();
    EndMode2D();
    if (playing())
    {
        gLives[gSession.lives]->render();
        gLabels[static_cast<int>(gSceneID)]->render();
        gPaperHUD[gSession.papers]->render();
    }
    EndDrawing();
}
void shutdown()
{
    for (std::pair<const SceneID, Scene *> &entry : gLevels) delete entry.second;
    gLevels.clear();
    for (int i = 0; i < 6; i++) delete gLives[i];
    for (int i = 0; i < 3; i++) delete gLabels[i];
    for (int i = 0; i < 4; i++) delete gPaperHUD[i];
    UnloadMusicStream(gMusic);
    UnloadSound(gSession.jumpSound);
    UnloadSound(gSession.hurtSound);
    UnloadSound(gSession.exitSound);
    UnloadSound(gSession.paperSound);
    CloseAudioDevice();
    CloseWindow();
}
int main()
{
    initialise();
    while (gAppStatus == RUNNING) { processInput(); update(); render(); }
    shutdown();
    return 0;
}
