Super Mario Strikers  
[![Build Status]][actions] [![Code Progress]][progress] [![Link Progress]][progress] [![Discord Badge]][discord]
=============

[Build Status]: https://github.com/yannicksuter/smstrikers-decomp/actions/workflows/build.yml/badge.svg
[actions]: https://github.com/yannicksuter/smstrikers-decomp/actions/workflows/build.yml

[Code Progress]: https://decomp.dev/yannicksuter/smstrikers-decomp.svg?mode=shield&measure=code&label=Code
[Link Progress]: https://decomp.dev/yannicksuter/smstrikers-decomp.svg?mode=shield&measure=complete_code_percent&label=Linked
[Fuzzy Progress]: https://decomp.dev/yannicksuter/smstrikers-decomp.svg?mode=shield&measure=fuzzy_match_percent&label=Fuzzy
[progress]: https://decomp.dev/yannicksuter/smstrikers-decomp

[Discord Badge]: https://img.shields.io/discord/727908905392275526?color=%237289DA&logo=discord&logoColor=%23FFFFFF
[discord]: https://discord.gg/hKx3FJJgrV

> [!IMPORTANT]
> This repository does **not** provide a new way to play *Super Mario Strikers*. It is not a modern recompilation ("recomp") or a port. Its 100% code and linking status refers to a high-fidelity reconstruction of what the original source code may have looked like—one that compiles into an executable byte-for-byte identical to the original retail GameCube release.
>
> Recompilations and ports may emerge elsewhere from this work—and we look forward to seeing them—but they are outside the scope of this repository. Please do not ask for recompilations or ports in the decompilation Discord, as producing and supporting them is not the focus of this community.

A work-in-progress decompilation of Super Mario Strikers for GameCube.

This repository does **not** contain any game assets or assembly whatsoever. An existing copy of the game is required.

Supported versions, in release order:

| Version | Region and title | Release date | Revision | `main.dol` SHA-1 |
| --- | --- | --- | --- | --- |
| `G4QP01` | Europe — *Mario Smash Football* | November 18, 2005 | Rev 0 | `6dc83dc91d0a5887f0056623498d4cbcd88bc463` |
| `G4QE01` | North America — *Super Mario Strikers* | December 5, 2005 | Rev 0 | `376d699c99b6b0949abe1b4ceccefdef7828d2b5` |
| `G4QJ01` | Japan — *Super Mario Strikers* | January 19, 2006 | Rev 0 | `d116f02b778a4f69725fd1c00656012d16ebf94a` |

The releases share nearly all game code; see [Version differences](docs/version_differences.md) for a short summary of the regional changes.

Decompilation
=============

Decompilation is the process of reverse-engineering compiled machine code back into human-readable source code. Unlike disassembly, which produces assembly language, decompilation aims to reconstruct high-level code (like C or C++) that closely matches what the original developers wrote. This process involves analyzing the binary executable, understanding its structure and behavior, and translating it back into source code that compiles to produce identical machine code. In this project, the goal is not just a close match, but a **100% match**—the decompiled source code must compile to produce byte-for-byte identical machine code to the original. This is why diffing (see the [Diffing](#diffing) section below) is an essential piece of the process, as it allows us to verify that our decompiled code produces exactly the same binary output as the original game. Decompilation projects like this one enable deeper understanding of game mechanics, facilitate modding and preservation, and serve as valuable learning resources for understanding how games were built.

For interesting discoveries and insights found during the decompilation process, check out [Fun Facts](FunFacts.md).

Progress
========

![progress overview](https://decomp.dev/projects/989774797.svg?mode=overview&version=G4QE01)

Track the project decompilation progress and explore the interactive graph on [decomp.dev](https://decomp.dev/yannicksuter/smstrikers-decomp).


Contributing
============

Everybody is warmly welcome to contribute! Whether you're experienced with decompilation or just getting started, your contributions are valuable.

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for the full guidelines — it covers where the project stands, how to set up your environment, pull request expectations, code style, tips for working on matches, header reorganisation, and the project's stance on AI-assisted contributions.

For real-time discussion, coordination, and help, join the [Discord server](https://discord.gg/hKx3FJJgrV).

Dependencies
============

Windows
--------

On Windows, it's **highly recommended** to use native tooling. WSL or msys2 are **not** required.  
When running under WSL, [objdiff](#diffing) is unable to get filesystem notifications for automatic rebuilds.

- Install [Python](https://www.python.org/downloads/) and add it to `%PATH%`.
  - Also available from the [Windows Store](https://apps.microsoft.com/store/detail/python-311/9NRWMJP3717K).
- Download [ninja](https://github.com/ninja-build/ninja/releases) and add it to `%PATH%`.
  - Quick install via pip: `pip install ninja`

macOS
------

- Install [ninja](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages):

  ```sh
  brew install ninja
  ```

- Install [wine-crossover](https://github.com/Gcenx/homebrew-wine):

  ```sh
  brew install --cask --no-quarantine gcenx/wine/wine-crossover
  ```

After OS upgrades, if macOS complains about `Wine Crossover.app` being unverified, you can unquarantine it using:

```sh
sudo xattr -rd com.apple.quarantine '/Applications/Wine Crossover.app'
```

Linux
------

- Install [ninja](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages).
- For non-x86(_64) platforms: Install wine from your package manager.
  - For x86(_64), [wibo](https://github.com/decompals/wibo), a minimal 32-bit Windows binary wrapper, will be automatically downloaded and used.

Building
========

- Clone the repository (including the `musyx` submodule):

  ```sh
  git clone --recursive https://github.com/yannicksuter/smstrikers-decomp
  ```

  If you've already cloned the repository without --recursive, initialize the submodule manually:

  ```sh
  git submodule update --init --recursive
  ```

- To update the repository and its submodules in one go:

  ```sh
  git pull --recurse-submodules
  ```

- Copy your game's disc image to `orig/<version>` (for example, `orig/G4QP01`,
  `orig/G4QE01`, or `orig/G4QJ01`).
  - Supported formats: ISO (GCM), RVZ, WIA, WBFS, CISO, NFS, GCZ, TGC
  - After the initial build, the disc image can be deleted to save space.

- Configure:

  ```sh
  python configure.py
  ```

  To use a version other than `G4QE01` (USA, Rev 0), specify it with `--version`.

- Build:

  ```sh
  ninja
  ```

### Building a specific version

`G4QE01` (USA, Rev 0) is the default. To select another version, pass its game
ID to `configure.py`:

```sh
# Europe
python configure.py --version G4QP01
ninja

# USA (default)
python configure.py
ninja

# Japan
python configure.py --version G4QJ01
ninja
```

Build outputs are kept separately in `build/G4QP01`, `build/G4QE01`, and
`build/G4QJ01`. To switch versions, rerun `configure.py` with the other game
ID; cleaning the build directory is not necessary. Add `--map` to the configure
command to generate a linker map.

All three versions are configured to build entirely from reconstructed source
and are checked against their retail `main.dol` SHA-1. GitHub Actions and
decomp.dev progress reporting currently default to the USA version.

Diffing
=======

Once the initial build succeeds, an `objdiff.json` should exist in the project root.

Download the latest release from [encounter/objdiff](https://github.com/encounter/objdiff). Under project settings, set `Project directory`. The configuration should be loaded automatically.

Select an object from the left sidebar to begin diffing. Changes to the project will rebuild automatically: changes to source files, headers, `configure.py`, `splits.txt` or `symbols.txt`.

Related project
===============

[mscharged-decomp](https://github.com/yannicksuter/mscharged-decomp) is the
work-in-progress matching decompilation of *Mario Strikers Charged*, the
Nintendo Wii successor to *Super Mario Strikers*.

Acknowledgements
================

This project wouldn’t be possible without the collective knowledge, tools, and support of the broader decompilation community. Huge thanks to contributors of other GameCube decomp projects, the teams behind [decomp.dev](https://decomp.dev/) and [decomp.me](https://decomp.me/), and the incredibly helpful discussions happening on Discord. These resources have been invaluable for solving problems, speeding up setup, and staying motivated throughout the process.
