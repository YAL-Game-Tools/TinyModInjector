# TinyModInjector

A very tiny (8KB!) mod injector. Poses as `dinput8.dll`, loads DLLs from `NativeMods/`.

## How it works

- You place the DLL next to the game executable (or in a directory that it looks for DLLs in).
- The game loads the DLL instead of the real `dinput8.dll`.
- On default configuration, the DLL loads other DLLs from `NativeMods` folder as soon as it's loaded (on `DLL_PROCESS_ATTACH`).  
  On "delay" configuration, the DLL only does so when `DirectInput8Create` is called for the first time, which can be safer for some games (read: don't use it unless the default configuration causes the game to hard crash).
- The real `dinput8.dll` (in `System32` or `SysWOW64`) is loaded as necessary and calls are forwarded to it.

## Supported games

- GameMaker games (GMS1, GMS2)
- Unreal Engine games (3, 4?)
- Various random other games that load `dinput8.dll` (read: support DirectInput gamepads) - so long as you can figure out which directory(ies) the game is looking for DLLs in.

## Versions

- `TinyModLoader-x86`: 32-bit, loads mods immediately.
- `TinyModLoader-x86-delay`: 32-bit, loads mods on first `DirectInput8Create` call.
- `TinyModLoader-x64`: 64-bit, loads mods immediately.
- `TinyModLoader-x64-delay`: 64-bit, loads mods on first `DirectInput8Create` call.

## Debugging

If you need help figuring out whether the DLL and/or mods are getting loaded or not, you can add a `-tmi-debug` option to the game's command-line flags or create an empty text file called "tmi-debug" (so, `tmi-debug.txt`) in the game directory ("current working directory"). This will display message boxes on important events (DLL loaded, loading the real `dinput8.dll`, mods loaded).

## Caveats

- No API, just DLL loading - you'll want to make a DLL that implements an API for a game and then loads other mod DLLs _from a different directory_.

## Compiling

Run `build.bat` - this invokes `msbuild` with different configurations and packages them using 7-zip (`7z`).

## About

Made by YellowAfterlife

Inspired by an undocumented (yet most certainly alike) injector used in [this Dishonored mod](https://github.com/adm244/Dishonored-MissionStats/).

[MIT license](https://opensource.org/licenses/mit-license.php); if you want to bundle the DLL with your mod as a drop-in injector option, you can do so - credits are listed in DLL's "version info".