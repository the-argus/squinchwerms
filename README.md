# squinchwerms

Game where you are a worm and you jump around and stuff

## Building

In this directory, run:

```bash
cmake -P vendor/package_manager.cmake
```

It should build the dependencies for your platform and install them into
`vendor/.packages/prefixes`. It will also create a file,
`vendor/.packages/cmake_prefix_path`, which is a list of entries that are needed
for the prefix path of the main build. The root CMakeLists.txt of this project
will look at that and automatically add entries in that file to its prefix path.

Now run:

```bash
cmake --preset dev
cmake --build build-dev --parallel
```

That will configure and build the project. To run the project, execute the program
`build-dev/squinchwerms` with the root of this repo as your working directory.
