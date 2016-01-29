//
//  frangifilter.cpp
//  ITKVessel
//
//  Created by Jonathan Young on 1/15/16.
//  Copyright © 2016 Jonathan Young. All rights reserved.
//

#include <iostream>
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkHessianToObjectnessMeasureImageFilter.h"
#include "itkMultiScaleHessianBasedMeasureImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"


int main(int argc, const char * argv[])
{
    // Validate input parameters
    if (argc < 2) {
        std::cerr << "Usage: "
        << argv[0]
        << " <InputDir> <OutputImage>"
        << std::endl;
        return EXIT_FAILURE;
    }
    
    const unsigned int Dimension = 3;
    typedef unsigned short PixelType;
    typedef itk::Image< PixelType, Dimension > ImageType;
    const char * outputImage = argv[2];
    
    ////////////////////////////////////////////////
    // 1) Read the input series
    
	typedef itk::ImageFileReader< ImageType >  ReaderType;
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName( argv[1] );
	
    try {
        reader->Update();
    } catch (itk::ExceptionObject &excp) {
        std::cerr << "Exception thrown while reading the series" << std::endl;
        std::cerr << excp << std::endl;
        return EXIT_FAILURE;
    }
    
    ////////////////////////////////////////////////
    // 2) Antiga (Frangi-based) filter
	
	typedef itk::SymmetricSecondRankTensor< double, Dimension > HessianPixelType;
	typedef itk::Image< HessianPixelType, Dimension > HessianImageType;
	typedef itk::HessianToObjectnessMeasureImageFilter<HessianImageType, 
	ImageType> ObjectnessFilterType;
	ObjectnessFilterType::Pointer objectnessFilter = ObjectnessFilterType::New();
	objectnessFilter->SetBrightObject( true );
	objectnessFilter->SetScaleObjectnessMeasure( false );
	objectnessFilter->SetAlpha( 0.25 );
	objectnessFilter->SetBeta( 0.6 );
	objectnessFilter->SetGamma( 10.0 );
	objectnessFilter->SetScaleObjectnessMeasure(1);
	
	typedef itk::MultiScaleHessianBasedMeasureImageFilter<ImageType, 
	HessianImageType, ImageType> MultiScaleEnhancementFilterType;
	MultiScaleEnhancementFilterType::Pointer multiScaleEnhancementFilter = 
		MultiScaleEnhancementFilterType::New();
	multiScaleEnhancementFilter->SetInput(reader->GetOutput());
	multiScaleEnhancementFilter->SetHessianToMeasureFilter( objectnessFilter );
	multiScaleEnhancementFilter->SetSigmaStepMethodToEquispaced();
	multiScaleEnhancementFilter->SetSigmaMinimum( 1.0 );
	multiScaleEnhancementFilter->SetSigmaMaximum( 4.0 );
	multiScaleEnhancementFilter->SetNumberOfSigmaSteps( 4 );
	
    ////////////////////////////////////////////////
    // 3) Rescale image intensity	
	
	typedef itk::Image< unsigned char, Dimension > OutputImageType;
	typedef itk::RescaleIntensityImageFilter< ImageType, OutputImageType > 
		RescaleFilterType;
	RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
	rescaleFilter->SetInput( multiScaleEnhancementFilter->GetOutput() );
	
    ////////////////////////////////////////////////
    // 4) Write output image
    
	typedef itk::ImageFileWriter< OutputImageType > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetInput( rescaleFilter->GetOutput() );
    writer->SetFileName( outputImage );
    
    try {
        writer->Update();
    } catch (itk::ExceptionObject & error) {
        std::cerr << "Error: " << error << std::endl;
        return EXIT_FAILURE;
    }
}