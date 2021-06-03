Prerequisite
============
- OpenCV library

Work Explanation
================
- task1.cpp: Compute values from annotating results and eye-tracker maps (Use method COB, COP, and FOB)
- task2.cpp: Compute values from annotating results and error maps (Use method COB and COP)
- task3.cpp: Compute values from the combination of both two maps (Use method COB and COP)

After the mean values are saved in text files, I use Gnuplot tool for plotting on the graph.

Compilation & Execution
=======================
You must edit the CMakeLists.txt file to compile exactly each task (task1, task2, or task3).
Ex: To compile task1, you check two following lines to make sure that TARGET is task1 and SRCS includes task1.cpp.
SET(TARGET task1)
SET(SRCS task1.cpp BBoxFileReader.cpp AnnotatedFrame.cpp Methods.cpp)
