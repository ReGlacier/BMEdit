<p align="center">
	<img width="400" height="400" src=".github/logo.png">
</p>

BMEdit
------

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Build](https://github.com/ReGlacier/BMEdit/actions/workflows/build.yml/badge.svg)](https://github.com/ReGlacier/BMEdit/actions/workflows/build.yml)

The Blood Money Editor (aka BMEdit) is a tool to create & edit levels in Hitman Blood Money game.

Dependencies
------------

 * [Qt6](https://qt.io)
 * [ZBinaryIO](https://github.com/pawREP/ZBinaryIO)
 * [Nlohmann JSON](https://github.com/nlohmann/json)
 * [zlib](https://github.com/madler/zlib)
 * [conan](https://conan.io) (see "Build" for details)
 * [Kenney Prototype Textures](https://www.kenney.nl/assets/prototype-textures)

Build
-----

First of all you need to install [conan](https://conan.io) dependencies manager on your system.

**Note** Currently supported only Conan 2 (2.0.9 in my env)

Download (or git clone) this repository and do
```
conan profile detect --force
conan install . --output-folder=cmake-build-debug --build=missing -s build_type=Debug
```

(replace `cmake-build-debug` to your <build_folder>; replace Debug to Release for release build)

Then reload cmake project in your IDE or 
```
cd <build_folder>
cmake --build .
```

Contact Information
-------------------

The team can be contacted in the Project's issue tracker or via Discord (any questions, ideas, etc).

<a href="https://discord.gg/V5grGRw">
	<img src="https://img.shields.io/badge/discord-join-7289DA.svg?logo=discord&longCache=true&style=flat" />
</a>