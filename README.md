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

For debain-based distros take a look on the Dockerfile. It has some clues what to install.

Next use cmake as easy as following assuming you are in the projects directory:

```bash
mkdir -p build && cd build && cmake .. && cmake --build .
```

If it still doesn't build, try to build with docker

```bash
docker build . -t mp4crawler
```

## Starting up

Check `./tools` directory for a demo app.  Try `--help` too. By default it won't try to get data from HTTP. Instead it's possible to start it with `mp4crawler -u https://demo.castlabs.com/tmp/text0.mp4`. Or it's possible to download `text0.mp4` into the current directory and just start the app (or point to the file with `-u /path/to/text0.mp4`).

Or with docker

```bash
docker run --rm -it mp4crawler:latest -u https://demo.castlabs.com/tmp/text0.mp4
```

## Design and usage

So we have boxes and one of is root box. A box can emit other boxes or byte arrays depending on its type. There is also a controlling object, an entry point for all the operations. Except providing boxes the controlling object will also report any transport issues.

From a library user perspective everything looks like following

1. Create Unboxer object of some specific flavor
2. Attach stream signals, like stream open.
3. Get root box on stream open and attach to its signals like opening sub-boxes and/or reading  payload.
4. For every open sub-box it's possible to change how it's treated (as an another box container or a binary blob)
