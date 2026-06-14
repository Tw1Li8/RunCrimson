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
        constexpr int DetailTextureSize = 64;

        unsigned int Rgba(unsigned int r, unsigned int g, unsigned int b)
        {
            return 0xff000000u | (b << 16) | (g << 8) | r;
        }

        bool InRect(int x, int y, int left, int top, int right, int bottom)
        {
            return x >= left && x <= right && y >= top && y <= bottom;
        }

        bool InEllipse(int x, int y, float cx, float cy, float rx, float ry)
        {
            const float dx = (static_cast<float>(x) - cx) / rx;
            const float dy = (static_cast<float>(y) - cy) / ry;
            return dx * dx + dy * dy <= 1.0f;
        }

        unsigned int Shade(unsigned int baseR, unsigned int baseG, unsigned int baseB, int amount)
        {
            const auto clamp = [](int value)
                {
                    return static_cast<unsigned int>(std::clamp(value, 0, 255));
                };
            return Rgba(clamp(static_cast<int>(baseR) + amount),
                clamp(static_cast<int>(baseG) + amount),
                clamp(static_cast<int>(baseB) + amount));
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
        std::array<unsigned int, DetailTextureSize* DetailTextureSize> MakePoliceTexture()
        {
            std::array<unsigned int, DetailTextureSize* DetailTextureSize> pixels = {};
            for (int y = 0; y < DetailTextureSize; ++y)
            {
                for (int x = 0; x < DetailTextureSize; ++x)
                {
                    unsigned int color = Rgba(0, 0, 0);

                    const bool head = InEllipse(x, y, 32.0f, 19.0f, 13.0f, 11.0f);
                    const bool earLeft = InEllipse(x, y, 18.0f, 20.0f, 3.5f, 5.0f);
                    const bool earRight = InEllipse(x, y, 46.0f, 20.0f, 3.5f, 5.0f);
                    const bool cap = InRect(x, y, 19, 7, 45, 14) || InRect(x, y, 15, 14, 49, 17);
                    const bool capBadge = InRect(x, y, 30, 8, 34, 12);
                    const bool neck = InRect(x, y, 28, 29, 36, 34);
                    const bool torso = InRect(x, y, 18, 33, 46, 52);
                    const bool shoulder = InRect(x, y, 13, 35, 51, 41);
                    const bool belt = InRect(x, y, 18, 48, 46, 51);
                    const bool legLeft = InRect(x, y, 22, 52, 29, 62);
                    const bool legRight = InRect(x, y, 35, 52, 42, 62);
                    const bool armLeft = InRect(x, y, 11, 40, 17, 54);
                    const bool armRight = InRect(x, y, 47, 38, 53, 52);

                    if (armLeft || armRight) color = Rgba(22, 43, 115);
                    if (shoulder || torso) color = Shade(35, 70, 165, (x + y) % 7);
                    if (legLeft || legRight || belt) color = Rgba(10, 18, 48);
                    if (neck || earLeft || earRight || head) color = Rgba(220, 170, 120);
                    if (InRect(x, y, 24, 20, 26, 22) || InRect(x, y, 38, 20, 40, 22)) color = Rgba(24, 24, 24);
                    if (InRect(x, y, 27, 26, 37, 28)) color = Rgba(78, 42, 22);
                    if (cap) color = Rgba(9, 22, 66);
                    if (InRect(x, y, 15, 17, 49, 19)) color = Rgba(18, 36, 92);
                    if (capBadge || InRect(x, y, 31, 38, 36, 44)) color = Rgba(245, 205, 55);
                    if (InRect(x, y, 20, 34, 44, 36) || InRect(x, y, 20, 45, 44, 46)) color = Rgba(58, 92, 188);

                    pixels[y * DetailTextureSize + x] = color;
                }
            }
            return pixels;
        }

        // ── 군견 (독일 셰퍼드 옆모습) ────────────────────────────────
        std::array<unsigned int, DetailTextureSize* DetailTextureSize> MakeDogTexture()
        {
            std::array<unsigned int, DetailTextureSize* DetailTextureSize> pixels = {};
            for (int y = 0; y < DetailTextureSize; ++y)
            {
                for (int x = 0; x < DetailTextureSize; ++x)
                {
                    unsigned int color = Rgba(0, 0, 0);

                    const unsigned int fur = Rgba(140, 80, 30);
                    const unsigned int darkFur = Rgba(60, 35, 12);

                    const bool tail = (x >= 3 && x <= 18 && y >= 18 - x / 3 && y <= 23 - x / 3);
                    const bool body = InEllipse(x, y, 29.0f, 32.0f, 20.0f, 12.0f);
                    const bool head = InEllipse(x, y, 48.0f, 25.0f, 10.0f, 9.0f);
                    const bool snout = InEllipse(x, y, 57.0f, 28.0f, 7.0f, 5.0f);
                    const bool ear = InRect(x, y, 43, 11, 49, 22) || InRect(x, y, 49, 13, 54, 23);
                    const bool legBack = InRect(x, y, 16, 39, 22, 58) || InRect(x, y, 27, 40, 32, 58);
                    const bool legFront = InRect(x, y, 42, 38, 47, 58) || InRect(x, y, 51, 37, 56, 58);
                    const bool paw = InRect(x, y, 14, 57, 24, 61) || InRect(x, y, 25, 57, 34, 61) ||
                        InRect(x, y, 40, 57, 49, 61) || InRect(x, y, 49, 57, 59, 61);

                    if (tail || body || head || legBack || legFront) color = fur;
                    if (InRect(x, y, 18, 23, 41, 32) || ear) color = darkFur;
                    if (snout) color = Rgba(184, 135, 78);
                    if (paw) color = Rgba(70, 42, 18);
                    if (InRect(x, y, 51, 22, 53, 24)) color = Rgba(18, 18, 18);
                    if (InRect(x, y, 61, 27, 63, 30)) color = Rgba(18, 12, 10);
                    if (body && ((x + y) % 9 == 0)) color = Rgba(164, 96, 38);

                    pixels[y * DetailTextureSize + x] = color;
                }
            }
            return pixels;
        }

        // ── 벽 (교도소 벽돌) ─────────────────────────────────────────
        std::array<unsigned int, DetailTextureSize* DetailTextureSize> MakeWallTexture()
        {
            std::array<unsigned int, DetailTextureSize* DetailTextureSize> pixels = {};
            for (int y = 0; y < DetailTextureSize; ++y)
            {
                for (int x = 0; x < DetailTextureSize; ++x)
                {
                    const bool hMortar = (y % 10 == 9);
                    const int  offset = (y / 10) % 2 == 0 ? 0 : 8;
                    const bool vMortar = !hMortar && ((x + offset) % 16 == 15);

                    unsigned int color;
                    if (hMortar || vMortar)
                    {
                        color = Rgba(40, 40, 44);
                    }
                    else
                    {
                        const int brickX = (x + offset) / 16;
                        const int brickY = y / 10;
                        const int grain = ((x * 13 + y * 7) % 11) - 5;
                        const bool chip = (x + y * 3) % 29 == 0 || (x * 5 + y) % 37 == 0;
                        color = (brickX + brickY) % 2 == 0 ? Shade(142, 138, 148, grain) : Shade(116, 112, 126, grain);
                        if (chip) color = Rgba(76, 74, 82);
                    }
                    pixels[y * DetailTextureSize + x] = color;
                }
            }
            return pixels;
        }

        // ── 낮은 장애물 (철조망 바) ──────────────────────────────────
        std::array<unsigned int, DetailTextureSize* DetailTextureSize> MakeLowBarTexture()
        {
            std::array<unsigned int, DetailTextureSize* DetailTextureSize> pixels = {};
            for (int y = 0; y < DetailTextureSize; ++y)
            {
                for (int x = 0; x < DetailTextureSize; ++x)
                {
                    unsigned int color = Rgba(0, 0, 0);

                    const bool railTop = InRect(x, y, 0, 16, 63, 25);
                    const bool railBottom = InRect(x, y, 0, 39, 63, 48);
                    const bool post = (x % 16 >= 6 && x % 16 <= 10) && y >= 14 && y <= 53;
                    const bool base = y >= 54;
                    const bool highlight = y == 16 || y == 39 || (post && x % 16 == 6);
                    const bool spikeBase = y >= 12 && y <= 15;
                    bool spike = false;
                    for (int sx = 8; sx < 64; sx += 16)
                    {
                        const int dx = std::abs(x - sx);
                        if (dx <= 6 && y >= 2 && y <= 13 - dx)
                        {
                            spike = true;
                        }
                    }

                    if (spike) color = Rgba(226, 226, 232);
                    else if (spikeBase) color = Rgba(170, 170, 178);
                    else if (highlight) color = Rgba(235, 62, 72);
                    else if (railTop || railBottom || post) color = Rgba(177, 28, 42);
                    else if (base) color = Rgba(58, 9, 15);
                    if ((railTop || railBottom) && (x + y) % 17 == 0) color = Rgba(118, 18, 28);

                    pixels[y * DetailTextureSize + x] = color;
                }
            }
            return pixels;
        }

        // ── 배경 ─────────────────────────────────────────────────────
        // stage 0: 교도소 내부 (벽돌+창살창문+형광등+바닥)
        // stage 1: 교도소 외부 (밤하늘+달+별+건물실루엣+철조망+아스팔트)
        std::array<unsigned int, DetailTextureSize* DetailTextureSize> MakeBackgroundTexture(int stage)
        {
            std::array<unsigned int, DetailTextureSize* DetailTextureSize> pixels = {};
            for (int y = 0; y < DetailTextureSize; ++y)
            {
                for (int x = 0; x < DetailTextureSize; ++x)
                {
                    unsigned int color = Rgba(25, 25, 35);

                    if (stage == 0)
                    {
                        const bool floor = y >= 48;
                        const bool floorCrack = (y == 50 && x >= 8 && x <= 17) || (y == 56 && x >= 35 && x <= 49) ||
                            (x == 43 && y >= 52 && y <= 60);
                        const bool hMortar = !floor && (y % 10 == 9);
                        const int  bOff = (y / 10) % 2 == 0 ? 0 : 8;
                        const bool vMortar = !floor && !hMortar && ((x + bOff) % 16 == 15);

                        const bool windowBg = InRect(x, y, 42, 6, 58, 29);
                        const bool windowBar = windowBg && (x == 47 || x == 53 || y == 17);
                        const bool windowFrame = (x == 41 || x == 59) && y >= 5 && y <= 30;
                        const bool windowTop = y == 5 && x >= 41 && x <= 59;
                        const bool windowBot = y == 30 && x >= 41 && x <= 59;
                        const bool lamp = InEllipse(x, y, 14.0f, 8.0f, 10.0f, 4.0f);
                        const bool lampGlow = InEllipse(x, y, 14.0f, 10.0f, 18.0f, 9.0f);
                        const bool pipe = InRect(x, y, 5, 18, 8, 47) || InRect(x, y, 5, 18, 23, 21);
                        const bool poster = InRect(x, y, 23, 24, 34, 39);

                        if (lampGlow)                          color = Rgba(75, 68, 62);
                        if (lamp)                              color = Rgba(242, 232, 168);
                        else if (windowBar)                         color = Rgba(40, 45, 55);
                        else if (windowFrame || windowTop || windowBot) color = Rgba(55, 60, 70);
                        else if (windowBg)                          color = Rgba(54, 90, 138);
                        else if (pipe)                              color = Rgba(58, 62, 68);
                        else if (poster)                            color = ((x + y) % 5 == 0) ? Rgba(118, 98, 72) : Rgba(152, 128, 88);
                        else if (floorCrack)                        color = Rgba(30, 30, 36);
                        else if (floor)                             color = Shade(50, 50, 60, ((x * 3 + y) % 9) - 4);
                        else if (hMortar || vMortar)                color = Rgba(28, 30, 38);
                        else
                        {
                            const int bx = (x + bOff) / 16;
                            const int by = y / 10;
                            color = (bx + by) % 2 == 0 ? Shade(94, 87, 99, ((x + y) % 7) - 3) : Shade(78, 72, 84, ((x * 2 + y) % 7) - 3);
                        }
                    }
                    else
                    {
                        const bool moon = InEllipse(x, y, 51.0f, 11.0f, 8.0f, 8.0f);
                        const bool moonShad = InEllipse(x, y, 55.0f, 8.0f, 6.0f, 7.0f);
                        const bool star = (x * 17 + y * 23) % 113 == 0 && y < 27;
                        const bool bldg = (InRect(x, y, 0, 27, 11, 43) || InRect(x, y, 19, 23, 31, 43) ||
                            InRect(x, y, 47, 29, 63, 43));
                        const bool bldgWin = bldg && y % 7 == 2 && x % 5 == 1;
                        const bool fencePost = y >= 36 && y <= 52 && (x % 10 == 0);
                        const bool fenceWire = (y == 40 || y == 47) && y <= 52;
                        const bool road = y >= 51;
                        const bool roadLine = y == 55 && (x / 8) % 2 == 0;

                        if (moonShad) color = Rgba(185, 184, 138);
                        else if (moon) color = Rgba(245, 238, 170);
                        else if (star) color = Rgba(240, 240, 200);
                        else if (bldgWin) color = Rgba(220, 190, 80);
                        else if (bldg) color = Rgba(20, 23, 32);
                        else if (fencePost) color = Rgba(86, 92, 98);
                        else if (fenceWire) color = Rgba(70, 76, 82);
                        else if (roadLine) color = Rgba(120, 118, 90);
                        else if (road) color = Shade(42, 42, 48, ((x + y * 2) % 7) - 3);
                        else
                        {
                            const unsigned int dark = 34 + y * 3;
                            const unsigned int b = dark + 30 < 255u ? dark + 30 : 255u;
                            color = Rgba(dark / 2, dark * 3 / 4, b);
                        }
                    }

                    pixels[y * DetailTextureSize + x] = color;
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

        Material* groundMaterial = resources.LoadMaterialFromFile(
            "Ground", { 0.90f, 0.90f, 0.86f, 1.0f }, L"Textures/Ground_Prison_Run.png");
        if (!groundMaterial)
        {
            groundMaterial = resources.CreateMaterial("Ground", { 0.22f, 0.22f, 0.22f, 1.0f });
        }
        Material* jailMaterial = resources.CreateMaterial("JailBars", { 0.03f, 0.04f, 0.06f, 1.0f });
        Material* gameOverOverlayMaterial = resources.CreateMaterial("GameOverOverlay", { 0.0f, 0.0f, 0.0f, 0.45f });
        Material* jailCageMaterial = resources.LoadMaterialFromFile(
            "JailCage", { 1, 1, 1, 1 }, L"Textures/GameOver_JailCage.png");
        if (!jailCageMaterial)
        {
            jailCageMaterial = jailMaterial;
        }
        policeMaterial = resources.LoadMaterialFromFile(
            "Police", { 1, 1, 1, 1 }, L"Textures/Obstacle_Police.png");
        if (!policeMaterial)
        {
            policeMaterial = resources.CreateTexturedMaterial("Police", { 1, 1, 1, 1 }, policeTexture.data(), DetailTextureSize, DetailTextureSize);
        }
        dogMaterial = resources.LoadMaterialFromFile(
            "Dog", { 1, 1, 1, 1 }, L"Textures/Obstacle_Dog.png");
        if (!dogMaterial)
        {
            dogMaterial = resources.CreateTexturedMaterial("Dog", { 1, 1, 1, 1 }, dogTexture.data(), DetailTextureSize, DetailTextureSize);
        }
        wallMaterial = resources.LoadMaterialFromFile(
            "Wall", { 1, 1, 1, 1 }, L"Textures/Obstacle_Wall.png");
        if (!wallMaterial)
        {
            wallMaterial = resources.CreateTexturedMaterial("Wall", { 1, 1, 1, 1 }, wallTexture.data(), DetailTextureSize, DetailTextureSize);
        }
        lowBarMaterial = resources.LoadMaterialFromFile(
            "LowBar", { 1, 1, 1, 1 }, L"Textures/Obstacle_UpperWall.png");
        if (!lowBarMaterial)
        {
            lowBarMaterial = resources.CreateTexturedMaterial("LowBar", { 1, 1, 1, 1 }, lowBarTexture.data(), DetailTextureSize, DetailTextureSize);
        }
        backgroundMaterials[0] = resources.LoadMaterialFromFile(
            "BackgroundPrisonInside", { 1, 1, 1, 1 }, L"Textures/Background_Prison_Day.png");
        if (!backgroundMaterials[0])
        {
            backgroundMaterials[0] = resources.CreateTexturedMaterial("BackgroundPrisonInside", { 1, 1, 1, 1 }, background0.data(), DetailTextureSize, DetailTextureSize);
        }
        backgroundMaterials[1] = resources.LoadMaterialFromFile(
            "BackgroundEscapeOutside", { 1, 1, 1, 1 }, L"Textures/Background_Prison_Night.png");
        if (!backgroundMaterials[1])
        {
            backgroundMaterials[1] = resources.CreateTexturedMaterial("BackgroundEscapeOutside", { 1, 1, 1, 1 }, background1.data(), DetailTextureSize, DetailTextureSize);
        }
        Mesh* quad = resources.GetMesh("Quad");

        GameObject& backgroundVisual = CreateObject("Background");
        backgroundVisual.GetTransform().position = { 0.0f, 0.0f };
        backgroundVisual.GetTransform().scale = { 2.45f, 2.05f };
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
        ground.GetTransform().position = { 0.0f, -0.675f };
        ground.GetTransform().scale = { 2.18f, 0.075f };
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

        GameObject& overlay = CreateObject("GameOverOverlay");
        overlay.GetTransform().position = { -2.0f, -2.0f };
        overlay.GetTransform().scale = { 2.6f, 2.2f };
        overlay.AddComponent<MeshRenderer>(quad, gameOverOverlayMaterial);
        gameOverOverlay = &overlay;

        GameObject& cage = CreateObject("JailCage");
        cage.GetTransform().position = { -2.0f, -2.0f };
        cage.GetTransform().scale = { 0.32f, 0.34f };
        cage.AddComponent<MeshRenderer>(quad, jailCageMaterial);
        jailCage = &cage;

        ResetGame();
    }

    void RunnerGameScene::Update(float dt)
    {
        if (gameState == GameState::GameOver && Input::WasKeyPressed('R'))
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
            graphics.QueueText(L"SCORE " + std::to_wstring(score), 24, 20, 30, { 1.0f, 0.0f, 0.0f, 1.0f });
        }
        else
        {
            graphics.QueueText(L"GAME OVER", 420, 90, 64, { 1.0f, 0.18f, 0.12f, 1.0f });
            graphics.QueueText(L"R RESTART", 455, 160, 34, { 1.0f, 1.0f, 1.0f, 1.0f });
        }
    }

    void RunnerGameScene::UpdatePlayer(float dt)
    {
        const bool jumpPressed = Input::WasKeyPressed(VK_SPACE) || Input::WasKeyPressed(VK_UP);
        const bool jumpReleased = Input::WasKeyReleased(VK_SPACE) || Input::WasKeyReleased(VK_UP);
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
                obstacle.visual->GetTransform().position = GetObstacleVisualPosition(obstacle);
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

        const Vec2 cageCenter = { player.position.x + 0.015f, player.position.y + 0.055f + jailDropOffsetY };
        if (jailCage)
        {
            jailCage->GetTransform().position = cageCenter;
            jailCage->GetTransform().scale = { 0.32f, 0.34f };
        }
        if (gameOverOverlay)
        {
            gameOverOverlay->GetTransform().position = { 0.0f, 0.0f };
            gameOverOverlay->GetTransform().scale = { 2.6f, 2.2f };
        }

        for (size_t i = 0; i < jailBars.size(); ++i)
        {
            jailBars[i]->GetTransform().position = { -2.0f, -2.0f };
        }
    }

    void RunnerGameScene::ResetGame()
    {
        player.position = { -0.55f, -0.55f };
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

        SpawnObstacleByType(spawnData.firstSpawnX, Obstacle::WALL);
        SpawnNextObstacle();
        SpawnNextObstacle();
        SpawnNextObstacle();

        UpdatePlayerVisual();
        UpdateBackground();

        for (GameObject* bar : jailBars)
        {
            bar->GetTransform().position = { -2.0f, -2.0f };
        }
        if (jailCage)
        {
            jailCage->GetTransform().position = { -2.0f, -2.0f };
        }
        if (gameOverOverlay)
        {
            gameOverOverlay->GetTransform().position = { -2.0f, -2.0f };
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

        obstacle.visual->GetTransform().position = GetObstacleVisualPosition(obstacle);
        obstacle.visual->GetTransform().scale = { GetObstacleVisualWidth(type), GetObstacleVisualHeight(type) };
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
        if (type == Obstacle::DOG)     return 0.16f;
        if (type == Obstacle::WALL)    return 0.12f;
        if (type == Obstacle::LOW_BAR) return 0.34f;
        return 0.10f;
    }

    float RunnerGameScene::GetObstacleHeight(Obstacle::Type type) const
    {
        if (type == Obstacle::DOG)     return 0.10f;
        if (type == Obstacle::WALL)    return 0.16f;
        if (type == Obstacle::LOW_BAR) return 0.055f;
        return 0.15f;
    }

    float RunnerGameScene::GetObstacleVisualWidth(Obstacle::Type type) const
    {
        if (type == Obstacle::DOG)     return 0.28f;
        if (type == Obstacle::WALL)    return 0.16f;
        if (type == Obstacle::LOW_BAR) return 0.42f;
        return 0.17f;
    }

    float RunnerGameScene::GetObstacleVisualHeight(Obstacle::Type type) const
    {
        if (type == Obstacle::DOG)     return 0.16f;
        if (type == Obstacle::WALL)    return 0.18f;
        if (type == Obstacle::LOW_BAR) return 0.14f;
        return 0.23f;
    }

    Vec2 RunnerGameScene::GetObstacleVisualPosition(const Obstacle& obstacle) const
    {
        return
        {
            obstacle.position.x,
            obstacle.position.y + (GetObstacleVisualHeight(obstacle.type) - obstacle.height) * 0.5f
        };
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
