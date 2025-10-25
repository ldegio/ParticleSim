// Build as Windows subsystem without console, keep main() entry:
#ifdef _MSC_VER
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif

#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iostream>
#include "raylib.h"

#define initializeVector2D {0, 0}

// bounds
#define distFromScreenEdge 10
#define xUpperBound GetScreenWidth() - distFromScreenEdge
#define xLowerBound distFromScreenEdge
#define yUpperBound GetScreenHeight() - distFromScreenEdge
#define yLowerBound distFromScreenEdge
constexpr int particleNum = 4000;
constexpr int numDist = (particleNum - 1) * (particleNum / 2);

class Time {
public:
    // Returns delta time in seconds as a float
    static float DeltaTime() {
        using clock = std::chrono::high_resolution_clock;
        static auto last = clock::now();   // persists across calls
        auto now = clock::now();
        std::chrono::duration<float> dt = now - last;
        last = now;
        return dt.count();
    }
};

float maxSpeed = 10000;
static int lastID = 0;
Vector2 bounds = initializeVector2D;
float mouseAttraction = 10000;

class particleData;

struct Distances
{
public:

    float dist = 0;
    particleData* data1 = nullptr;
    particleData* data2 = nullptr;
};

class particleData
{
public:

    int id = 0;
    int type = 1; // 0 = stationary, 1 = free moving
    float Attraction = ((std::rand() & 800) - 400) * 150;
    const float range = (std::rand() % 20) + 10 * 1000;
    const float collisionRange = (rand() % 10) + 10;
    Vector2 Direction = { 0, 0 };
    Distances* dists[particleNum - 1] = {};
    Vector2 AveragedAttractions = initializeVector2D;
    Vector2 position = initializeVector2D;

    particleData() {
        id = lastID;
        lastID++;
    }
};


particleData particles[particleNum];
Distances distances[numDist];

void start()
{
    int arrayNumb = 0;

    for (int i = 0; i < particleNum; i++)
    {
        for (int v = 0; v < particleNum; v++)
        {
            particles[i].dists[v] = &distances[arrayNumb];
            particles[v].dists[i] = &distances[arrayNumb];

            if (v < i) continue;
            if (particles[v].id == particles[i].id) continue;

            distances[arrayNumb].data1 = &particles[i];
            distances[arrayNumb].data2 = &particles[v];

            arrayNumb++;
        }
    }
}

void update(float deltaTime)
{
    for (Distances& calculation : distances)
    {
        calculation.dist = sqrt((calculation.data1->position.x - calculation.data2->position.x) * (calculation.data1->position.x - calculation.data2->position.x) + (calculation.data1->position.y - calculation.data2->position.y) * (calculation.data1->position.y - calculation.data2->position.y));
    }

    for (int particle = 0; particle < particleNum; particle++)
    {
        particles[particle].AveragedAttractions = initializeVector2D;

        int neighbourCount = 0;

        if (particles[particle].type == 0) {
            DrawPixel(particles[particle].position.x, particles[particle].position.y, GREEN);
            continue;
        }

        int innerParticleID = 0;

        for (int particleDetected = 0; particleDetected < particleNum; particleDetected++)
        {
            if (particles[particleDetected].id == particles[particle].id) continue;

            if (particles[particle].dists[innerParticleID]->dist > particles[particle].range) continue;

            if (particles[particle].dists[innerParticleID]->dist == 0) particles[particle].dists[innerParticleID]->dist = 0.0001;

            innerParticleID++;

            float inversDist = 1 / particles[particle].dists[innerParticleID]->dist;

            neighbourCount++;

            if (particles[particle].dists[innerParticleID]->dist > particles[particle].collisionRange)
            {
                particles[particle].Direction.x = (particles[particleDetected].position.x - particles[particle].position.x) * inversDist;
                particles[particle].Direction.y = (particles[particleDetected].position.y - particles[particle].position.y) * inversDist;
            }
            else
            {
                if (particles[particleDetected].Attraction >= 0)
                {
                    particles[particle].Direction.x = (particles[particleDetected].position.x - particles[particle].position.x) * -1 * inversDist;
                    particles[particle].Direction.y = (particles[particleDetected].position.y - particles[particle].position.y) * -1 * inversDist;
                }
                else
                {
                    particles[particle].Direction.x = ((particles[particleDetected].position.x - particles[particle].position.x)) * inversDist;
                    particles[particle].Direction.y = ((particles[particleDetected].position.y - particles[particle].position.y)) * inversDist;
                }
            }

            particles[particle].AveragedAttractions.x += particles[particleDetected].Attraction * particles[particle].Direction.x * abs(((particles[particle].range - particles[particle].dists[innerParticleID]->dist) / particles[particle].range)) - ((particles[particle].position.x - GetScreenWidth() * 0.5) * 0.4);
            particles[particle].AveragedAttractions.y += particles[particleDetected].Attraction * particles[particle].Direction.y * abs(((particles[particle].range - particles[particle].dists[innerParticleID]->dist) / particles[particle].range)) - ((particles[particle].position.y - GetScreenHeight() * 0.5) * 0.4);
        }

        if (IsMouseButtonDown(0))
        {
            float dist = sqrt((GetMousePosition().x - particles[particle].position.x) * (GetMousePosition().x - particles[particle].position.x) + (GetMousePosition().y - particles[particle].position.y) * (GetMousePosition().y - particles[particle].position.y));

            if (dist < particles[particle].range)
            {
                if (dist == 0) dist = 0.0001;

                float inversDist = 1 / dist;
                particles[particle].Direction.x = ((GetMousePosition().x - particles[particle].position.x));
                particles[particle].Direction.y = ((GetMousePosition().y - particles[particle].position.y));
                particles[particle].AveragedAttractions.x = particles[particle].Direction.x * mouseAttraction * abs(((particles[particle].range - dist) / particles[particle].range));
                particles[particle].AveragedAttractions.y = particles[particle].Direction.y * mouseAttraction * abs(((particles[particle].range - dist) / particles[particle].range));
            }
        }

        if (neighbourCount != 0)
        {
            particles[particle].AveragedAttractions.x /= neighbourCount;
            particles[particle].AveragedAttractions.y /= neighbourCount;
        }

        particles[particle].AveragedAttractions.x = std::clamp(particles[particle].AveragedAttractions.x, -maxSpeed, maxSpeed);
        particles[particle].AveragedAttractions.y = std::clamp(particles[particle].AveragedAttractions.y, -maxSpeed, maxSpeed);

        particles[particle].position.x += particles[particle].AveragedAttractions.x * deltaTime;
        particles[particle].position.y += particles[particle].AveragedAttractions.y * deltaTime;

        if (particles[particle].position.x > xUpperBound)
        {
            particles[particle].position.x = xUpperBound;
        }
        else if (particles[particle].position.x < xLowerBound)
        {
            particles[particle].position.x = xLowerBound;
        }

        if (particles[particle].position.y > yUpperBound)
        {
            particles[particle].position.y = yUpperBound;
        }
        else if (particles[particle].position.y < yLowerBound)
        {
            particles[particle].position.y = yLowerBound;
        }

        //DrawText(std::to_string(particle.Direction.x).data(), 40, 40, 35, WHITE);
        DrawPixel(particles[particle].position.x, particles[particle].position.y, WHITE);
    }
}

int main() {
    std::srand(std::time(nullptr)); // seed with current time

#if defined(_DEBUG)
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);             // no fullscreen in debug
    InitWindow(1280, 720, "raylib");
#else
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT | FLAG_FULLSCREEN_MODE);
    InitWindow(0, 0, "raylib fullscreen");
#endif

    for (particleData& particle : particles)
    {
        particle.position.x = (float)(std::rand() % GetScreenWidth());
        particle.position.y = (float)(std::rand() % GetScreenHeight());
    }

    SetTargetFPS(60);

    start();

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_F11)) ToggleFullscreen();
        if (IsKeyPressed(KEY_ESCAPE)) break;

        BeginDrawing();

        ClearBackground(BLACK);
        //DrawText("Hello, fullscreen raylib!", 40, 40, 32, RAYWHITE);
        //DrawCircle(GetScreenWidth() / 2, GetScreenHeight() / 2, 80, RED);
        update(Time::DeltaTime());
        EndDrawing();
    }

    CloseWindow();
}