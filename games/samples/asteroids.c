/*******************************************************************************************
*
*   raylib - sample game: asteroids
*
*   Sample game developed by Ian Eito, Albert Martos and Ramon Santamaria
*
*   This game has been created using raylib v1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <math.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define MAX_SPEED           6
#define METEORS_SPEED       2
#define NUM_SHOOTS          10
#define NUM_BIG_METEORS     4
#define NUM_MEDIUM_METEORS  8
#define NUM_SMALL_METEORS   16
#define SHIP_BASE_SIZE      20.0f

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

typedef struct Player {
    Vector2 position;
    Vector2 speed;
    float acceleration;
    float rotation;
    Vector3 collider;
    Color color;
} Player;

typedef struct Shoot {
    Vector2 position;
    Vector2 speed;
    float radius;
    float rotation;
    int lifeSpawn;
    bool active;
    Color color;
} Shoot;

typedef struct BigMeteor {
    Vector2 position;
    Vector2 speed;
    float radius;
    bool active;
    Color color;
} BigMeteor;

typedef struct MediumMeteor {
    Vector2 position;
    Vector2 speed;
    float radius;
    bool active;
    Color color;
} MediumMeteor;

typedef struct SmallMeteor {
    Vector2 position;
    Vector2 speed;
    float radius;
    bool active;
    Color color;
} SmallMeteor;

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
static int screenWidth = 800;
static int screenHeight = 450;

static int framesCounter;
static bool gameOver;
static bool pause;
static bool victory;

// NOTE: Defined triangle is isosceles with common angles of 70 degrees.
static float shipHeight;

static Player player;
static Shoot shoot[NUM_SHOOTS];
static BigMeteor bigMeteor[NUM_BIG_METEORS];
static MediumMeteor mediumMeteor[NUM_MEDIUM_METEORS];
static SmallMeteor smallMeteor[NUM_SMALL_METEORS];

static int countMediumMeteors;
static int countSmallMeteors;
static int meteorsDestroyed;

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(void);  // Update and Draw (one frame)

static void InitShoot(Shoot shoot);
static void DrawSpaceship(Vector2 position, float rotation, Color color);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "sample game: asteroids");

    InitGame();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateGame();
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        DrawGame();
        //----------------------------------------------------------------------------------
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadGame();         // Unload loaded data (textures, sounds, models...)
    
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definitions (local)
//------------------------------------------------------------------------------------

// Initialize game variables
void InitGame(void)
{
    int posx, posy;
    int velx, vely;
    bool correctRange = false;
    victory = false;
    pause = false;

    shipHeight = (SHIP_BASE_SIZE/2)/tanf(20*DEG2RAD);

    // Initialization player
    player.position = (Vector2){screenWidth/2, screenHeight/2 - shipHeight/2};
    player.speed = (Vector2){0, 0};
    player.acceleration = 0;
    player.rotation = 0;
    player.collider = (Vector3){player.position.x + sin(player.rotation*DEG2RAD)*(shipHeight/2.5f), player.position.y - cos(player.rotation*DEG2RAD)*(shipHeight/2.5f), 12};
    player.color = LIGHTGRAY;

    meteorsDestroyed = 0;

    //InitShoot(&shoot);

    // Initialization shoot
    for (int i = 0; i < NUM_SHOOTS; i++)
    {
        shoot[i].position = (Vector2){0, 0};
        shoot[i].speed = (Vector2){0, 0};
        shoot[i].radius = 2;
        shoot[i].active = false;
        shoot[i].lifeSpawn = 0;
        shoot[i].color = WHITE;
    }

    for (int i = 0; i < NUM_BIG_METEORS; i++)
    {
        posx = GetRandomValue(0, screenWidth);

        while(!correctRange)
        {
            if (posx > screenWidth/2 - 150 && posx < screenWidth/2 + 150) posx = GetRandomValue(0, screenWidth);
            else correctRange = true;
        }

        correctRange = false;

        posy = GetRandomValue(0, screenHeight);

        while(!correctRange)
        {
            if (posy > screenHeight/2 - 150 && posy < screenHeight/2 + 150)  posy = GetRandomValue(0, screenHeight);
            else correctRange = true;
        }

        bigMeteor[i].position = (Vector2){posx, posy};

        correctRange = false;
        velx = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);
        vely = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);

        while(!correctRange)
        {
            if (velx == 0 && vely == 0)
            {
                velx = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);
                vely = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);
            }
            else correctRange = true;
        }

        bigMeteor[i].speed = (Vector2){velx, vely};
        bigMeteor[i].radius = 40;
        bigMeteor[i].active = true;
        bigMeteor[i].color = BLUE;
    }

    for (int i = 0; i < NUM_MEDIUM_METEORS; i++)
    {
        mediumMeteor[i].position = (Vector2){-100, -100};
        mediumMeteor[i].speed = (Vector2){0,0};
        mediumMeteor[i].radius = 20;
        mediumMeteor[i].active = false;
        mediumMeteor[i].color = BLUE;
    }

    for (int i = 0; i < NUM_SMALL_METEORS; i++)
    {
        smallMeteor[i].position = (Vector2){-100, -100};
        smallMeteor[i].speed = (Vector2){0,0};
        smallMeteor[i].radius = 10;
        smallMeteor[i].active = false;
        smallMeteor[i].color = BLUE;
    }

    countMediumMeteors = 0;
    countSmallMeteors = 0;
}

// Update game (one frame)
void UpdateGame(void)
{
    if (!gameOver)
    {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause)
        {
            // Player logic

            // Rotation
            if (IsKeyDown(KEY_LEFT)) player.rotation -= 5;
            if (IsKeyDown(KEY_RIGHT)) player.rotation += 5;

            // Speed
            player.speed.x = sin(player.rotation*DEG2RAD)*MAX_SPEED;
            player.speed.y = cos(player.rotation*DEG2RAD)*MAX_SPEED;

            // Controller
            if (IsKeyDown(KEY_UP))
            {
                if (player.acceleration < 1) player.acceleration += 0.04f;
            }
            else
            {
                if (player.acceleration > 0) player.acceleration -= 0.02f;
                else if (player.acceleration < 0) player.acceleration = 0;
            }
            if (IsKeyDown(KEY_DOWN))
            {
                if (player.acceleration > 0) player.acceleration -= 0.04f;
                else if (player.acceleration < 0) player.acceleration = 0;
            }

            // Movement
            player.position.x += (player.speed.x*player.acceleration);
            player.position.y -= (player.speed.y*player.acceleration);

            // Wall behaviour for player
            if (player.position.x > screenWidth + shipHeight) player.position.x = -(shipHeight);
            else if (player.position.x < -(shipHeight)) player.position.x = screenWidth + shipHeight;
            if (player.position.y > (screenHeight + shipHeight)) player.position.y = -(shipHeight);
            else if (player.position.y < -(shipHeight)) player.position.y = screenHeight + shipHeight;

             // Activation of shoot
            if (IsKeyPressed(KEY_SPACE))
            {
                for (int i = 0; i < NUM_SHOOTS; i++)
                {
                    if (!shoot[i].active)
                    {
                        shoot[i].position = (Vector2){ player.position.x + sin(player.rotation*DEG2RAD)*(shipHeight), player.position.y - cos(player.rotation*DEG2RAD)*(shipHeight) };
                        shoot[i].active = true;
                        shoot[i].speed.x = 1.5*sin(player.rotation*DEG2RAD)*MAX_SPEED;
                        shoot[i].speed.y = 1.5*cos(player.rotation*DEG2RAD)*MAX_SPEED;
                        shoot[i].rotation = player.rotation;
                        break;
                    }
                }
            }

            // Shoot life timer
            for (int i = 0; i < NUM_SHOOTS; i++)
            {
                if (shoot[i].active) shoot[i].lifeSpawn++;
            }

            // Shot logic
            for (int i = 0; i < NUM_SHOOTS; i++)
            {
                if (shoot[i].active)
                {
                    // Movement
                    shoot[i].position.x += shoot[i].speed.x;
                    shoot[i].position.y -= shoot[i].speed.y;

                    // Wall behaviour for shoot
                    if  (shoot[i].position.x > screenWidth + shoot[i].radius)
                    {
                        shoot[i].active = false;
                        shoot[i].lifeSpawn = 0;
                    }
                    else if (shoot[i].position.x < 0 - shoot[i].radius)
                    {
                        shoot[i].active = false;
                        shoot[i].lifeSpawn = 0;
                    }
                    if (shoot[i].position.y > screenHeight + shoot[i].radius)
                    {
                        shoot[i].active = false;
                        shoot[i].lifeSpawn = 0;
                    }
                    else if (shoot[i].position.y < 0 - shoot[i].radius)
                    {
                        shoot[i].active = false;
                        shoot[i].lifeSpawn = 0;
                    }

                    // Life of shoot
                    if (shoot[i].lifeSpawn >= 60)
                    {
                        shoot[i].position = (Vector2){0, 0};
                        shoot[i].speed = (Vector2){0, 0};
                        shoot[i].lifeSpawn = 0;
                        shoot[i].active = false;
                    }
                }
            }

            // Collision Player to meteors
            player.collider = (Vector3){player.position.x + sin(player.rotation*DEG2RAD)*(shipHeight/2.5f), player.position.y - cos(player.rotation*DEG2RAD)*(shipHeight/2.5f), 12};

            for (int a = 0; a < NUM_BIG_METEORS; a++)
            {
                if (CheckCollisionCircles((Vector2){player.collider.x, player.collider.y}, player.collider.z, bigMeteor[a].position, bigMeteor[a].radius) && bigMeteor[a].active) gameOver = true;
            }

            for (int a = 0; a < NUM_MEDIUM_METEORS; a++)
            {
                if (CheckCollisionCircles((Vector2){player.collider.x, player.collider.y}, player.collider.z, mediumMeteor[a].position, mediumMeteor[a].radius) && mediumMeteor[a].active) gameOver = true;
            }

            for (int a = 0; a < NUM_SMALL_METEORS; a++)
            {
                if (CheckCollisionCircles((Vector2){player.collider.x, player.collider.y}, player.collider.z, smallMeteor[a].position, smallMeteor[a].radius) && smallMeteor[a].active) gameOver = true;
            }

            // Meteor logic
            for (int i = 0; i < NUM_BIG_METEORS; i++)
            {
                if (bigMeteor[i].active)
                {
                    // movement
                    bigMeteor[i].position.x += bigMeteor[i].speed.x;
                    bigMeteor[i].position.y += bigMeteor[i].speed.y;

                    // wall behaviour
                    if  (bigMeteor[i].position.x > screenWidth + bigMeteor[i].radius) bigMeteor[i].position.x = -(bigMeteor[i].radius);
                    else if (bigMeteor[i].position.x < 0 - bigMeteor[i].radius) bigMeteor[i].position.x = screenWidth + bigMeteor[i].radius;
                    if (bigMeteor[i].position.y > screenHeight + bigMeteor[i].radius) bigMeteor[i].position.y = -(bigMeteor[i].radius);
                    else if (bigMeteor[i].position.y < 0 - bigMeteor[i].radius) bigMeteor[i].position.y = screenHeight + bigMeteor[i].radius;
                }
            }

            for (int i = 0; i < NUM_MEDIUM_METEORS; i++)
            {
                if (mediumMeteor[i].active)
                {
                    // movement
                    mediumMeteor[i].position.x += mediumMeteor[i].speed.x;
                    mediumMeteor[i].position.y += mediumMeteor[i].speed.y;

                    // wall behaviour
                    if  (mediumMeteor[i].position.x > screenWidth + mediumMeteor[i].radius) mediumMeteor[i].position.x = -(mediumMeteor[i].radius);
                    else if (mediumMeteor[i].position.x < 0 - mediumMeteor[i].radius) mediumMeteor[i].position.x = screenWidth + mediumMeteor[i].radius;
                    if (mediumMeteor[i].position.y > screenHeight + mediumMeteor[i].radius) mediumMeteor[i].position.y = -(mediumMeteor[i].radius);
                    else if (mediumMeteor[i].position.y < 0 - mediumMeteor[i].radius) mediumMeteor[i].position.y = screenHeight + mediumMeteor[i].radius;
                }
            }

            for (int i = 0; i < NUM_SMALL_METEORS; i++)
            {
                if (smallMeteor[i].active)
                {
                    // movement
                    smallMeteor[i].position.x += smallMeteor[i].speed.x;
                    smallMeteor[i].position.y += smallMeteor[i].speed.y;

                    // wall behaviour
                    if  (smallMeteor[i].position.x > screenWidth + smallMeteor[i].radius) smallMeteor[i].position.x = -(smallMeteor[i].radius);
                    else if (smallMeteor[i].position.x < 0 - smallMeteor[i].radius) smallMeteor[i].position.x = screenWidth + smallMeteor[i].radius;
                    if (smallMeteor[i].position.y > screenHeight + smallMeteor[i].radius) smallMeteor[i].position.y = -(smallMeteor[i].radius);
                    else if (smallMeteor[i].position.y < 0 - smallMeteor[i].radius) smallMeteor[i].position.y = screenHeight + smallMeteor[i].radius;
                }
            }

            // Collision behaviour
            for (int i = 0; i < NUM_SHOOTS; i++)
            {
                if ((shoot[i].active))
                {
                    for (int a = 0; a < NUM_BIG_METEORS; a++)
                    {
                        if (bigMeteor[a].active && CheckCollisionCircles(shoot[i].position, shoot[i].radius, bigMeteor[a].position, bigMeteor[a].radius))
                        {
                            shoot[i].active = false;
                            shoot[i].lifeSpawn = 0;
                            bigMeteor[a].active = false;
                            meteorsDestroyed++;
                            for (int j = 0; j < 2; j ++)
                            {
                                if (countMediumMeteors%2 == 0)
                                {
                                    mediumMeteor[countMediumMeteors].position = (Vector2){bigMeteor[a].position.x, bigMeteor[a].position.y};
                                    mediumMeteor[countMediumMeteors].speed = (Vector2){cos(shoot[i].rotation*DEG2RAD)*METEORS_SPEED*-1, sin(shoot[i].rotation*DEG2RAD)*METEORS_SPEED*-1};
                                }
                                else
                                {
                                    mediumMeteor[countMediumMeteors].position = (Vector2){bigMeteor[a].position.x, bigMeteor[a].position.y};
                                    mediumMeteor[countMediumMeteors].speed = (Vector2){cos(shoot[i].rotation*DEG2RAD)*METEORS_SPEED, sin(shoot[i].rotation*DEG2RAD)*METEORS_SPEED};
                                }

                                mediumMeteor[countMediumMeteors].active = true;
                                countMediumMeteors ++;
                            }
                            //bigMeteor[a].position = (Vector2){-100, -100};
                            bigMeteor[a].color = RED;
                            a = NUM_BIG_METEORS;
                        }
                    }
                }
                if ((shoot[i].active))
                {
                    for (int b = 0; b < NUM_MEDIUM_METEORS; b++)
                    {
                        if (mediumMeteor[b].active && CheckCollisionCircles(shoot[i].position, shoot[i].radius, mediumMeteor[b].position, mediumMeteor[b].radius))
                        {
                            shoot[i].active = false;
                            shoot[i].lifeSpawn = 0;
                            mediumMeteor[b].active = false;
                            meteorsDestroyed++;
                            for (int j = 0; j < 2; j ++)
                            {
                                 if (countSmallMeteors%2 == 0)
                                {
                                    smallMeteor[countSmallMeteors].position = (Vector2){mediumMeteor[b].position.x, mediumMeteor[b].position.y};
                                    smallMeteor[countSmallMeteors].speed = (Vector2){cos(shoot[i].rotation*DEG2RAD)*METEORS_SPEED*-1, sin(shoot[i].rotation*DEG2RAD)*METEORS_SPEED*-1};
                                }
                                else
                                {
                                    smallMeteor[countSmallMeteors].position = (Vector2){mediumMeteor[b].position.x, mediumMeteor[b].position.y};
                                    smallMeteor[countSmallMeteors].speed = (Vector2){cos(shoot[i].rotation*DEG2RAD)*METEORS_SPEED, sin(shoot[i].rotation*DEG2RAD)*METEORS_SPEED};
                                }

                                smallMeteor[countSmallMeteors].active = true;
                                countSmallMeteors ++;
                            }
                            //mediumMeteor[b].position = (Vector2){-100, -100};
                            mediumMeteor[b].color = GREEN;
                            b = NUM_MEDIUM_METEORS;
                        }
                    }
                }
                if ((shoot[i].active))
                {
                    for (int c = 0; c < NUM_SMALL_METEORS; c++)
                    {
                        if (smallMeteor[c].active && CheckCollisionCircles(shoot[i].position, shoot[i].radius, smallMeteor[c].position, smallMeteor[c].radius))
                        {
                            shoot[i].active = false;
                            shoot[i].lifeSpawn = 0;
                            smallMeteor[c].active = false;
                            meteorsDestroyed++;
                            smallMeteor[c].color = YELLOW;
                           // smallMeteor[c].position = (Vector2){-100, -100};
                            c = NUM_SMALL_METEORS;
                        }
                    }
                }
            }
        }

        if (meteorsDestroyed == NUM_BIG_METEORS + NUM_MEDIUM_METEORS + NUM_SMALL_METEORS) victory = true;
    }
    else
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            InitGame();
            gameOver = false;
        }
    }
}

// Draw game (one frame)
void DrawGame(void)
{
    BeginDrawing();

        ClearBackground(DARKGRAY);

        if (!gameOver)
        {
            // Draw spaceship
            Vector2 v1 = { player.position.x + sinf(player.rotation*DEG2RAD)*(shipHeight), player.position.y - cosf(player.rotation*DEG2RAD)*(shipHeight) };
            Vector2 v2 = { player.position.x - cosf(player.rotation*DEG2RAD)*(SHIP_BASE_SIZE/2), player.position.y - sinf(player.rotation*DEG2RAD)*(SHIP_BASE_SIZE/2) };
            Vector2 v3 = { player.position.x + cosf(player.rotation*DEG2RAD)*(SHIP_BASE_SIZE/2), player.position.y + sinf(player.rotation*DEG2RAD)*(SHIP_BASE_SIZE/2) };
            DrawTriangleLines(v1, v2, v3, player.color);
            
            // Draw meteors
            for (int i = 0; i < NUM_BIG_METEORS; i++)
            {
                if (bigMeteor[i].active) DrawCircleV(bigMeteor[i].position, bigMeteor[i].radius, bigMeteor[i].color);
                else DrawCircleV(bigMeteor[i].position, bigMeteor[i].radius, Fade(bigMeteor[i].color, 0.25f));
            }

            for (int i = 0; i < NUM_MEDIUM_METEORS; i++)
            {
                if (mediumMeteor[i].active) DrawCircleV(mediumMeteor[i].position, mediumMeteor[i].radius, mediumMeteor[i].color);
                else DrawCircleV(mediumMeteor[i].position, mediumMeteor[i].radius, Fade(mediumMeteor[i].color, 0.25f));
            }

            for (int i = 0; i < NUM_SMALL_METEORS; i++)
            {
                if (smallMeteor[i].active) DrawCircleV(smallMeteor[i].position, smallMeteor[i].radius, smallMeteor[i].color);
                else DrawCircleV(smallMeteor[i].position, smallMeteor[i].radius, Fade(smallMeteor[i].color, 0.25f));
            }

            // Draw shoot
            for (int i = 0; i < NUM_SHOOTS; i++)
            {
                if (shoot[i].active) DrawCircleV(shoot[i].position, shoot[i].radius, shoot[i].color);
            }

            if (victory) DrawText("VICTORY", screenWidth/2 - MeasureText("VICTORY", 20)/2, screenHeight/2, 20, LIGHTGRAY);

            if (pause) DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", 40)/2, screenHeight/2 - 40, 40, GRAY);
        }
        else DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, GRAY);
        
    EndDrawing();
}

// Unload game variables
void UnloadGame(void)
{
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
}

// Update and Draw (one frame)
void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}