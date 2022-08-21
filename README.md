<p align="center">
	<img width="400" height="400" src=".github/logo.png">
</p>

BMEdit
------

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

The Blood Money Editor (aka BMEdit) is a tool to create & edit levels in Hitman Blood Money game.

Dependencies
------------

 * [Qt6](https://qt.io)
 * [ZBinaryIO](https://github.com/pawREP/ZBinaryIO)
 * [Nlohmann JSON](https://github.com/nlohmann/json)
 * [zlib](https://github.com/madler/zlib)
 * [conan](https://conan.io) (see "Build" for details)

Build
-----

First of all you need to install [conan](https://conan.io) dependencies manager on your system.

Then download (or git clone) this repository and do
```
conan install . -s build_type=Debug --install-folder=cmake-build-debug
```
or
```
conan install . -s build_type=Release --install-folder=cmake-build-release
```

Reload cmake project in your IDE or 
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