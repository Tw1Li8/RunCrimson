#include "Game/RunnerGameScene.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <windows.h>
#include "Graphics/GraphicsContext.h"
#include "Graphics/MeshRenderer.h"
#include "Input/Input.h"
#include "Object/GameObject.h"

namespace Engine
{
    namespace
    {
        constexpr int MaxObstacleVisuals = 8;
        constexpr int TextureSize = 16;

        unsigned int Rgba(unsigned int r, unsigned int g, unsigned int b)
        {
            return 0xff000000u | (b << 16) | (g << 8) | r;
        }

        // ── 죄수 : 달리기 / 점프 (서있는 자세) ──────────────────────
        // 픽셀맵 직접 정의 방식 — 행 순서(y=0 상단 → y=15 하단)
        std::array<unsigned int, TextureSize* TextureSize> MakePrisonerRunTexture()
        {
            // 색상 단축 상수
            const unsigned int BG = Rgba(20, 24, 32); // 배경
            const unsigned int HR = Rgba(40, 28, 14); // 머리카락
            const unsigned int SK = Rgba(220, 170, 120); // 피부
            const unsigned int S2 = Rgba(200, 145, 95); // 피부 윤곽
            const unsigned int EY = Rgba(30, 30, 30); // 눈
            const unsigned int NS = Rgba(185, 130, 85); // 코
            const unsigned int MT = Rgba(155, 60, 50); // 입
            const unsigned int W = Rgba(238, 238, 220); // 수의 흰줄
            const unsigned int BK = Rgba(18, 22, 30); // 수의 검은줄
            const unsigned int NP = Rgba(190, 190, 170); // 번호판
            const unsigned int SH = Rgba(28, 20, 14); // 신발

            // 16x16 픽셀맵 (각 행 = y, 각 열 = x 0→15)
            const unsigned int map[16][16] =
            {
                //  0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15
                  { BG,  BG,  BG,  BG,  HR,  HR,  HR,  HR,  HR,  HR,  HR,  HR,  BG,  BG,  BG,  BG }, // y0  머리카락
                  { BG,  BG,  BG,  S2,  SK,  SK,  SK,  SK,  SK,  SK,  SK,  SK,  S2,  BG,  BG,  BG }, // y1  얼굴 위
                  { BG,  BG,  BG,  SK,  SK,  EY,  SK,  NS,  SK,  EY,  SK,  SK,  SK,  BG,  BG,  BG }, // y2  눈·코
                  { BG,  BG,  BG,  SK,  SK,  MT,  MT,  MT,  MT,  MT,  SK,  SK,  SK,  BG,  BG,  BG }, // y3  입
                  { BG,  BG,  BG,  S2,  SK,  SK,  SK,  SK,  SK,  SK,  SK,  SK,  S2,  BG,  BG,  BG }, // y4  목
                  { BG,  BG,  W,   BK,  W,   BK,  W,   BK,  W,   BK,  W,   BK,  W,   BK,  BG,  BG }, // y5  어깨/몸통
                  { BG,  BG,  W,   BK,  W,   BK,  NP,  NP,  NP,  NP,  W,   BK,  W,   BK,  BG,  BG }, // y6  번호판
                  { BG,  BG,  W,   BK,  W,   BK,  NP,  NP,  NP,  NP,  W,   BK,  W,   BK,  BG,  BG }, // y7  번호판
                  { BG,  BG,  W,   BK,  W,   BK,  NP,  NP,  NP,  NP,  W,   BK,  W,   BK,  BG,  BG }, // y8  번호판
                  { BG,  BG,  W,   BK,  W,   BK,  W,   BK,  W,   BK,  W,   BK,  W,   BK,  BG,  BG }, // y9  몸통
                  { BG,  BG,  W,   BK,  W,   BK,  W,   BK,  W,   BK,  W,   BK,  W,   BK,  BG,  BG }, // y10 몸통
                  { BG,  BG,  BG,  W,   BK,  W,   BK,  W,   BK,  W,   BK,  W,   BK,  BG,  BG,  BG }, // y11 허리
                  { BG,  BG,  BG,  W,   BK,  W,   BG,  BG,  BG,  BG,  W,   BK,  W,   BG,  BG,  BG }, // y12 다리
                  { BG,  BG,  BG,  W,   BK,  W,   BG,  BG,  BG,  BG,  W,   BK,  W,   BG,  BG,  BG }, // y13 다리
                  { BG,  BG,  BG,  W,   BK,  W,   BG,  BG,  BG,  BG,  W,   BK,  W,   BG,  BG,  BG }, // y14 다리
                  { BG,  BG,  SH,  SH,  SH,  SH,  BG,  BG,  BG,  BG,  SH,  SH,  SH,  SH,  BG,  BG }, // y15 신발
            };

            std::array<unsigned int, TextureSize* TextureSize> pixels = {};
            for (int y = 0; y < TextureSize; ++y)
                for (int x = 0; x < TextureSize; ++x)
                    pixels[y * TextureSize + x] = map[y][x];
            return pixels;
        }

        // ── 죄수 : 슬라이딩 (엎드린 자세) ──────────────────────────
        // 가로 구도: 왼쪽=신발, 오른쪽=머리 / 세로 중앙(y4~y10)에 몸 집중
        std::array<unsigned int, TextureSize* TextureSize> MakePrisonerSlideTexture()
        {
            const unsigned int BG = Rgba(20, 24, 32);
            const unsigned int HR = Rgba(40, 28, 14);
            const unsigned int SK = Rgba(220, 170, 120);
            const unsigned int S2 = Rgba(200, 145, 95);
            const unsigned int EY = Rgba(30, 30, 30);
            const unsigned int NS = Rgba(185, 130, 85);
            const unsigned int MT = Rgba(155, 60, 50);
            const unsigned int W = Rgba(238, 238, 220);
            const unsigned int BK = Rgba(18, 22, 30);
            const unsigned int NP = Rgba(190, 190, 170);
            const unsigned int SH = Rgba(28, 20, 14);

            // 가로 눕힌 구도: 왼쪽(x0~1)=신발, 중간(x2~10)=몸통줄무늬+번호판, 오른쪽(x11~15)=얼굴
            const unsigned int map[16][16] =
            {
                //  0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15
                  { BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG }, // y0
                  { BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG }, // y1
                  { BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  HR,  HR,  HR,  BG }, // y2  머리카락 위
                  { BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  S2,  SK,  HR,  HR,  BG }, // y3  머리카락
                  { SH,  SH,  W,   BK,  W,   BK,  W,   BK,  W,   BK,  W,   SK,  SK,  SK,  SK,  BG }, // y4  몸통/얼굴 시작
                  { SH,  SH,  W,   BK,  W,   BK,  NP,  NP,  NP,  W,   BK,  SK,  EY,  SK,  SK,  BG }, // y5  눈
                  { SH,  W,   BK,  W,   BK,  W,   NP,  NP,  NP,  W,   BK,  SK,  NS,  SK,  SK,  BG }, // y6  코
                  { SH,  W,   BK,  W,   BK,  W,   NP,  NP,  NP,  W,   BK,  SK,  MT,  MT,  SK,  BG }, // y7  입
                  { SH,  W,   BK,  W,   BK,  W,   BK,  W,   BK,  W,   BK,  S2,  SK,  SK,  SK,  BG }, // y8  얼굴 하단
                  { SH,  SH,  W,   BK,  W,   BK,  W,   BK,  W,   BK,  W,   SK,  SK,  SK,  SK,  BG }, // y9  몸통/얼굴 끝
                  { SH,  SH,  W,   BK,  W,   BK,  W,   BK,  W,   BK,  W,   BK,  W,   BG,  BG,  BG }, // y10 몸통 끝
                  { BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG }, // y11
                  { BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG }, // y12
                  { BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG }, // y13
                  { BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG }, // y14
                  { BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG,  BG }, // y15
            };

            std::array<unsigned int, TextureSize* TextureSize> pixels = {};
            for (int y = 0; y < TextureSize; ++y)
                for (int x = 0; x < TextureSize; ++x)
                    pixels[y * TextureSize + x] = map[y][x];
            return pixels;
        }

        // ── 경찰관 ───────────────────────────────────────────────────
        std::array<unsigned int, TextureSize* TextureSize> MakePoliceTexture()
        {
            std::array<unsigned int, TextureSize* TextureSize> pixels = {};
            for (int y = 0; y < TextureSize; ++y)
            {
                for (int x = 0; x < TextureSize; ++x)
                {
                    unsigned int color = Rgba(20, 24, 32);

                    const bool hatTop = y <= 2 && x >= 3 && x <= 12;
                    const bool hatBrim = y == 3 && x >= 2 && x <= 13;
                    const bool face = y >= 4 && y <= 6 && x >= 4 && x <= 11;
                    const bool leftEye = y == 5 && x == 6;
                    const bool rightEye = y == 5 && x == 9;
                    const bool mustache = y == 6 && x >= 6 && x <= 9;
                    const bool body = y >= 7 && y <= 13 && x >= 3 && x <= 12;
                    const bool badge = y >= 8 && y <= 10 && x >= 7 && x <= 9;
                    const bool epLeft = y == 7 && x >= 3 && x <= 4;
                    const bool epRight = y == 7 && x >= 11 && x <= 12;
                    const bool belt = y == 11 && x >= 3 && x <= 12;
                    const bool leftLeg = y >= 14 && x >= 4 && x <= 6;
                    const bool rightLeg = y >= 14 && x >= 9 && x <= 11;

                    if (badge)              color = Rgba(255, 210, 50);
                    else if (epLeft || epRight)  color = Rgba(20, 50, 120);
                    else if (belt)               color = Rgba(10, 12, 18);
                    else if (body)               color = Rgba(35, 65, 160);
                    else if (hatTop)             color = Rgba(10, 20, 55);
                    else if (hatBrim)            color = Rgba(15, 28, 70);
                    else if (mustache)           color = Rgba(60, 40, 20);
                    else if (leftEye || rightEye)color = Rgba(30, 30, 30);
                    else if (face)               color = Rgba(220, 170, 120);
                    else if (leftLeg || rightLeg)color = Rgba(10, 20, 55);

                    pixels[y * TextureSize + x] = color;
                }
            }
            return pixels;
        }

        // ── 군견 (독일 셰퍼드 옆모습) ────────────────────────────────
        std::array<unsigned int, TextureSize* TextureSize> MakeDogTexture()
        {
            std::array<unsigned int, TextureSize* TextureSize> pixels = {};
            for (int y = 0; y < TextureSize; ++y)
            {
                for (int x = 0; x < TextureSize; ++x)
                {
                    unsigned int color = Rgba(20, 24, 32);

                    const unsigned int fur = Rgba(140, 80, 30);
                    const unsigned int darkFur = Rgba(60, 35, 12);
                    const unsigned int noseCol = Rgba(20, 15, 12);

                    const bool tail = (x == 0 && y == 3) || (x == 1 && y == 2) || (x == 2 && y == 1);
                    const bool body = y >= 4 && y <= 10 && x >= 2 && x <= 12;
                    const bool saddle = y >= 4 && y <= 7 && x >= 3 && x <= 10;
                    const bool head = y >= 2 && y <= 6 && x >= 10 && x <= 14;
                    const bool ear = (y == 1 || y == 2) && (x == 11 || x == 12);
                    const bool snout = y >= 6 && y <= 7 && x >= 13 && x <= 15;
                    const bool nosePixel = y == 6 && x == 15;
                    const bool eye = y == 3 && x == 12;
                    const bool frontLegs = y >= 11 && y <= 15 && (x == 10 || x == 11);
                    const bool backLegs = y >= 11 && y <= 15 && (x == 3 || x == 4);

                    if (nosePixel)              color = noseCol;
                    else if (eye)                    color = Rgba(30, 30, 30);
                    else if (ear)                    color = darkFur;
                    else if (snout)                  color = Rgba(180, 140, 90);
                    else if (head)                   color = fur;
                    else if (saddle)                 color = darkFur;
                    else if (body)                   color = fur;
                    else if (tail)                   color = fur;
                    else if (frontLegs || backLegs)  color = darkFur;

                    pixels[y * TextureSize + x] = color;
                }
            }
            return pixels;
        }

        // ── 벽 (교도소 벽돌) ─────────────────────────────────────────
        std::array<unsigned int, TextureSize* TextureSize> MakeWallTexture()
        {
            std::array<unsigned int, TextureSize* TextureSize> pixels = {};
            for (int y = 0; y < TextureSize; ++y)
            {
                for (int x = 0; x < TextureSize; ++x)
                {
                    const bool hMortar = (y % 4 == 3);
                    const int  offset = (y / 4) % 2 == 0 ? 0 : 4;
                    const bool vMortar = !hMortar && ((x + offset) % 8 == 7);

                    unsigned int color;
                    if (hMortar || vMortar)
                    {
                        color = Rgba(38, 38, 42);
                    }
                    else
                    {
                        const int brickX = (x + offset) / 8;
                        const int brickY = y / 4;
                        const bool lighter = (brickX + brickY) % 2 == 0;
                        color = lighter ? Rgba(145, 142, 150) : Rgba(120, 118, 128);
                    }
                    pixels[y * TextureSize + x] = color;
                }
            }
            return pixels;
        }

        // ── 낮은 장애물 (철조망 바) ──────────────────────────────────
        std::array<unsigned int, TextureSize* TextureSize> MakeLowBarTexture()
        {
            std::array<unsigned int, TextureSize* TextureSize> pixels = {};
            for (int y = 0; y < TextureSize; ++y)
            {
                for (int x = 0; x < TextureSize; ++x)
                {
                    unsigned int color = Rgba(65, 10, 18);

                    const bool topSpike = y <= 2 && (x % 4 == 1 || x % 4 == 2)
                        && (y < 2 - (x % 4 == 2 ? 1 : 0));
                    const bool spikeBase = y == 2;
                    const bool railTop = y >= 3 && y <= 5;
                    const bool railBottom = y >= 9 && y <= 11;
                    const bool highlight = (y == 3 || y == 9);
                    const bool base = y >= 12;

                    if (topSpike)   color = Rgba(215, 215, 220);
                    else if (spikeBase)  color = Rgba(190, 185, 195);
                    else if (highlight)  color = Rgba(220, 50, 60);
                    else if (railTop)    color = Rgba(185, 30, 42);
                    else if (railBottom) color = Rgba(185, 30, 42);
                    else if (base)       color = Rgba(50, 8, 14);

                    pixels[y * TextureSize + x] = color;
                }
            }
            return pixels;
        }

        // ── 배경 ─────────────────────────────────────────────────────
        // stage 0: 교도소 내부 (벽돌+창살창문+형광등+바닥)
        // stage 1: 교도소 외부 (밤하늘+달+별+건물실루엣+철조망+아스팔트)
        std::array<unsigned int, TextureSize* TextureSize> MakeBackgroundTexture(int stage)
        {
            std::array<unsigned int, TextureSize* TextureSize> pixels = {};
            for (int y = 0; y < TextureSize; ++y)
            {
                for (int x = 0; x < TextureSize; ++x)
                {
                    unsigned int color = Rgba(25, 25, 35);

                    if (stage == 0)
                    {
                        const bool floor = y >= 12;
                        const bool floorCrack = y == 12 && (x == 3 || x == 9);
                        const bool hMortar = !floor && (y % 4 == 3);
                        const int  bOff = (y / 4) % 2 == 0 ? 0 : 4;
                        const bool vMortar = !floor && !hMortar && ((x + bOff) % 8 == 7);

                        const bool windowBg = x >= 10 && x <= 14 && y >= 1 && y <= 6;
                        const bool windowBar = windowBg && (x == 11 || x == 13);
                        const bool windowFrame = (x == 10 || x == 14) && y >= 1 && y <= 6;
                        const bool windowTop = y == 1 && x >= 10 && x <= 14;
                        const bool windowBot = y == 6 && x >= 10 && x <= 14;
                        const bool lamp = y == 0 && x >= 1 && x <= 5;

                        if (lamp)                              color = Rgba(240, 235, 190);
                        else if (windowBar)                         color = Rgba(40, 45, 55);
                        else if (windowFrame || windowTop || windowBot) color = Rgba(55, 60, 70);
                        else if (windowBg)                          color = Rgba(70, 110, 160);
                        else if (floorCrack)                        color = Rgba(30, 30, 36);
                        else if (floor)                             color = Rgba(50, 50, 60);
                        else if (hMortar || vMortar)                color = Rgba(28, 30, 38);
                        else
                        {
                            const int bx = (x + bOff) / 8;
                            const int by = y / 4;
                            color = (bx + by) % 2 == 0 ? Rgba(95, 88, 100) : Rgba(78, 72, 84);
                        }
                    }
                    else
                    {
                        const bool sky = y <= 7;
                        const bool moon = y >= 1 && y <= 4 && x >= 11 && x <= 14;
                        const bool moonShad = y >= 1 && y <= 3 && x >= 13 && x <= 14;
                        const bool star1 = y == 0 && x == 2;
                        const bool star2 = y == 1 && x == 6;
                        const bool star3 = y == 3 && x == 1;
                        const bool star4 = y == 2 && x == 9;
                        const bool bldg = y >= 6 && y <= 9 &&
                            ((x >= 0 && x <= 2) || (x >= 5 && x <= 7) || (x >= 12 && x <= 15));
                        const bool bldgWin = y == 8 && (x == 1 || x == 6 || x == 13);
                        const bool fencePost = y >= 8 && y <= 12 && (x % 5 == 0);
                        const bool fenceWire = (y == 9 || y == 11) && y <= 12;
                        const bool road = y >= 13;
                        const bool roadLine = y == 13;

                        if (moonShad)   color = Rgba(200, 195, 140);
                        else if (moon)       color = Rgba(245, 238, 170);
                        else if (star1 || star2 || star3 || star4) color = Rgba(240, 240, 200);
                        else if (bldgWin)    color = Rgba(220, 190, 80);
                        else if (bldg)       color = Rgba(22, 24, 32);
                        else if (fencePost)  color = Rgba(85, 90, 95);
                        else if (fenceWire)  color = Rgba(70, 75, 78);
                        else if (roadLine)   color = Rgba(55, 55, 58);
                        else if (road)       color = Rgba(42, 42, 48);
                        else if (sky)
                        {
                            const unsigned int dark = 40 + y * 8;
                            const unsigned int b = dark + 30 < 255u ? dark + 30 : 255u;
                            color = Rgba(dark / 2, dark * 3 / 4, b);
                        }
                        else color = Rgba(30, 45, 70);
                    }

                    pixels[y * TextureSize + x] = color;
                }
            }
            return pixels;
        }
    }

    void RunnerGameScene::Initialize(GraphicsContext& graphics)
    {
        resources.Initialize(graphics);

        rng.seed(12345);

        // ── 스프라이트 시트 로딩 ──────────────────────────────────────
        // Textures/ 폴더에 SpriteSheet_Player.png 가 있어야 함
        Material* spriteMat = resources.LoadMaterialFromFile(
            "PlayerSprite", { 1, 1, 1, 1 }, L"Textures/SpriteSheet_Player.png");

        // 로딩 실패 시 기존 픽셀맵 텍스처로 폴백
        if (!spriteMat)
        {
            const auto prisonerRunTexture = MakePrisonerRunTexture();
            const auto prisonerSlideTexture = MakePrisonerSlideTexture();
            spriteMat = resources.CreateTexturedMaterial(
                "PlayerSprite", { 1, 1, 1, 1 }, prisonerRunTexture.data(), TextureSize, TextureSize);
            player.runMaterial = spriteMat;
            player.slideMaterial = resources.CreateTexturedMaterial(
                "PlayerSlide", { 1, 1, 1, 1 }, prisonerSlideTexture.data(), TextureSize, TextureSize);
        }
        else
        {
            // 스프라이트 시트 모드: UV 오프셋 초기화
            spriteMat->SetSpriteUV(0.0f, Player::RowRun, Player::FrameWidth, Player::FrameHeight);
        }
        player.spriteMaterial = spriteMat;

        const auto policeTexture = MakePoliceTexture();
        const auto dogTexture = MakeDogTexture();
        const auto wallTexture = MakeWallTexture();
        const auto lowBarTexture = MakeLowBarTexture();
        const auto background0 = MakeBackgroundTexture(0);
        const auto background1 = MakeBackgroundTexture(1);

        Material* groundMaterial = resources.CreateMaterial("Ground", { 0.20f, 0.78f, 0.42f, 1.0f });
        Material* jailMaterial = resources.CreateMaterial("JailBars", { 0.03f, 0.04f, 0.06f, 1.0f });
        policeMaterial = resources.CreateTexturedMaterial("Police", { 1, 1, 1, 1 }, policeTexture.data(), TextureSize, TextureSize);
        dogMaterial = resources.CreateTexturedMaterial("Dog", { 1, 1, 1, 1 }, dogTexture.data(), TextureSize, TextureSize);
        wallMaterial = resources.CreateTexturedMaterial("Wall", { 1, 1, 1, 1 }, wallTexture.data(), TextureSize, TextureSize);
        lowBarMaterial = resources.CreateTexturedMaterial("LowBar", { 1, 1, 1, 1 }, lowBarTexture.data(), TextureSize, TextureSize);
        backgroundMaterials[0] = resources.CreateTexturedMaterial("BackgroundPrisonInside", { 1, 1, 1, 1 }, background0.data(), TextureSize, TextureSize);
        backgroundMaterials[1] = resources.CreateTexturedMaterial("BackgroundEscapeOutside", { 1, 1, 1, 1 }, background1.data(), TextureSize, TextureSize);
        Mesh* quad = resources.GetMesh("Quad");

        GameObject& backgroundVisual = CreateObject("Background");
        backgroundVisual.GetTransform().position = { 0.0f, 0.15f };
        backgroundVisual.GetTransform().scale = { 2.4f, 1.9f };
        backgroundVisual.AddComponent<MeshRenderer>(quad, backgroundMaterials[0]);
        background = &backgroundVisual;

        GameObject& playerVisual = CreateObject("PlayerUniform");
        player.normalHeight = hitboxData.playerStandingHeight;
        player.slideHeight = hitboxData.playerSlideHeight;
        playerVisual.GetTransform().position = player.position;
        playerVisual.GetTransform().scale = { hitboxData.playerWidth, player.normalHeight };
        playerVisual.AddComponent<MeshRenderer>(quad, spriteMat);
        player.visual = &playerVisual;
        player.collider = { player.position, hitboxData.playerWidth, player.normalHeight };

        GameObject& ground = CreateObject("Ground");
        ground.GetTransform().position = { 0.0f, -0.55f };
        ground.GetTransform().scale = { 2.4f, 0.12f };
        ground.AddComponent<MeshRenderer>(quad, groundMaterial);

        for (int i = 0; i < MaxObstacleVisuals; ++i)
        {
            GameObject& obstacleVisual = CreateObject("Obstacle");
            obstacleVisual.GetTransform().position = { -2.0f, -2.0f };
            obstacleVisual.GetTransform().scale = { 0.1f, 0.1f };
            obstacleVisual.AddComponent<MeshRenderer>(quad, policeMaterial);
            obstacleVisuals.push_back(&obstacleVisual);
        }

        for (int i = 0; i < 7; ++i)
        {
            GameObject& bar = CreateObject("JailBar");
            bar.GetTransform().position = { -2.0f, -2.0f };
            bar.GetTransform().scale = { 0.04f, 0.44f };
            bar.AddComponent<MeshRenderer>(quad, jailMaterial);
            jailBars.push_back(&bar);
        }

        ResetGame();
    }

    void RunnerGameScene::Update(float dt)
    {
        if (Input::WasKeyPressed('R'))
        {
            ResetGame();
        }

        if (gameState == GameState::Play)
        {
            if (Input::WasKeyPressed('P'))
            {
                score += 1000;
            }

            scoreAccumulator += dt;
            while (scoreAccumulator >= 0.05f)
            {
                ++score;
                scoreAccumulator -= 0.05f;
            }

            scrollSpeed = speedData.baseSpeed * GetDifficulty();

            UpdateBackground();

            UpdatePlayer(dt);
            UpdateObstacles(dt);

            const AABB forgivingBox = GetForgivingPlayerBox();
            for (const Obstacle& obstacle : obstacles)
            {
                if (CheckOverlap(forgivingBox, obstacle))
                {
                    gameState = GameState::GameOver;
                    break;
                }
            }
        }
        else
        {
            UpdateGameOverBars(dt);
        }

        Scene::Update(dt);
    }

    void RunnerGameScene::Render(GraphicsContext& graphics)
    {
        Scene::Render(graphics);

        if (gameState == GameState::Play)
        {
            graphics.QueueText(L"SCORE " + std::to_wstring(score), 24, 20, 30, { 1.0f, 0.95f, 0.30f, 1.0f });
        }
        else
        {
            graphics.QueueText(L"GAME OVER", 455, 230, 48, { 1.0f, 0.18f, 0.12f, 1.0f });
            graphics.QueueText(L"R RESTART", 462, 285, 32, { 1.0f, 1.0f, 1.0f, 1.0f });
        }
    }

    void RunnerGameScene::UpdatePlayer(float dt)
    {
        const bool jumpPressed = Input::WasKeyPressed(VK_SPACE);
        const bool jumpReleased = Input::WasKeyReleased(VK_SPACE);
        const bool slideHeld = Input::IsKeyDown(VK_DOWN);

        if (jumpPressed && player.isGrounded)
        {
            player.state = PlayerState::Jump;
            player.velocity.y = player.jumpPower;
            player.isGrounded = false;
        }

        if (jumpReleased && player.velocity.y > 0.0f)
        {
            player.velocity.y *= 0.5f;
        }

        if (slideHeld && player.isGrounded)
        {
            player.state = PlayerState::Slide;
            player.collider.height = player.slideHeight;
        }
        else if (player.isGrounded)
        {
            player.state = PlayerState::Run;
            player.collider.height = player.normalHeight;
        }

        if (!player.isGrounded)
        {
            player.velocity.y += player.gravity * dt;
            player.position.y += player.velocity.y * dt;

            if (player.position.y <= player.groundY)
            {
                player.position.y = player.groundY;
                player.velocity.y = 0.0f;
                player.isGrounded = true;
                player.state = PlayerState::Run;
                player.collider.height = player.normalHeight;
            }
        }

        player.collider.center = player.position;

        // 애니메이션 프레임 업데이트
        player.frameTimer += dt;
        if (player.frameTimer >= 1.0f / Player::Fps)
        {
            player.frameTimer -= 1.0f / Player::Fps;
            player.currentFrame = (player.currentFrame + 1) % Player::FrameCount;
        }

        UpdatePlayerVisual();
    }

    void RunnerGameScene::UpdateObstacles(float dt)
    {
        for (Obstacle& obstacle : obstacles)
        {
            obstacle.position.x -= scrollSpeed * dt;
            if (obstacle.visual)
            {
                obstacle.visual->GetTransform().position = obstacle.position;
            }
        }

        obstacles.erase(
            std::remove_if(obstacles.begin(), obstacles.end(),
                [this](Obstacle& obstacle)
                {
                    if (obstacle.position.x >= spawnData.offscreenX)
                    {
                        return false;
                    }
                    if (obstacle.visual)
                    {
                        obstacle.visual->GetTransform().position = { -2.0f, -2.0f };
                    }
                    return true;
                }),
            obstacles.end());

        while (obstacles.size() < static_cast<size_t>(spawnData.targetObstacleCount))
        {
            SpawnNextObstacle();
        }
    }

    void RunnerGameScene::UpdateBackground()
    {
        const int backgroundStage = (score / 1000) % 2;

        if (backgroundStage == currentBackgroundStage || !background)
        {
            return;
        }

        if (MeshRenderer* renderer = background->GetComponent<MeshRenderer>())
        {
            renderer->SetMaterial(backgroundMaterials[backgroundStage]);
        }
        currentBackgroundStage = backgroundStage;
    }

    void RunnerGameScene::UpdateGameOverBars(float dt)
    {
        jailDropOffsetY -= jailDropSpeed * dt;
        if (jailDropOffsetY < 0.0f)
        {
            jailDropOffsetY = 0.0f;
        }

        const Vec2 cageCenter = { player.position.x + 0.03f, player.position.y + 0.16f + jailDropOffsetY };
        for (size_t i = 0; i < jailBars.size(); ++i)
        {
            if (i < 5)
            {
                jailBars[i]->GetTransform().position = { cageCenter.x - 0.22f + static_cast<float>(i) * 0.11f, cageCenter.y };
                jailBars[i]->GetTransform().scale = { 0.035f, 0.46f };
            }
            else
            {
                jailBars[i]->GetTransform().position = { cageCenter.x, cageCenter.y + (i == 5 ? 0.23f : -0.23f) };
                jailBars[i]->GetTransform().scale = { 0.50f, 0.035f };
            }
        }
    }

    void RunnerGameScene::ResetGame()
    {
        player.position = { -0.55f, -0.37f };
        player.velocity = { 0.0f, 0.0f };
        player.state = PlayerState::Run;
        player.isGrounded = true;
        player.normalHeight = hitboxData.playerStandingHeight;
        player.slideHeight = hitboxData.playerSlideHeight;
        player.collider = { player.position, hitboxData.playerWidth, player.normalHeight };
        player.currentFrame = 0;
        player.frameTimer = 0.0f;

        score = 0;
        scoreAccumulator = 0.0f;
        currentBackgroundStage = -1;
        gameState = GameState::Play;
        scrollSpeed = speedData.baseSpeed;
        jailDropOffsetY = 1.7f;

        obstacles.clear();
        for (GameObject* visual : obstacleVisuals)
        {
            visual->GetTransform().position = { -2.0f, -2.0f };
        }

        SpawnObstacle(0, spawnData.firstSpawnX, Obstacle::WALL, 0.14f, 0.18f);
        SpawnNextObstacle();
        SpawnNextObstacle();
        SpawnNextObstacle();

        UpdatePlayerVisual();
        UpdateBackground();

        for (GameObject* bar : jailBars)
        {
            bar->GetTransform().position = { -2.0f, -2.0f };
        }
    }

    void RunnerGameScene::SpawnObstacle(int visualIndex, float x, Obstacle::Type type, float width, float height)
    {
        if (visualIndex < 0 || visualIndex >= static_cast<int>(obstacleVisuals.size()))
        {
            return;
        }

        Obstacle obstacle;
        obstacle.visual = obstacleVisuals[visualIndex];
        obstacle.position =
        {
            x,
            type == Obstacle::LOW_BAR ? GetLowBarCenterY(height) : GetGroundObstacleY(height)
        };
        obstacle.width = width;
        obstacle.height = height;
        obstacle.type = type;

        obstacle.visual->GetTransform().position = obstacle.position;
        obstacle.visual->GetTransform().scale = { obstacle.width, obstacle.height };
        if (MeshRenderer* renderer = obstacle.visual->GetComponent<MeshRenderer>())
        {
            renderer->SetMaterial(GetObstacleMaterial(type));
        }
        obstacles.push_back(obstacle);
        std::sort(obstacles.begin(), obstacles.end(),
            [](const Obstacle& left, const Obstacle& right)
            {
                return left.position.x < right.position.x;
            });
    }

    void RunnerGameScene::SpawnObstacleByType(float x, Obstacle::Type type)
    {
        SpawnObstacle(FindFreeObstacleVisual(), x, type, GetObstacleWidth(type), GetObstacleHeight(type));
    }

    void RunnerGameScene::SpawnNextObstacle()
    {
        if (obstacles.size() >= obstacleVisuals.size())
        {
            return;
        }

        const float jumpAirTime = (-2.0f * player.jumpPower) / player.gravity;
        const float safeJumpGap = scrollSpeed * jumpAirTime + spawnData.safetyPadding;
        const int   scoreAfterAdaptation = score > speedData.adaptationScore ? score - speedData.adaptationScore : 0;
        const float tempoGap = Clamp(
            spawnData.baseGap - static_cast<float>(scoreAfterAdaptation) * spawnData.scoreGapReduction,
            spawnData.absoluteMinGap,
            spawnData.absoluteMaxGap);
        const float minClearance = (std::max)(tempoGap, safeJumpGap * 0.75f);

        float previousX = spawnData.firstSpawnX;
        for (const Obstacle& obstacle : obstacles)
        {
            if (obstacle.position.x > previousX)
            {
                previousX = obstacle.position.x;
            }
        }

        std::uniform_real_distribution<float> extraGap(spawnData.minExtraGap, spawnData.maxExtraGap);
        std::uniform_int_distribution<int>    typeRoll(0, 9);

        const int roll = typeRoll(rng);
        const Obstacle::Type type =
            roll <= 2 ? Obstacle::POLICE :
            roll <= 4 ? Obstacle::DOG :
            roll <= 7 ? Obstacle::WALL :
            Obstacle::LOW_BAR;
        const float spawnX = previousX + minClearance + extraGap(rng);

        const bool canSpawnPair =
            score >= spawnData.pairStartScore &&
            obstacles.size() + 1 < obstacleVisuals.size() &&
            RandomChance(spawnData.pairChance);

        if (canSpawnPair)
        {
            std::uniform_int_distribution<int> pairRoll(0, 2);
            const int pair = pairRoll(rng);

            Obstacle::Type first = Obstacle::DOG;
            Obstacle::Type second = Obstacle::LOW_BAR;
            if (pair == 1)
            {
                first = Obstacle::POLICE;
                second = Obstacle::WALL;
            }
            else if (pair == 2)
            {
                first = Obstacle::WALL;
                second = Obstacle::DOG;
            }

            const float pairGap = minClearance * spawnData.pairGapScale;
            SpawnObstacleByType(spawnX, first);
            SpawnObstacleByType(spawnX + pairGap, second);
        }
        else
        {
            SpawnObstacleByType(spawnX, type);
        }
    }

    int RunnerGameScene::FindFreeObstacleVisual() const
    {
        for (int visualIndex = 0; visualIndex < static_cast<int>(obstacleVisuals.size()); ++visualIndex)
        {
            bool used = false;
            for (const Obstacle& obstacle : obstacles)
            {
                if (obstacle.visual == obstacleVisuals[visualIndex])
                {
                    used = true;
                    break;
                }
            }
            if (!used)
            {
                return visualIndex;
            }
        }
        return -1;
    }

    void RunnerGameScene::UpdatePlayerVisual()
    {
        if (!player.visual)
        {
            return;
        }

        // 상태에 따라 스케일 및 위치 설정
        if (player.state == PlayerState::Slide)
        {
            player.visual->GetTransform().scale = { player.collider.width * 2.2f, player.collider.height };
            player.visual->GetTransform().position = player.position;
        }
        else
        {
            player.visual->GetTransform().scale = { player.collider.width, player.collider.height };
            player.visual->GetTransform().position = player.position;
        }

        // 스프라이트 시트 UV 업데이트
        if (player.spriteMaterial)
        {
            // 행 선택: Run=0, Jump=1, Slide=2
            float rowV = Player::RowRun;
            if (player.state == PlayerState::Jump)  rowV = Player::RowJump;
            if (player.state == PlayerState::Slide) rowV = Player::RowSlide;

            // 현재 프레임의 U 오프셋
            const float frameU = static_cast<float>(player.currentFrame) * Player::FrameWidth;

            player.spriteMaterial->SetSpriteUV(frameU, rowV, Player::FrameWidth, Player::FrameHeight);

            if (MeshRenderer* renderer = player.visual->GetComponent<MeshRenderer>())
            {
                renderer->SetMaterial(player.spriteMaterial);
            }
        }
        else if (player.runMaterial && player.slideMaterial)
        {
            // 폴백: 기존 텍스처 방식
            Material* target = (player.state == PlayerState::Slide)
                ? player.slideMaterial : player.runMaterial;
            if (MeshRenderer* renderer = player.visual->GetComponent<MeshRenderer>())
            {
                renderer->SetMaterial(target);
            }
        }
    }

    bool RunnerGameScene::CheckOverlap(const AABB& playerBox, const Obstacle& obstacle) const
    {
        const AABB obstacleBox = GetObstacleBox(obstacle);
        const bool xOverlap = std::abs(playerBox.center.x - obstacleBox.center.x) <=
            ((playerBox.width + obstacleBox.width) * 0.5f);
        const bool yOverlap = std::abs(playerBox.center.y - obstacleBox.center.y) <=
            ((playerBox.height + obstacleBox.height) * 0.5f);
        return xOverlap && yOverlap;
    }

    AABB RunnerGameScene::GetForgivingPlayerBox() const
    {
        AABB box = player.collider;
        box.width *= hitboxData.forgivingScale;
        box.height *= hitboxData.forgivingScale;
        return box;
    }

    AABB RunnerGameScene::GetObstacleBox(const Obstacle& obstacle) const
    {
        return { obstacle.position, obstacle.width, obstacle.height };
    }

    float RunnerGameScene::GetDifficulty() const
    {
        if (score <= speedData.adaptationScore)
        {
            return 1.0f;
        }
        const int   scoreAfterAdaptation = score - speedData.adaptationScore;
        const float difficulty = 1.0f + static_cast<float>(scoreAfterAdaptation) / speedData.scorePerDifficulty;
        return Clamp(difficulty, 1.0f, speedData.maxDifficulty);
    }

    float RunnerGameScene::GetObstacleWidth(Obstacle::Type type) const
    {
        if (type == Obstacle::DOG)    return 0.18f;
        if (type == Obstacle::LOW_BAR)return 0.28f;
        return 0.14f;
    }

    float RunnerGameScene::GetObstacleHeight(Obstacle::Type type) const
    {
        if (type == Obstacle::DOG)    return 0.12f;
        if (type == Obstacle::LOW_BAR)return 0.08f;
        return 0.18f;
    }

    bool RunnerGameScene::RandomChance(float chance)
    {
        std::uniform_real_distribution<float> roll(0.0f, 1.0f);
        return roll(rng) < chance;
    }

    float RunnerGameScene::GetStandingPlayerTopY() const
    {
        return player.groundY + player.normalHeight * hitboxData.forgivingScale * 0.5f;
    }

    float RunnerGameScene::GetSlidingPlayerTopY() const
    {
        return player.groundY + player.slideHeight * hitboxData.forgivingScale * 0.5f;
    }

    float RunnerGameScene::GetGroundSurfaceY() const
    {
        return player.groundY - player.normalHeight * 0.5f;
    }

    float RunnerGameScene::GetGroundObstacleY(float height) const
    {
        return GetGroundSurfaceY() + height * 0.5f;
    }

    float RunnerGameScene::GetLowBarCenterY(float lowBarHeight) const
    {
        const float lowBarBottomY = GetStandingPlayerTopY() - hitboxData.lowBarStandingOverlap;
        const float slideTopY = GetSlidingPlayerTopY();

        if (lowBarBottomY <= slideTopY)
        {
            return slideTopY + lowBarHeight * 0.5f + 0.01f;
        }
        return lowBarBottomY + lowBarHeight * 0.5f;
    }

    Material* RunnerGameScene::GetObstacleMaterial(Obstacle::Type type) const
    {
        switch (type)
        {
        case Obstacle::DOG:     return dogMaterial;
        case Obstacle::WALL:    return wallMaterial;
        case Obstacle::LOW_BAR: return lowBarMaterial;
        case Obstacle::POLICE:
        default:                return policeMaterial;
        }
    }
}