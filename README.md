# Mp2tClip
The project builds a MPEG-2 TS Clipper __Mp2tClip__ application that decompose (clips) a MPEG-2 TS file into multiple files.

## To Build 
The MPEG-2 TS Clipper __Mp2tClip__ depends on multiple projects.  To build this application, do the following steps:

1. Clone `mp2tp` library, https://github.com/jimcavoy/mp2tp.git.  Build and install the library.
2. Clone `h264p` library, https://github.com/jimcavoy/h264p.git.  Build and install the library.
3. To configure, build and install __Mp2tClip__ application, change directory (cd) into the Mp2tClip folder and do the following:

a. Generate build environment

    cmake -S . -B ./build
    
b. Build application

    cmake --build ./build

c. Install application

    cmake --install ./build

The application is can be build and run on both Windows and Linux platforms.

### Test
After you build the application, you can run a test case by running the following:

    ctest --test-dir ./build

## Usage
 Usage: Mp2tClip \<OPTIONS\>

Options:

  __-s__   Source MPEG-2 TS file; otherwise, stdin.
  
  __-d__    Clip duration in seconds. (default: 60 seconds)
  
  __-o__    Output directory where the application writes the clipped files. (default: clipped)
  
  __-O__    Offset time when to start clipping in seconds. (default: 0 seconds)
  
  __-n__    The base name for the output clip file; otherwise, use the source file name.
          If stdin, use default name.  (default: clip)
          
  __-b__    Break when confidentiality label changes.

Help Options:
  
  __-?__    Print this message.

### Example
Clip a MPEG-2 TS file, `~/Videos/SomeVideo.ts`, into two minute (120 seconds) clips and place them in `~/tmp/myclippedfolder`

    Mp2tClip -s ~/Videos/SomeVideo.ts -d 120 -o ~/tmp/myclippedfolder


  
