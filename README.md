# MainSketch
This is an OpenFrameworks .9 and Kinect 2.0 SDK based project, both need to be installed and OpenFramworks needs to be in the Visual Studio 2015 solution.  The goal of this project is a thin layer around computer graphics, video and user input devices.  The thin-ness is unlike the game platforms for Windows like Unity which are big and its not always clear what is going on with them.

https://github.com/elliotwoods and https://github.com/UnaNancyOwen provide Kinect 2 projects and make good references as an FYI.  Much better than the MS SDK samples I found.

I also found  its best to make a simple and direct solution vs. trying to tie other Kinect projects together as the Kinect is touchy and any abstraction seems to cause  subtle issues that take a bit of work to track down.

The Kinect face code worked better than any code I could run form OpenCV and a HD camera, and its much (much) less code to deal with.  I Kinect Gestures  seemed to be time consuming and a bit of a concern as the tools have been in Prevew since Summer 2014 so I did not put them in this project. I also did not use the camera but only because it was not needed by this app.

NuiDatabase is required for face tracking, https://github.com/Kinect/Docs/blob/master/Kinect4Windows2.0/k4w2/Programming_Guide/Face_tracking.md.
