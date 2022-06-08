# BallanceSpeedrunShortcuts

Useful keyboard shortcuts for Ballance Speedrunning.

## How to use

### Installation

Make sure you have Ballance and [Ballance Mod Loader](https://github.com/Gamepiaynmo/BallanceModLoader) installed on your computer. Then download `SpeedrunShortcuts.bmod` from the Release page and put it into `%BallanceDirectory%\ModLoader\Mods\`.

### Shortcuts

- `Alt` + `C`: switch *Cheat Mode* back and forth, just like `/cheat`.
- `Alt` + `S`: set your current position as the respawn point, just like `/spawn`.
- `Alt` + `E`: restart the current level.

## FAQs

### Why not more shortcuts? How about adding the equivalent shortcut of "/sector X"?

The `/sector` command in Ballance Mod Loader uses its own private API and thus cannot be accomplished easily, and it's the same for many other functions of built-in mods. Executing arbitrary commands is also impossible since that's also a private API inaccessible for third-party mods. Yes, **BML totally fails to deliver the promise here despite [its modding help page](https://github.com/Gamepiaynmo/BallanceModLoader/wiki/Modding) asking people to learn from its own built-in mods**.

### Your coding style is so weird and inconsistent!

Code of this project was mainly copied from [Ballance Mod Loader](https://github.com/Gamepiaynmo/BallanceModLoader) and [BallanceMMO](https://github.com/Swung0x48/BallanceMMO) and their code have already contained many different and conflicting styles. For such a small project (look at its code, it's literally only about matching shortcut keys) I don't think it's worth it to take the time tidying it up.

## Build the project

1. Install Visual Studio and open the project.

2. Download and extract BML Dev pack, and place the following directories:

```
include\BML -> %RepoDirectory%\include\BML
lib\Debug -> %RepoDirectory%\lib\BML\Debug
lib\Release -> %RepoDirectory%\lib\BML\Release
```

3. Just build it.