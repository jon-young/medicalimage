//
//  Execute fast marching method on 3D image
//
//  INPUT: 
//    - read/write directory with trailing slash
//    - region-of-interest as single image file
//    - output file name
//    - (x,y,z) seed coordinates
//    - sigma, sigmoid K1, K2 for gradient and sigmoid mapping
//    - stopping time, binary threshold for fast marching
//  
//  Code copied from ITK examples
//  Created on 3 February 2016
//  

#include <iostream>
#include <string>
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include "itkSigmoidImageFilter.h"
#include "itkFastMarchingImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"


int main(int argc, const char *argv[])
{
	// Validate input parameters
	if (argc < 12) {
		std::cerr << "Usage:" << std::endl;
		std::cerr << argv[0];
		std::cerr << " <Read/WriteDir> <InputImg> <OutputImg> ";
		std::cerr << "[seedX] [seedY] [seedZ] ";
		std::cerr << "[sigma] [sigmoid K1] [sigmoid K2] ";
		std::cerr << "[stopping time] [binary threshold]" << std::endl;
		return EXIT_FAILURE;
	}
	
	const unsigned int Dimension = 3;
	typedef unsigned short InputPixelType;
	typedef float InternalPixelType;
	typedef unsigned char OutputPixelType;
    typedef itk::Image< InputPixelType, Dimension > InputImageType;
	typedef itk::Image< InternalPixelType, Dimension > InternalImageType;
	typedef itk::Image< OutputPixelType, Dimension > OutputImageType;
	
	////////////////////////////////////////////////
    // 1) Read the input image

	typedef itk::ImageFileReader< InputImageType > ReaderType;
	ReaderType::Pointer reader = ReaderType::New();
	std::string readpath(argv[1]);
	readpath.append(argv[2]);
	reader->SetFileName( readpath );
	
    ////////////////////////////////////////////////
    // 2) Curvature anisotropic diffusion
	
	typedef itk::CurvatureAnisotropicDiffusionImageFilter< 
		InputImageType, InternalImageType > SmoothingFilterType;
	SmoothingFilterType::Pointer smoothing = SmoothingFilterType::New();
	smoothing->SetInput( reader->GetOutput() );
	smoothing->SetTimeStep(0.04);
	smoothing->SetNumberOfIterations(5);
	smoothing->SetConductanceParameter(9.0);
	
    ////////////////////////////////////////////////
    // 3) Gradient magnitude recursive Gaussian
	
	const double sigma = atof(argv[7]);
	typedef itk::GradientMagnitudeRecursiveGaussianImageFilter< 
		InternalImageType, InternalImageType > GradientFilterType;
	GradientFilterType::Pointer gradMagnitude = GradientFilterType::New();
	gradMagnitude->SetInput( smoothing->GetOutput() );
	gradMagnitude->SetSigma( sigma );
	
    ////////////////////////////////////////////////
    // 4) Sigmoid mapping
	
	const double K1 = atof(argv[8]);
	const double K2 = atof(argv[9]);
	typedef itk::SigmoidImageFilter< InternalImageType, InternalImageType > 
		SigmoidFilterType;
	SigmoidFilterType::Pointer sigmoid = SigmoidFilterType::New();
	sigmoid->SetInput( gradMagnitude->GetOutput() );
	sigmoid->SetOutputMinimum(0.0);
	sigmoid->SetOutputMaximum(1.0);
	sigmoid->SetAlpha( (K2 - K1)/6 );
	sigmoid->SetBeta( (K1 + K2)/2 );
	
    ////////////////////////////////////////////////
    // 5) Fast Marching
	
	typedef itk::FastMarchingImageFilter< InternalImageType, InternalImageType > 
		FastMarchingFilterType;
	typedef FastMarchingFilterType::NodeContainer NodeContainer;
	typedef FastMarchingFilterType::NodeType NodeType;
	
	InternalImageType::IndexType seedPosition;
	seedPosition[0] = atoi( argv[4] );
	seedPosition[1] = atoi( argv[5] );
	seedPosition[2] = atoi( argv[6] );
	NodeType node;
	const double seedVal = 0.0;
	node.SetValue( seedVal );
	node.SetIndex( seedPosition );
	NodeContainer::Pointer seeds = NodeContainer::New();
	seeds->Initialize();
	seeds->InsertElement(0, node);
	
	const double stoppingTime = atof( argv[10] );
	FastMarchingFilterType::Pointer fastMarching = FastMarchingFilterType::New();
	fastMarching->SetInput( sigmoid->GetOutput() );
	fastMarching->SetTrialPoints( seeds );
	fastMarching->SetOutputSize( 
		reader->GetOutput()->GetBufferedRegion().GetSize() );
	fastMarching->SetStoppingValue( stoppingTime );
	
    ////////////////////////////////////////////////
    // 6) Binary Thresholding
	
	const InternalPixelType timeThreshold = atof( argv[11] );
	typedef itk::BinaryThresholdImageFilter< InternalImageType, OutputImageType > 
		ThresholdingFilterType;
	ThresholdingFilterType::Pointer thresholder = ThresholdingFilterType::New();
	thresholder->SetInput( fastMarching->GetOutput() );
	thresholder->SetLowerThreshold( 0.0 );
	thresholder->SetUpperThreshold( timeThreshold );
	thresholder->SetOutsideValue( 0 );
	thresholder->SetInsideValue( 255 );
	
    ////////////////////////////////////////////////
    // 7) Write output image
	
	typedef itk::ImageFileWriter< OutputImageType > WriterType;
	WriterType::Pointer writer = WriterType::New();
	std::string writepath(argv[1]);
	writepath.append(argv[3]);
	writer->SetFileName( writepath );
	writer->SetInput( thresholder->GetOutput() );
	try {
		writer->Update();
	}
	catch( itk::ExceptionObject & excep ) {
		std::cerr << "Exception caught!" << std::endl;
		std::cerr << excep << std::endl;
		return EXIT_FAILURE;
	}
	
    // The following writer type is used to save the output of the sigmoid 
    // mapping so that it can be used with a viewer to help determine an 
	// appropriate threshold to be used on the output of the fast marching filter
	//
	/*
	typedef itk::ImageFileWriter< InternalImageType > InternalWriterType;
	InternalWriterType::Pointer speedWriter = InternalWriterType::New();
	speedWriter->SetInput( sigmoid->GetOutput() );
	std::string sigmoidpath(argv[1]);
	speedWriter->SetFileName( sigmoidpath.append("SigmoidOutput.mha") );
	speedWriter->Update();
	*/
	return 0;
}
