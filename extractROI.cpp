//
//  extractROI.cpp
//  Reads in DICOM series and allows user to input coordinates to extract ROI
//  
//  Created by Jonathan Young on 28 January 2016.
//

#include <iostream>
#include "itkImage.h"
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "itkImageSeriesWriter.h"
#include "itkRegionOfInterestImageFilter.h"


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
    typedef itk::ImageSeriesReader< ImageType > ReaderType;
    typedef itk::GDCMImageIO ImageIOType;
    typedef itk::GDCMSeriesFileNames InputNamesGeneratorType;
    const char * outputImage = argv[2];
		
    ////////////////////////////////////////////////
    // 1) Read the input series
    
    ImageIOType::Pointer gdcmIO = ImageIOType::New();
    InputNamesGeneratorType::Pointer inputNames = 
		InputNamesGeneratorType::New();
    inputNames->SetInputDirectory( argv[1] );
    
    const ReaderType::FileNamesContainer &filenames =
      inputNames->GetInputFileNames();
    
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
    // 2) Extract ROI
    
	int x_i, x_f, y_i, y_f, z_i, z_f;
	
    std::cout << "Extracting region-of-interest (ROI)..." << std::endl;
    std::cout << "\nEnter x-coordinate (column) start: ";
    std::cin >> x_i;
    std::cout << "Enter x-coordinate (column) end: ";
    std::cin >> x_f;
    std::cout << "\nEnter y-coordinate (row) start: ";
    std::cin >> y_i;
    std::cout << "Enter y-coordinate (row) end: ";
    std::cin >> y_f;
    std::cout << "\nEnter z-coordinate (slice) start: ";
    std::cin >> z_i;
    std::cout << "Enter z-coordinate (slice) end: ";
    std::cin >> z_f;
    
    const itk::IndexValueType startx = static_cast<itk::IndexValueType>(x_i);
    const itk::IndexValueType endx = static_cast<itk::IndexValueType>(x_f);
    const itk::IndexValueType starty = static_cast<itk::IndexValueType>(y_i);
    const itk::IndexValueType endy = static_cast<itk::IndexValueType>(y_f);
    const itk::IndexValueType startz = static_cast<itk::IndexValueType>(z_i);
    const itk::IndexValueType endz = static_cast<itk::IndexValueType>(z_f);
    
    ImageType::IndexType start;
    start[0] = startx;
    start[1] = starty;
    start[2] = startz;
    
    ImageType::IndexType end;
    end[0] = endx;
    end[1] = endy;
    end[2] = endz;
    
    ImageType::RegionType region;
    region.SetIndex( start );
    region.SetUpperIndex( end );
    
    typedef itk::RegionOfInterestImageFilter<ImageType, ImageType> ROIfilter;
    ROIfilter::Pointer ROI = ROIfilter::New();
    ROI->SetInput( reader->GetOutput() );
    ROI->SetRegionOfInterest( region );
	
    ////////////////////////////////////////////////
    // 3) Write output image
    
	typedef itk::ImageFileWriter< ImageType > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetInput( ROI->GetOutput() );
    writer->SetFileName( outputImage );
    
    try {
        writer->Update();
    } catch (itk::ExceptionObject & error) {
        std::cerr << "Error: " << error << std::endl;
        return EXIT_FAILURE;
    }
}