# -*- coding: utf-8 -*-
"""
Created on Fri Sep 25 05:08:14 2015

@author: jyoung
"""

import matplotlib.pyplot as plt
import numpy as np
import SimpleITK as sitk
import sys
from os.path import expanduser, join


def sitk_show(img):
    """Displays SimpleITK image from its array. Includes a function to report 
    the pixel value under the mouse cursor."""
    X = sitk.GetArrayFromImage(img)
    
    fig, ax = plt.subplots()
    ax.imshow(X, cmap=plt.cm.Greys_r)
    
    if X.ndim == 2:
        numrows, numcols = X.shape
        def format_coord(x, y):
            col = int(x + 0.5)
            row = int(y + 0.5)
            if col>=0 and col<numcols and row>=0 and row<numrows:
                z = X[row, col]
                return 'x=%1.4f, y=%1.4f, z=%1.4f' %(x, y, z)
            else:
                return 'x=%1.4f, y=%1.4f' %(x, y)   
        ax.format_coord = format_coord

    plt.show()


def anisotropic_diffusion(img, *args):
    """INPUT: arguments are time step; conductance; # of iterations"""
    imgRecast = sitk.Cast(img, sitk.sitkFloat32)
    timeStep_ = args[0]
    condP = args[1]
    num_ = args[2]
    imgSmooth = sitk.CurvatureAnisotropicDiffusion(image1=imgRecast, 
                                                   timeStep=timeStep_, 
                                                   conductanceParameter=condP, 
                                                   numberOfIterations=num_)
    return imgSmooth


def gradient_magnitude(img, sigma_):
    return sitk.GradientMagnitudeRecursiveGaussian(image1=img, sigma=sigma_)


def sigmoid_filter(img, K1, K2):
    alpha_ = (K2 - K1)/6
    beta_ = (K1 + K2)/2
    imgSigmoid = sitk.Sigmoid(image1=img, alpha=alpha_, beta=beta_, 
                              outputMaximum=1.0, outputMinimum=0.0)
    return imgSigmoid


def fast_marching(img, seeds, stop):
    return sitk.FastMarching(image1=img, trialPoints=seeds, stoppingValue=stop)


def distance_mapping(img, seeds):
    seedImg = sitk.Image(img.GetSize()[0], img.GetSize()[1], sitk.sitkUInt8)
    seedImg.SetSpacing(img.GetSpacing())
    seedImg.SetOrigin(img.GetOrigin())
    seedImg.SetDirection(img.GetDirection())
    for s in seeds:
        seedImg[s] = 1
    
    distance = sitk.SignedMaurerDistanceMapImageFilter()
    distance.InsideIsPositiveOff()
    distance.UseImageSpacingOn()
    return distance.Execute(seedImg)


def input_level_set(imgSigmoid, imgDist, lowThresh, upThresh):
    """INPUT:
    1.) imgSigmoid <- feature image
    2.) imgDist <- image from distance mapping
    3.) lowThresh, upThresh <- limits for binary threshold"""
    initImg = sitk.BinaryThreshold(imgDist, lowThresh, upThresh)
    return sitk.Cast(initImg, imgSigmoid.GetPixelIDValue()) * -1 + 0.5


def shape_detection(imgInit, imgSigmoid, *args):
    """INPUT arguments are:
    1.) RMS change in level set func.
    2.) propagation scaling
    3.) curvature scaling
    4.) # of iterations"""
    shapeDetect = sitk.ShapeDetectionLevelSetImageFilter()
    shapeDetect.SetMaximumRMSError(args[0])
    shapeDetect.SetPropagationScaling(args[1])
    shapeDetect.SetCurvatureScaling(args[2])
    shapeDetect.SetNumberOfIterations(args[3])
    return shapeDetect.Execute(imgInit, imgSigmoid)


def geodesic_active_contour(imgInit, imgSigmoid, *args):
    """INPUT arguments are: 
    1.) propagation scaling
    2.) curvature scaling
    3.) advection scaling
    4.) RMS change in level set func.
    5.) # of iterations"""
    gac = sitk.GeodesicActiveContourLevelSetImageFilter()
    gac.SetPropagationScaling(args[0])
    gac.SetCurvatureScaling(args[1])
    gac.SetAdvectionScaling(args[2])
    gac.SetMaximumRMSError(args[3])
    gac.SetNumberOfIterations(args[4])
    return gac.Execute(imgInit, imgSigmoid)


def binary_threshold(img, lowerThreshold_, upperThreshold_):
    """Produce binary mask representing segmented object to be overlaid over
    original image."""
    binaryThresh = sitk.BinaryThresholdImageFilter()
    binaryThresh.SetLowerThreshold(lowerThreshold_)
    binaryThresh.SetUpperThreshold(upperThreshold_)
    binaryThresh.SetInsideValue(1)
    binaryThresh.SetOutsideValue(255)
    return binaryThresh.Execute(img)


def main():
    # read in DICOM images    
    sliceNum = int(sys.argv[1])
    dicomPath = join(expanduser('~'), 'Documents', 'SlicerDICOMDatabase', 
                     'TCIALocal', '0', 'images', '')
    reader = sitk.ImageSeriesReader()
    seriesIDread = reader.GetGDCMSeriesIDs(dicomPath)[1]
    dicomFilenames = reader.GetGDCMSeriesFileNames(dicomPath, seriesIDread)
    reader.SetFileNames(dicomFilenames)
    imgSeries = reader.Execute()
    imgSlice = imgSeries[:,:,sliceNum]
    imgSliceUInt8 = sitk.Cast(sitk.RescaleIntensity(imgSlice), sitk.sitkUInt8)
    print('\nDisplaying image slice...')
    sitk_show(imgSlice)

    # image filtering
    ans = -1
    while ans not in range(1, 4):   
        print('Enter number of desired filtering method:')
        print('1 - Curvature Anisotropic Diffusion')
        print('2 - Recursive Gaussian IIR')
        print('3 - Median')
        ans = input()

    if ans == 1:
        anisoParams = (0.06, 9.0, 5)
        imgFilter = anisotropic_diffusion(imgSlice, *anisoParams)
        print('\nDisplaying anisotropic diffusion-smoothed image...')
        sitk_show(imgFilter)
    elif ans == 2:
        recurGaussX = sitk.RecursiveGaussianImageFilter()
        recurGaussY = sitk.RecursiveGaussianImageFilter()
        recurGaussX.SetSigma(2.0)
        recurGaussY.SetSigma(2.0)
        recurGaussY.SetDirection(1)
        imgFilter = recurGaussY.Execute(recurGaussX.Execute(imgSlice))
        print('\nDisplaying recursive Gaussian-filtered image...')
        sitk_show(imgFilter)
    else:
        med = sitk.MedianImageFilter()
        med.SetRadius(3)
        imgFilter = med.Execute(imgSlice)
        print('\nDisplaying median-filtered image...')
        sitk_show(imgFilter)
    
    # compute edge potential with gradient magnitude recursive Gaussian
    sigma = 2.0
    imgGauss = gradient_magnitude(imgFilter, sigma)
    print('\nImage from gradient magnitude recursive Gaussian:')
    sitk_show(imgGauss)
    
    # sigmoid mapping to create feature image
    K1 = float(input('Enter value for K1: '))
    K2 = float(input('Enter value for K2: '))
    imgSigmoid = sigmoid_filter(imgGauss, K1, K2)
    print('\nDisplaying feature image...')
    sitk_show(imgSigmoid)
    
    # read in user-supplied seeds
    numSeeds = int(input('Enter the desired number of seeds: '))
    seeds = list()
    for i in range(numSeeds):
        coord = input('Enter x- and y-coordinates separated by a comma: ')
        seeds.append(tuple([int(n) for n in coord.split(',')]))
    
    # segmentation by fast marching    
    stopVal = int(input('Enter the stopping value: '))
    fastMarch = fast_marching(imgSigmoid, seeds, stopVal)
    print('\nDisplaying label image:')
    sitk_show(fastMarch)
    labelLowThresh = float(input('Enter lower threshold from label image: '))
    labelUpThresh = float(input('Enter upper threshold from label image: '))
    binaryThresh = binary_threshold(fastMarch, labelLowThresh, labelUpThresh)
    print('\nResult of fast marching segmentation:')
    sitk_show(sitk.LabelOverlay(imgSliceUInt8, binaryThresh, backgroundValue=255))
    
    # create image from distance mapping
    imgDist = distance_mapping(imgSlice, seeds)
    
    # get upper & lower limits within radii of seeds
    upThresh = -1e6
    X_dist = sitk.GetArrayFromImage(imgDist)
    for s in seeds:
        r = int(input('Enter the desired radius of the seed at %s: ' %(s,)))
        candUpThresh = float(np.amax(X_dist[s[1]-r:s[1]+r, s[0]-r:s[0]+r]))
        if candUpThresh > upThresh:
            upThresh = candUpThresh
    lowThresh = 0.0
    
    # get input level set    
    imgInit = input_level_set(imgSigmoid, imgDist, lowThresh, upThresh)
    print('\nDisplaying input level set...')
    sitk_show(imgInit)
    
    # shape detection segmentation
    shapeParams = (0.02, 1.0, 0.2, 500)
    imgShape = shape_detection(imgInit, imgSigmoid, *shapeParams)
    print('\nDisplaying label image:')
    sitk_show(imgShape)
    labelLowThresh = float(input('Enter lower threshold from label image: '))
    labelUpThresh = float(input('Enter upper threshold from label image: '))
    binaryThresh = binary_threshold(imgShape, labelLowThresh, labelUpThresh)
    print('\nDisplaying segmentation by shape detection...')
    sitk_show(sitk.LabelOverlay(imgSliceUInt8, binaryThresh, backgroundValue=255))
    
    # segmentation with geodesic active contours
    gacParams = (1.0, 0.2, 4.0, 0.01, 600)
    imgGac = geodesic_active_contour(imgInit, imgSigmoid, *gacParams)
    print('\nDisplaying label image:')
    sitk_show(imgGac)
    labelLowThresh = float(input('Enter lower threshold from label image: '))
    labelUpThresh = float(input('Enter upper threshold from label image: '))
    binaryThresh = binary_threshold(imgGac, labelLowThresh, labelUpThresh)
    print('\nDisplaying segmentation by geodesic active contours...')
    sitk_show(sitk.LabelOverlay(imgSliceUInt8, binaryThresh, backgroundValue=255))
    

if __name__=="__main__":
    main()
