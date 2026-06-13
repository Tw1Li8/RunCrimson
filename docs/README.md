# RunnerEngine

RunnerEngine is a small C++ and DirectX 11 learning project built as a minimal custom game engine prototype. It is not a full commercial engine architecture. The goal is to show the core structure of a simple Win32/DirectX game loop and a playable runner game in a form that is easy to build and inspect in Visual Studio.

## Project Purpose

This project demonstrates:

- Win32 window creation
- DirectX 11 rendering setup
- Scene based update and render flow
- Minimal `GameObject`, `Component`, and `Transform` structure
- Resource creation through `ResourceManager`
- HLSL shader based rendering
- A playable prison escape runner prototype

## Main Game Scene

The actual game logic is concentrated in `RunnerGameScene`.

`RunnerGameScene` manages:

- Player run, jump, and slide states
- Gravity based jump movement
- Variable jump height
- Obstacle movement and spawning
- One-hit game over collision
- Forgiving player AABB collision scaled to 85 percent
- Score accumulation
- Score based difficulty increase
- Two alternating background stages every 1000 points
- Game over jail cage animation
- Restart with `ResetGame()`

## Gameplay Summary

The player escapes while avoiding obstacles. The game starts gently, then becomes more tense as score increases. The score still rises slowly, but obstacle speed and density increase separately through the difficulty logic.

The game keeps the UI minimal:

- `SCORE` while playing
- `GAME OVER` and `R RESTART` after game over

No separate difficulty UI or debug collision lines are rendered in the final game screen.

## Project Scope

This is a compact educational prototype. It intentionally avoids complex engine systems such as object pooling, asset pipelines, editor tools, advanced physics, or a separate difficulty manager.

