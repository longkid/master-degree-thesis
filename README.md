The _InternshipSRC_ folder contains the source code of all tasks that I have done in my internship at the [LaBRI](https://www.labri.fr/en).
Besides that, some related concepts are also described in _internship-report.pdf_ file.  
My work is performed under instructions of Boris Mansencal (boris.mansencal@labri.fr) and Hugo Boujut (hugo.boujut@labri.fr).

## Work Explanation

- BBBox: Adapt this software for annotating HD videos.
- GaussianMap: Generate saliency maps with the help of Gaussian filtering.
- FaceDetection: Detect faces automatically by using Viola-Jones face detector of OpenCV library.
- Tracking: Test the CAMSHIFT object tracking algorithm for HD videos.
- Nov.Tasks (November Tasks): Examine statistics of faces in saliency map.

## Compilation & Execution

For most tasks, you can compile the source code by running these following commands:

```
cd <src_folder>
mkdir build
cd build
ccmake .. (Press c, then press g)
make
```

To execute, you run the generated file in build folder.

**Note**: For specific source code, you need to install some prerequisite tools or set up variable. Please read README file in each folder for more details.
