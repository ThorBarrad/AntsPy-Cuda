
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include<pybind11/numpy.h>

#include "itkCudaRegistration.cuh"
#include "itkImageFileReader.h"
#include "itkJPEGImageIOFactory.h"

#include "itkSize.h"
#include "itkImportImageContainer.h"

#include "itkImageToHistogramFilter.h"
#include "itkIntensityWindowingImageFilter.h"
#include "itkHistogramMatchingImageFilter.h"
#include "itkEuler2DTransform.h"
#include "itkCenteredTransformInitializer.h"

namespace py = pybind11;

template <typename ImageType>
typename ImageType::Pointer
PreprocessImage(typename ImageType::ConstPointer inputImage,
                typename ImageType::PixelType    lowerScaleValue,
                typename ImageType::PixelType    upperScaleValue,
                float                            winsorizeLowerQuantile,
                float                            winsorizeUpperQuantile,
                typename ImageType::ConstPointer histogramMatchSourceImage = nullptr)
{
    typedef itk::Statistics::ImageToHistogramFilter<ImageType>   HistogramFilterType;
    typedef typename HistogramFilterType::InputBooleanObjectType InputBooleanObjectType;
    typedef typename HistogramFilterType::HistogramSizeType      HistogramSizeType;

    HistogramSizeType histogramSize(1);
    histogramSize[0] = 256;

    typename InputBooleanObjectType::Pointer autoMinMaxInputObject = InputBooleanObjectType::New();
    autoMinMaxInputObject->Set(true);

    typename HistogramFilterType::Pointer histogramFilter = HistogramFilterType::New();
    histogramFilter->SetInput(inputImage);
    histogramFilter->SetAutoMinimumMaximumInput(autoMinMaxInputObject);
    histogramFilter->SetHistogramSize(histogramSize);
    histogramFilter->SetMarginalScale(10.0);
    histogramFilter->Update();

    float lowerValue = histogramFilter->GetOutput()->Quantile(0, winsorizeLowerQuantile);
    float upperValue = histogramFilter->GetOutput()->Quantile(0, winsorizeUpperQuantile);

    typedef itk::IntensityWindowingImageFilter<ImageType, ImageType> IntensityWindowingImageFilterType;

    typename IntensityWindowingImageFilterType::Pointer windowingFilter = IntensityWindowingImageFilterType::New();
    windowingFilter->SetInput(inputImage);
    windowingFilter->SetWindowMinimum(lowerValue);
    windowingFilter->SetWindowMaximum(upperValue);
    windowingFilter->SetOutputMinimum(lowerScaleValue);
    windowingFilter->SetOutputMaximum(upperScaleValue);
    windowingFilter->Update();

    typename ImageType::Pointer outputImage = nullptr;
    if (histogramMatchSourceImage)
    {
        typedef itk::HistogramMatchingImageFilter<ImageType, ImageType> HistogramMatchingFilterType;
        typename HistogramMatchingFilterType::Pointer                   matchingFilter = HistogramMatchingFilterType::New();
        matchingFilter->SetSourceImage(windowingFilter->GetOutput());
        matchingFilter->SetReferenceImage(histogramMatchSourceImage);
        matchingFilter->SetNumberOfHistogramLevels(256);
        matchingFilter->SetNumberOfMatchPoints(12);
        matchingFilter->ThresholdAtMeanIntensityOn();
        matchingFilter->Update();

        outputImage = matchingFilter->GetOutput();
        outputImage->Update();
        outputImage->DisconnectPipeline();
    }
    else
    {
        outputImage = windowingFilter->GetOutput();
        outputImage->Update();
        outputImage->DisconnectPipeline();
    }
    return outputImage;
}

py::array_t<int> cudaRegistration(py::array_t<float> fixedImageArray, py::array_t<float> movingImageArray)
{
    py::buffer_info buf1 = fixedImageArray.request();
    py::buffer_info buf2 = movingImageArray.request();

    int w = buf1.shape[0];  // 640
    int h = buf1.shape[1];  // 198

    float* ptr1 = (float*)buf1.ptr;
    float* ptr2 = (float*)buf2.ptr;
    
    //1、定义图像的维度以及像素执行
    const   unsigned int Dimension = 2;                       //定义维度
    typedef float PixelType;                                  //图像像素类型
    typedef itk::Image<PixelType,Dimension> FixedImageType;   //输入数据的类型：参考图像 
    typedef itk::Image<PixelType,Dimension> MovingImageType;  //浮动图像

    typedef itk::Size<Dimension> SizeType;
    typedef itk::ImportImageContainer<SizeType::SizeValueType, PixelType> PixelContainer;

    SizeType size;
    size[0] = h;
    size[1] = w;
    
    FixedImageType::Pointer fixedImage = FixedImageType::New();
    PixelContainer::Pointer fixedContainer = PixelContainer::New();
    fixedContainer->SetImportPointer(ptr1, w * h);
    fixedImage->SetRegions(size);
    fixedImage->SetPixelContainer(fixedContainer.GetPointer());

    MovingImageType::Pointer movingImage = MovingImageType::New();
    PixelContainer::Pointer movingContainer = PixelContainer::New();
    movingContainer->SetImportPointer(ptr2, w * h);
    movingImage->SetRegions(size);
    movingImage->SetPixelContainer(movingContainer.GetPointer());

    /*-------------------------------------------------------------------------------------------------------------*/

    FixedImageType::Pointer preprocessFixedImage = PreprocessImage<FixedImageType>(
        fixedImage.GetPointer(),0,1,0,1,nullptr);

    MovingImageType::Pointer preprocessMovingImage = PreprocessImage<MovingImageType>(
        movingImage.GetPointer(),0,1,0,1,preprocessFixedImage.GetPointer());

    /*-------------------------------------------------------------------------------------------------------------*/

    typedef typename itk::Euler2DTransform<float> TransformType;
    typename TransformType::Pointer transform = TransformType::New();

    typedef itk::CenteredTransformInitializer<TransformType, FixedImageType, MovingImageType> TransformInitializerType;
    typename TransformInitializerType::Pointer initializer = TransformInitializerType::New();

    initializer->SetTransform(transform);
    initializer->SetFixedImage(fixedImage);
    initializer->SetMovingImage(movingImage);
    initializer->MomentsOn();
    initializer->InitializeTransform();

    /*-------------------------------------------------------------------------------------------------------------*/

    auto result = py::array_t<int>(w * h);
    py::buffer_info buf = result.request();
    int* ptr = (int*)buf.ptr;

    int *resPtr = itk::cudaRegistrationStart<FixedImageType, MovingImageType, TransformType>(
                    preprocessFixedImage, preprocessMovingImage, transform, fixedImage, movingImage);

    for(int i = 0; i < w * h; i++){
        ptr[i] = resPtr[i];
    }

    delete []resPtr;

    /*-------------------------------------------------------------------------------------------------------------*/

    return result;
}

PYBIND11_MODULE(cudaRegistration, m)
{
    m.def("cudaRegistration", &cudaRegistration);
}