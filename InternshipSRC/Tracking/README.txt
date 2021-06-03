Prerequisite
============
- OpenCV library

Work Explanation
================
- camshiftdemo.cpp: This is an example program coming with OpenCV. This file is located in the folder OpenCV-2.3.1/samples/cpp
- camshiftdemo2.cpp: Test CAMSHIFT tracking on two images. They could be successive frames or non-successive frames in a video.
- camshiftdemo3.cpp: Test CAMSHIFT tracking on a movie.
- camshiftdemo4.cpp: Test CAMSHIFT tracking on a movie. Add a threshold to stop the false tracking.

Compilation & Execution
=======================
You must edit the CMakeLists.txt file to compile exactly each task (camshiftdemo2, camshiftdemo3, or camshiftdemo4).
Ex: To compile camshiftdemo2, you check two following lines to make sure that TARGET is camshiftdemo2 and SRCS includes camshiftdemo2.cpp.
SET(TARGET camshiftdemo2)
SET(SRCS camshiftdemo2.cpp BBoxFileReader.cpp AnnotatedFrame.cpp Methods.cpp)
