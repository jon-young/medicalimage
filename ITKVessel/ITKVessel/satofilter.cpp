//
//  satofilter.cpp
//  ITKVessel
//
//  Filter a region-of-interest (ROI) for blood vessels as detailed in 
//  Sato et al. (1997)
//
//  Created by Jonathan Young on 1/14/16.
//

#include <iostream>
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkHessianRecursiveGaussianImageFilter.h"
#include "itkHessian3DToVesselnessMeasureImageFilter.h"


int main(int argc, const char * argv[])
{
    // Validate input parameters
    if (argc < 3) {
        std::cerr << "Usage: " 
			<< argv[0]
            << " <InputImage> <OutputImage> [sigma] [alpha1] [alpha2]"
            << std::endl;
        return EXIT_FAILURE;
    }
	const char * inputImage = argv[1];
	const char * outputImage = argv[2];
	const char * sigma = NULL;
	if( argc > 3 ) {
		sigma = argv[3];
	}
	const char * alpha1 = NULL;
	if( argc > 4 ) {
		alpha1 = argv[4];
	}
	const char * alpha2 = NULL;
	if( argc > 5 ) {
		alpha2 = argv[5];
	}
    
    const unsigned int Dimension = 3;
    typedef unsigned short InputPixelType;
	typedef float OutputPixelType;
    typedef itk::Image< InputPixelType, Dimension > InputImageType;
    typedef itk::Image< OutputPixelType, Dimension > OutputImageType;
    
    ////////////////////////////////////////////////
    // 1) Read the input series
    
	typedef itk::ImageFileReader< InputImageType >  ReaderType;
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName( inputImage );

    try {
        reader->Update();
    } catch (itk::ExceptionObject &excp) {
        std::cerr << "Exception thrown while reading the series" << std::endl;
        std::cerr << excp << std::endl;
        return EXIT_FAILURE;
    }
	
    ////////////////////////////////////////////////
    // 2) Sato filter
	
	typedef itk::HessianRecursiveGaussianImageFilter< InputImageType > 
		HessianFilterType;
	HessianFilterType::Pointer hessianFilter = HessianFilterType::New();
	hessianFilter->SetInput( reader->GetOutput() );
	if( sigma ) {
		hessianFilter->SetSigma( atof( sigma ) );
	}
	
	typedef itk::Hessian3DToVesselnessMeasureImageFilter< OutputPixelType > 
		VesselnessMeasureFilterType;
	VesselnessMeasureFilterType::Pointer vesselnessFilter = 
		VesselnessMeasureFilterType::New();
	vesselnessFilter->SetInput( hessianFilter->GetOutput() );
	if( alpha1 ) {
		vesselnessFilter->SetAlpha1( atof( alpha1 ) );
	}
	if( alpha2 ) {
		vesselnessFilter->SetAlpha2( atof( alpha2 ) );
	}
	
	////////////////////////////////////////////////
    // 3) Write output image
    
	typedef itk::ImageFileWriter< OutputImageType > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetInput( vesselnessFilter->GetOutput() );
    writer->SetFileName( outputImage );
    
    try {
        writer->Update();
    } catch (itk::ExceptionObject & error) {
        std::cerr << "Error: " << error << std::endl;
        return EXIT_FAILURE;
    }
}
