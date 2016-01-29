# Vessel Filtering

**2015 December 31**

Using the Frangi filter implemented in MATLAB by Dirk-Jan Kroon on the VIBE_AXIAL_P series (ID ending in 7931) from patient TCGA-BC-4073. The parameters used are:

* BlackWhite = false
* &sigma; = 1...4 (step size = 1)
* &alpha; = 0.25, &beta; = 0.6, *c* = 20

Use the `DICOM2mat` function to read the series into a matrix and choose the ROI as follows:

    [img, slice2name] = DICOM2mat(dicomDir);
    ROI = img(33:335, 52:323, 10:52);
    ROIves = FrangiFilter3D(ROI, options);

where the options were set according to the parameters above. The result of the filtering can be visualized with `implay`. To prepare for 3D reconstruction, save the filtered images (using code from *DICOM2mat.m*):

    vesdicom = uint8(round(RescaleImg(ROIves)));
    [numRows numCols depth] = size(vesdicom);
    for k = 1:depth
        info = dicominfo(slice2name{k+9,2});
        dicomwrite(vesdicom(:,:,k), strcat(num2str(k+9), '.dcm'), info)
    end

**2016 January 2-4, 9, 14**

The ROI above includes many areas outside the liver which survived the Frangi filtering. This caused problems when using a connected threshold segmentation. Therefore, the liver regions will be masked using *livermap.mha*, the semi-manual segmentations done previously. Unfortunately, those segmentations did not include portions of the portal vein. These portions and other various "holes" in the image were filled in using the Editor module in 3D Slicer. Using the Mask Scalar Volume, also in 3D Slicer, the resulting mask was saved to *Masked Volume.nii*. Because reading medical imaging files in MATLAB proved difficult, the *.nii* file will be converted to a series of TIFF files using SimpleITK (3D Slicer apparently does not have DICOM export working):

    imgfile = 'Masked Volume.nii'
    imgvol = sitk.ReadImage(imgpath)
    imgRsc = sitk.Cast(sitk.RescaleIntensity(imgvol), sitk.sitkUInt8)
    os.chdir('Masked Series')
    z = imgRsc.GetSize()[2]
    sitk.WriteImage(imgRsc, ['slice{0:03d}.tiff'.format(i) for i in range(z)])

Read the image series in MATLAB:

    files = dir('*.tif*');
    masked = zeros([size(imread(files(1).name)) length(files)], 'uint8');
    for k = 1:length(files)
        masked(:,:,k) = imread(files(k).name);
    end
    ROI = masked(33:335, 52:323, 20:63);

Note that for the ROI here, lower values of the *z* dimension corresponds to more inferior. The parameters used for the Frangi filter are the same as above except &beta; is kept at the default value and *c* = 5. Save the filtered result to DICOM. This necessitates pulling DICOM headers from the original series (as above). Moreover, the slice numbers in the *slice2name* cell array need to be subtracted from the full series depth of 72 to correspond to the current ordering:

    ROIuint8 = uint8(round(RescaleImg(ROIves)));
    [numRows numCols depth] = size(ROIuint8);
    [img, slice2name] = DICOM2mat(dicomDir);
    for k = 1:depth
        info = dicominfo(slice2name{72-(k+19),2});
        dicomwrite(ROIuint8(:,:,k), strcat(num2str(k+19), '.dcm'), info)
    end

The intent for writing to DICOM was to use OsiriX's region growing method and interactive 3D editing to produce a 3D model. However, over-segmentation of the liver occurred as the connected threshold image filter included the boundaries of the liver in addition to the vessels. The Frangi filter with its current parameters seems to include the liver surface, which was very difficult to erase in OsiriX. Because of the thresholding, it also became difficult to trace the path of major vessels. Similar methods, such as the simple region growing in 3D Slicer, also have similar issues of grouping unwanted regions of the liver surface with the seed points in a vessel.

As a result, the method to resort to next is fast marching, which shows initial promise in testing on a portion of the portal vein. Modifications have been made to *imgscroll* to have a single class handle single image display and image overlays. A function in *imgscroll* now handles the plotting calls. Previous comparisons to `None` are replaced with checking the variable type and warnings about "converting a masked element to nan" have been temporarily resolved with explicitly setting `vmin` to 0.0 and `vmax` to the fast marching stopping value.

To capture seed points from mouse clicks, code from the `IndexMouseCapture` class in *Liver Segmentation 3D.ipynb* will be adapted to a new standalone importable script *mousecapture.py* that can display either a circle or arrow at the seed. For some currently unknown reason, *mousecapture.py* does not work properly in the Jupyter notebook but works in an IPython terminal. Code will be recorded below:

    frangipath = os.path.join('Liver Segmentation Data', 'TCGA-BC-4073', 'frangi_filtered')
    reader = sitk.ImageSeriesReader()
    franginames = reader.GetGDCMSeriesFileNames(frangipath)
    reader.SetFileNames(franginames)
    frangi = reader.Execute()
    I = sitk.GetArrayFromImage(frangi)
    I01 = I/255
    img01 = sitk.GetImageFromArray(I01)
    img01.CopyInformation(frangi)
    seeds = mousecapture.get_seeds(img01)
    fastmarch = sitk.FastMarchingImageFilter()
    fastmarch.SetTrialPoints(seeds)
    fastmarch.SetStoppingValue(10.0)
    imgfm = fastmarch.Execute(img01)
    imgscroll.show_imgs(img01, imgfm)

It is important to note that at this point, the fast marching stopping value is also hard-coded into the *imgscroll* script for overlaying the segmentation result.

**2016 January 14-16, 20**

Writing C++ code using ITK has begun in order to use the available Hessian-based filters. The source files *satofilter.cpp* and *frangifilter.cpp* have been created and CMake is used for compilation. Compilation instructions are gathered in a *README.md* file under the *ITKVessel/* directory. First steps have been taken to successfully read in a DICOM series, extract a region-of-interest, and write an output image *ROI.mha* on 16 January. The 1<sup>st</sup> working version of the Antiga (Frangi-based) filter was achieved on 20 January.

**2016 January 28-29**

The 1<sup>st</sup> working version of *frangifilter.cpp* included code to extract an ROI. It has been decided that this code will be moved to a stand-alone, separate file *extractROI.cpp*, which will be executed to write *ROI.mha* again. The directory location of *extractROI.cpp* is *Medical Imaging*, which also contains an accompanying CMake file *CMakeLists.txt*. The compilation proceeds as follows:

    cd Medical\ Imaging
    cmake -DITK_DIR=~/ITK/ITKbin .
    make

and the image coordinates to extract the ROI are:

    Enter x-coordinate (column) start: 52
    Enter x-coordinate (column) end: 323

    Enter y-coordinate (row) start: 33
    Enter y-coordinate (row) end: 335

    Enter z-coordinate (slice) start: 20
    Enter z-coordinate (slice) end: 62

First working version of *satofilter.cpp* achieved, setting &sigma; to either 1.0 or 2.0 and using the default parameter values of &alpha;<sub>1</sub> = 0.5 and &alpha;<sub>2</sub> = 2.0. At &sigma; = 1.0, there appear to be many artifacts and the vessels are not clearly shown or visible. At &sigma; = 2.0, the method appears to be a bit strict, where regions of vessels do not survive the filtering.

*frangifilter.cpp* has been modified by removing the code to extract a ROI. Both *satofilter.cpp* and *frangifilter.cpp* now read in a single image file instead of of a directory of images. *frangifilter.cpp* was re-run to write out *frangiresult.mha*. The parameters used were &alpha; = 0.25, &beta; = 0.6, &gamma; = 20.0, &sigma;<sub>min</sub> = 1.0, &sigma;<sub>max</sub> = 4.0, and number of &sigma; steps = 4. 
