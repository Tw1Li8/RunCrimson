#pragma once

#include <vector>
#include <random>
#include "Core/EngineTypes.h"
#include "Resources/ResourceManager.h"
#include "Scene/Scene.h"

namespace Engine
{
    class GameObject;
    class Material;

    enum class PlayerState
    {
        Run,
        Jump,
        Slide
    };

    enum class GameState
    {
        Play,
        GameOver
    };

    struct AABB
    {
        Vec2 center = { 0.0f, 0.0f };
        float width = 0.1f;
        float height = 0.1f;
    };

    struct Player
    {
        GameObject* visual = nullptr;
        std::vector<GameObject*> stripeVisuals;
        Vec2 position = { -0.55f, -0.55f };
        Vec2 velocity = { 0.0f, 0.0f };
        PlayerState state = PlayerState::Run;
        AABB collider;
        bool isGrounded = true;
        float normalHeight = 0.18f;
        float slideHeight = 0.09f;
        float jumpPower = 2.0f;
        float gravity = -4.2f;
        float groundY = -0.55f;
        Material* runMaterial = nullptr;
        Material* slideMaterial = nullptr;

        // 스프라이트 시트 애니메이션
        // 시트 구성: 가로 8프레임, 세로 3행 (Run=0, Jump=1, Slide=2)
        static constexpr int   FrameCount = 8;
        static constexpr float FrameWidth = 228.0f / 1824.0f; // 프레임 1장 UV 너비
        static constexpr float FrameHeight = 277.0f / 831.0f;  // 프레임 1장 UV 높이
        static constexpr float RowRun = 0.0f * (277.0f / 831.0f);
        static constexpr float RowJump = 1.0f * (277.0f / 831.0f);
        static constexpr float RowSlide = 2.0f * (277.0f / 831.0f);
        static constexpr float Fps = 8.0f;

        int   currentFrame = 0;
        float frameTimer = 0.0f;
        Material* spriteMaterial = nullptr; // 스프라이트 시트 머티리얼 (1장)
    };

    struct Obstacle
    {
        enum Type
        {
            POLICE,
            DOG,
            WALL,
            LOW_BAR
        };

        GameObject* visual = nullptr;
        Vec2 position = { 0.0f, 0.0f };
        float width = 0.14f;
        float height = 0.18f;
        Type type = POLICE;
        bool alreadyHit = false;
    };

    struct SpeedData
    {
        float baseSpeed = 0.85f;
        int adaptationScore = 500;
        float scorePerDifficulty = 1500.0f;
        float maxDifficulty = 2.2f;
    };

    struct SpawnData
    {
        int targetObstacleCount = 5;
        float firstSpawnX = 0.85f;
        float offscreenX = -1.35f;
        float baseGap = 1.12f;
        float scoreGapReduction = 0.00012f;
        float safetyPadding = 0.16f;
        float absoluteMinGap = 0.58f;
        float absoluteMaxGap = 1.45f;
        float minExtraGap = 0.03f;
        float maxExtraGap = 0.22f;
        int pairStartScore = 1500;
        float pairChance = 0.25f;
        float pairGapScale = 0.8f;
    };

    struct HitboxData
    {
        float playerWidth = 0.12f;
        float playerStandingHeight = 0.18f;
        float playerSlideHeight = 0.09f;
        float forgivingScale = 0.85f;
        float lowBarStandingOverlap = 0.025f;
    };

    class RunnerGameScene : public Scene
    {
    public:
        ~RunnerGameScene() override { resources.Shutdown(); }
        void Initialize(GraphicsContext& graphics) override;
        void Update(float dt) override;
        void Render(GraphicsContext& graphics) override;

    private:
        void UpdatePlayer(float dt);
        void UpdateObstacles(float dt);
        void UpdateBackground();
        void UpdateGameOverBars(float dt);
        void ResetGame();
        void SpawnObstacle(int visualIndex, float x, Obstacle::Type type, float width, float height);
        void SpawnObstacleByType(float x, Obstacle::Type type);
        void SpawnNextObstacle();
        int FindFreeObstacleVisual() const;
        void UpdatePlayerVisual();
        bool CheckOverlap(const AABB& playerBox, const Obstacle& obstacle) const;
        AABB GetForgivingPlayerBox() const;
        AABB GetObstacleBox(const Obstacle& obstacle) const;
        float GetDifficulty() const;
        float GetObstacleWidth(Obstacle::Type type) const;
        float GetObstacleHeight(Obstacle::Type type) const;
        float GetObstacleVisualWidth(Obstacle::Type type) const;
        float GetObstacleVisualHeight(Obstacle::Type type) const;
        Vec2 GetObstacleVisualPosition(const Obstacle& obstacle) const;
        bool RandomChance(float chance);
        float GetStandingPlayerTopY() const;
        float GetSlidingPlayerTopY() const;
        float GetGroundSurfaceY() const;
        float GetGroundObstacleY(float height) const;
        float GetLowBarCenterY(float lowBarHeight) const;
        Material* GetObstacleMaterial(Obstacle::Type type) const;

        ResourceManager resources;
        Player player;
        SpeedData speedData;
        SpawnData spawnData;
        HitboxData hitboxData;
        std::vector<Obstacle> obstacles;
        std::vector<GameObject*> obstacleVisuals;
        std::vector<GameObject*> jailBars;
        GameObject* gameOverOverlay = nullptr;
        GameObject* jailCage = nullptr;
        GameObject* background = nullptr;
        Material* policeMaterial = nullptr;
        Material* dogMaterial = nullptr;
        Material* wallMaterial = nullptr;
        Material* lowBarMaterial = nullptr;
        Material* backgroundMaterials[2] = {};
        int score = 0;
        float scoreAccumulator = 0.0f;
        int currentBackgroundStage = -1;
        GameState gameState = GameState::Play;
        float scrollSpeed = 0.85f;
        float jailDropOffsetY = 1.7f;
        float jailDropSpeed = 1.45f;
        std::mt19937 rng;
    };
}
