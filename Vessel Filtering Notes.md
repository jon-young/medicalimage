# Vessel Filtering in MATLAB

**2015 December 31**

Using the Frangi filter written by Dirk-Jan Kroon on the VIBE_AXIAL_P series (ID ending in 7931) from patient TCGA-BC-4073. The parameters used are:

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
