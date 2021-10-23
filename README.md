# MP4 crawler library and application

The goal of the project is parse MP4 files and extract internal data. MP4 files are comprised of "boxes" so it's where the project name comes from.

This is basically a test task for an employer, not a real project. If you are reading this text then most likely you're one of dev team members dedicated to review new candidates and me particularly.

The project is written with help of Qt framework. Qt has everything necessary for such kind of task and to visualize what's going on and also has perfect documentation. That will fit well the requirements and ensure fast development.

There are 3 sub-projects inside:

- src - crawler library
- tools - demo application and visualization tool
- test - unit tests for the library

## Building

First ensure you have next packages installed

- qt core dev
- qt gui dev
- qt widgets dev
- cmake

Next use cmake as easy as following assuming you are in the projects directory:

```bash
mkdir -p build && cd build && cmake .. && cmake --build .
```

## Starting up

One can you the built by default application in the `./tools` directory.
