# Gameplay

## Goal

The player escapes from prison by running forward and avoiding obstacles.

## Controls

- `Space`: Jump
- `Down`: Slide
- `R`: Restart
- `Esc`: Quit
- `P`: Test score increase

## Player States

- `Run`
- `Jump`
- `Slide`

Jump uses vertical velocity and gravity. Releasing `Space` while rising cuts the upward velocity, so short presses make lower jumps and longer presses make higher jumps.

Slide reduces the player collision height and allows the player to avoid the slide obstacle.

## Obstacles

- `POLICE`: one-hit game over
- `DOG`: one-hit game over
- `WALL`: one-hit game over
- `LOW_BAR`: avoid by sliding

All obstacle collisions use the same one-hit game over rule. There is no HP, stamina, or three-hit wall rule in the current implementation.

Ground obstacles use a shared floor alignment calculation so their rendered position and collision AABB match. `LOW_BAR` is positioned by player head height instead of floor height.

## Score

Score increases slowly through `scoreAccumulator`. Score stops when the game enters game over.

Score is used for:

- Background switching every 1000 points
- Difficulty calculation after the early adaptation section
- Pair obstacle spawning after 1500 points

## Difficulty

The first 500 points are an adaptation section. After that, difficulty increases from the score.

Difficulty affects:

- Obstacle scroll speed
- Obstacle spacing
- Chance to spawn a simple two-obstacle pattern after 1500 points

There is no difficulty UI. The player feels the difficulty through faster speed and denser obstacle placement.

## Backgrounds

Two background stages alternate every 1000 points:

- Prison interior
- Outdoor escape section

## Game Over

When the player collides with any obstacle, the game enters game over. Scrolling and score stop. A jail cage drops around the player, and the UI shows:

- `GAME OVER`
- `R RESTART`

`R` calls `ResetGame()` and restores player, score, speed, obstacles, background, and cage state.

