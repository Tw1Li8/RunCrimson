# Build Guide

## Environment

- Windows
- Visual Studio 2022
- C++17
- DirectX 11
- Windows SDK with `d3d11.lib`, `dxgi.lib`, and `d3dcompiler.lib`

## Solution File

Open:

```text
RunnerEngine.slnx
```

Project directory:

```text
C:\Users\yejun\Documents\Codex\2026-06-03\files-mentioned-by-the-user-26directx\outputs\RunnerEngine
```

## Build Configuration

Use:

```text
Debug | x64
```

## Visual Studio Steps

1. Open `RunnerEngine.slnx` in Visual Studio.
2. Select `Debug`.
3. Select `x64`.
4. Run `Build Solution`.
5. Start the project from Visual Studio.

## Working Directory

The project uses:

```xml
<LocalDebuggerWorkingDirectory>$(ProjectDir)</LocalDebuggerWorkingDirectory>
```

This matters because the shader is loaded with this relative path:

```text
Shaders/BasicColor.hlsl
```

Run the executable with the project directory as the working directory so the shader file can be found.

## Expected Result

The final `Debug|x64` build should complete with:

- 0 warnings
- 0 errors
- A Win32 game window
- DirectX rendered game scene

