//
//  main.cpp
//  ITKVessel
//
//  Created by Jonathan Young on 1/14/16.
//  Copyright © 2016 Jonathan Young. All rights reserved.
//

#include <iostream>
#include "itkImage.h"
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "itkImageSeriesWriter.h"


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
    typedef itk::Image< PixelType, Dimension > InputImageType;
    typedef itk::Image< PixelType, Dimension > OutputImageType;
    typedef itk::ImageSeriesReader< InputImageType > ReaderType;
    typedef itk::GDCMImageIO ImageIOType;
    typedef itk::GDCMSeriesFileNames InputNamesGeneratorType;
    typedef itk::ImageFileWriter< OutputImageType > WriterType;
    const char * outputImage = argv[2];
    
    ////////////////////////////////////////////////
    // 1) Read the input series
    
    ImageIOType::Pointer gdcmIO = ImageIOType::New();
    InputNamesGeneratorType::Pointer inputNames = InputNamesGeneratorType::New();
    inputNames->SetInputDirectory( argv[1] );
    
    const ReaderType::FileNamesContainer &filenames = inputNames->GetInputFileNames();
    
    ReaderType::Pointer reader = ReaderType::New();
    
    reader->SetImageIO( gdcmIO );
    reader->SetFileNames( filenames );
    try {
        reader->Update();
    } catch (itk::ExceptionObject &excp) {
        std::cerr << "Exception thrown while reading the series" << std::endl;
        std::cerr << excp << std::endl;
        return EXIT_FAILURE;
    }
    
    ////////////////////////////////////////////////
    // 2) Write output image
    
    WriterType::Pointer writer = WriterType::New();
    writer->SetInput( reader->GetOutput() );
    writer->SetFileName( outputImage );
    
    try {
        writer->Update();
    } catch (itk::ExceptionObject & error) {
        std::cerr << "Error: " << error << std::endl;
        return EXIT_FAILURE;
    }
}
