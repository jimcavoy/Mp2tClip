# Mp2tClip
The project builds a MPEG-2 TS Clipper __Mp2tClip__ application that decompose (clips) a MPEG-2 TS file into multiple files.

## To Build 
The MPEG-2 TS Clipper __Mp2tClip__ depends on multiple projects.  To build this application, do the following steps:

1. Clone `mp2tp` library, https://github.com/jimcavoy/mp2tp.git.  Build and install the library.
2. Clone `h264p` library, https://github.com/jimcavoy/h264p.git.  Build and install the library.
3. To configure, build and install __Mp2tClip__ application, change directory (cd) into the Mp2tClip folder and do the following:

a. Generate build environment
```
cmake -S . -B ./build
```
b. Build application
```
cmake --build ./build
```
c. Install application
```
cmake --install ./build
```
The application is can be build and run on both Windows and Linux platforms.

### Test
After you build the application, you can run a test case by running the following:
```
ctest --test-dir ./build
```

## Usage
```
Usage: Mp2tClip <OPTIONS>

Options:
  -s    Source MPEG-2 TS file; otherwise, stdin.
  -d    Clip duration in seconds. (default: 60 seconds)
  -o    Output directory where the application writes the clipped files. (default: clipped)
  -O    Offset time when to start clipping in seconds. (default: 0 seconds)
  -n    The base name for the output clip file; otherwise, use the source file name.
          If stdin, use default name.  (default: clip)
  -b    Break when confidentiality label changes.
  -?    Print this message.
```

## Examples

### 1. Clipping a File
Clip a MPEG-2 TS file, `~/Videos/SomeVideo.ts`, into two minute (120 seconds) clips and place them in `~/tmp/myclippedfolder`.

```
Mp2tClip -s ~/Videos/SomeVideo.ts -d 120 -o ~/tmp/myclippedfolder
```

### 2. Clipping a Stream
Pipe a stream into __Mp2pClip__ to create two minute clips and place them in `~/tmp/myclippedfolder`.

```
StreamingApp | Mp2tClip -d 120 -o ~/tmp/myclippedfolder
```
