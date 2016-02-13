//
//  Apply geodesic active contour method for liver segmentation
//  Follows the approach detailed in the Insight Software Guide
//  Code copied from ITK examples
//
//  INPUT:
//    - read/write directory with trailing slash
//    - region-of-interest as single image file
//    - output file name
//    - (x,y,z) seed coordinates
//    - sigma, sigmoid K1, K2 for gradient and sigmoid mapping
//    - propagation, curvature, advection scaling, # iterations for geodesic 
//      active contour
//  
//  Created on 2 February 2016
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
#include "itkGeodesicActiveContourLevelSetImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"


int main(int argc, const char *argv[])
{
	// Validate input parameters
	if (argc < 15) {
		std::cerr << "Missing parameters! Usage:" << std::endl;
		std::cerr << argv[0];
		std::cerr << " <Read/WriteDir> <InputImg> <OutputImg> ";
		std::cerr << "[seedX] [seedY] [seedZ] [initDist] ";
		std::cerr << "[sigma] [sigmoid K1] [sigmoid K2] ";
		std::cerr << "[propagation] [curvature] [advection] [iterations]";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	
	const unsigned int Dimension = 3;
	typedef float InputPixelType;
	typedef unsigned char OutputPixelType;
    typedef itk::Image< InputPixelType, Dimension > InputImageType;
	typedef itk::Image< OutputPixelType, Dimension > OutputImageType;

    ////////////////////////////////////////////////
    // 1) Read the input image

	typedef itk::ImageFileReader< InputImageType >  ReaderType;
	ReaderType::Pointer reader = ReaderType::New();
	std::string readpath( argv[1] );
	readpath.append( argv[2] );
	reader->SetFileName( readpath );
	reader->Update();
	
    ////////////////////////////////////////////////
    // 2) Curvature anisotropic diffusion
	
	typedef itk::CurvatureAnisotropicDiffusionImageFilter< 
		InputImageType, InputImageType > SmoothingFilterType;
	SmoothingFilterType::Pointer smoothing = SmoothingFilterType::New();
	smoothing->SetTimeStep(0.04);
	smoothing->SetNumberOfIterations(5);
	smoothing->SetConductanceParameter(9.0);
	smoothing->SetInput( reader->GetOutput() );
	
    ////////////////////////////////////////////////
    // 3) Gradient magnitude recursive Gaussian
	
	const double sigma = atof(argv[8]);
	typedef itk::GradientMagnitudeRecursiveGaussianImageFilter< 
		InputImageType, InputImageType > GradientFilterType;
	GradientFilterType::Pointer gradientMagnitude = GradientFilterType::New();
	gradientMagnitude->SetSigma( sigma );
	gradientMagnitude->SetInput( smoothing->GetOutput() );
	
    ////////////////////////////////////////////////
    // 4) Sigmoid mapping
	
	const double K1 = atof(argv[9]);
	const double K2 = atof(argv[10]);
	typedef itk::SigmoidImageFilter< InputImageType, InputImageType > 
		SigmoidFilterType;
	SigmoidFilterType::Pointer sigmoid = SigmoidFilterType::New();
	sigmoid->SetOutputMinimum(0.0);
	sigmoid->SetOutputMaximum(1.0);
	sigmoid->SetAlpha( (K2 - K1)/6 );
	sigmoid->SetBeta( (K1 + K2)/2 );
	sigmoid->SetInput( gradientMagnitude->GetOutput() );
	
    ////////////////////////////////////////////////
    // 5) Segmentation with geodesic active contour
	
	typedef itk::FastMarchingImageFilter< InputImageType, InputImageType > 
		FastMarchingFilterType;
	FastMarchingFilterType::Pointer fastMarching = FastMarchingFilterType::New();
	
	typedef itk::GeodesicActiveContourLevelSetImageFilter< 
		InputImageType, InputImageType > GeodesicActiveContourFilterType;
	GeodesicActiveContourFilterType::Pointer geodesicActiveContour = 
		GeodesicActiveContourFilterType::New();
	
	const double propagation = atof( argv[11] );
	const double curvature = atof( argv[12] );
	const double advection = atof( argv[13] );
	const double iterations = atoi( argv[14] );
	geodesicActiveContour->SetPropagationScaling( propagation );
	geodesicActiveContour->SetCurvatureScaling( curvature );
	geodesicActiveContour->SetAdvectionScaling( advection );
	geodesicActiveContour->SetMaximumRMSError(0.01);
	geodesicActiveContour->SetNumberOfIterations( iterations );
	
	geodesicActiveContour->SetInput( fastMarching->GetOutput() );
	geodesicActiveContour->SetFeatureImage( sigmoid->GetOutput() );
	
    ////////////////////////////////////////////////
    // 6) Binary thresholding
	
	typedef itk::BinaryThresholdImageFilter< InputImageType, OutputImageType > 
		ThresholdingFilterType;
	ThresholdingFilterType::Pointer thresholder = ThresholdingFilterType::New();
	thresholder->SetLowerThreshold(-1000.0);
	thresholder->SetUpperThreshold(0.0);
	thresholder->SetOutsideValue(0);
	thresholder->SetInsideValue(255);
	thresholder->SetInput( geodesicActiveContour->GetOutput() );
	
    ////////////////////////////////////////////////
    // 7) Finish setting up fast marching
	
	typedef FastMarchingFilterType::NodeContainer NodeContainer;
	typedef FastMarchingFilterType::NodeType NodeType;
	
	NodeContainer::Pointer seeds = NodeContainer::New();
	InputImageType::IndexType seedPosition;
	seedPosition[0] = atoi( argv[4] );
	seedPosition[1] = atoi( argv[5] );
	seedPosition[2] = atoi( argv[6] );
	const double initialDistance = atof( argv[7] );
	const double seedValue = -initialDistance;
	NodeType node;
	node.SetValue( seedValue );
	node.SetIndex( seedPosition );
	seeds->Initialize();
	seeds->InsertElement(0, node);
	
	fastMarching->SetTrialPoints( seeds );
	fastMarching->SetSpeedConstant(1.0);
	fastMarching->SetOutputSize( reader->GetOutput()->GetBufferedRegion().GetSize() );
	fastMarching->SetOutputRegion( reader->GetOutput()->GetBufferedRegion() );
	fastMarching->SetOutputSpacing( reader->GetOutput()->GetSpacing() );
	fastMarching->SetOutputOrigin( reader->GetOutput()->GetOrigin() );
	
	////////////////////////////////////////////////
    // 7) Write output image
	
	typedef itk::ImageFileWriter< OutputImageType > WriterType;
	WriterType::Pointer writer = WriterType::New();
	std::string writepath( argv[1] );
	writepath.append( argv[3] );
	writer->SetFileName( writepath );
	writer->SetInput( thresholder->GetOutput() );
	
	try {
		writer->Update();
	}
	catch( itk::ExceptionObject &excep ) {
		std::cerr << "Exception caught!" << std::endl;
		std::cerr << excep << std::endl;
		return EXIT_FAILURE;
	}
	
	// The following writer is used to save the output of the sigmoid mapping
	typedef itk::ImageFileWriter< InputImageType > InternalWriterType;
	InternalWriterType::Pointer speedWriter = InternalWriterType::New();
	speedWriter->SetInput( sigmoid->GetOutput() );
	std::string sigmoidpath( argv[1] );
	speedWriter->SetFileName( sigmoidpath.append("SigmoidForGeodesic.mha"));
	speedWriter->Update();
	
	return 0;
}
