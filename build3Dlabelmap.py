# -*- coding: utf-8 -*-
"""
Assembles 2D segmentations into a 3D label map.

Created on Sat Dec 12 14:03:23 2015

@author: jyoung
"""

import glob
import numpy as np
import os.path
import re
import SimpleITK as sitk


def read_dicom():
    """Read in DICOM series"""
    dicomPath = os.path.join(os.path.expanduser('~'), 'Documents', 
            'SlicerDICOMDatabase', 'TCIALocal', '0', 'images', '')
    reader = sitk.ImageSeriesReader()
    seriesIDread = reader.GetGDCMSeriesIDs(dicomPath)[1]
    dicomFilenames = reader.GetGDCMSeriesFileNames(dicomPath, seriesIDread)
    reader.SetFileNames(dicomFilenames)
    return reader.Execute()


def main():
    imgSeries = read_dicom()
    nX, nY, nZ = imgSeries.GetSize()
    segMat = np.zeros((nZ, nY, nX), dtype=np.int16)
    
    readDir = os.path.join('Liver Segmentation Data', 'TCGA-BC-4073', '')
    for p in glob.glob(readDir + '*.npy'):
        f = os.path.basename(p)
        match = re.search(r'\d+', f)
        sliceNum = int(match.group())
        segMat[sliceNum,:,:] = np.load(p)
    
    labelmap = sitk.GetImageFromArray(segMat)
    labelmap.CopyInformation(imgSeries)
    
    writer = sitk.ImageFileWriter()
    writer.SetFileName(readDir + 'labelmap.mha')
    writer.Execute(labelmap)


if __name__=="__main__":
    main()
