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

**2016 January 2-4**

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

Note that for the ROI here, lower values of the *z* dimension corresponds more inferior. The parameters used for the Frangi filter are the same as above except &beta; is kept at the default value and *c* = 5. Save the filtered result to DICOM. This necessitates pulling DICOM headers from the original series (as above). Moreover, the slice numbers in the *slice2name* cell array need to be subtracted from the full series depth of 72 to correspond to the current ordering:

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

    import matplotlib.pyplot as plt
    import os
    import SimpleITK as sitk
    import imgscroll
    import mousecapture
    dicompath = os.path.join('Liver Segmentation Data', 'TCGA-BC-4073', 'frangi_filtered')
    reader = sitk.ImageSeriesReader()
    filenames = reader.GetGDCMSeriesFileNames(dicompath)
    reader.SetFileNames(filenames)
    imgSeries = reader.Execute()
    I = sitk.GetArrayFromImage(imgSeries)
    I01 = I/255
    img01 = sitk.GetImageFromArray(I01)
    img01.CopyInformation(imgSeries)
    seeds = mousecapture.get_seeds(img01)
    fastmarch = sitk.FastMarchingImageFilter()
    fastmarch.SetTrialPoints(seeds)
    fastmarch.SetStoppingValue(10.0)
    imgfm = fastmarch.Execute(img01)
    imgscroll.show_imgs(img01, imgfm)
