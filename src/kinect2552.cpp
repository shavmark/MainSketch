#include "kinect2552.h"

namespace From2552Software {

	void Kinect2552::open()
	{
		HRESULT hResult = S_OK;
		hResult = GetDefaultKinectSensor(&pSensor);
		if (FAILED(hResult)) {
			std::cerr << "Error : GetDefaultKinectSensor" << std::endl;
			return; //bugbug add error/exception handling
		}

		hResult = pSensor->Open();
		if (FAILED(hResult)) {
			std::cerr << "Error : IKinectSensor::Open()" << std::endl;
		}

		
		hResult = pSensor->get_ColorFrameSource(&pColorSource);
		if (FAILED(hResult)) {
			std::cerr << "Error : IKinectSensor::get_ColorFrameSource()" << std::endl;
			return;
		}

		
		hResult = pSensor->get_BodyFrameSource(&pBodySource);
		if (FAILED(hResult)) {
			std::cerr << "Error : IKinectSensor::get_BodyFrameSource()" << std::endl;
			return;
		}

		hResult = pSensor->get_DepthFrameSource(&pDepthSource);
		if (FAILED(hResult)) {
			std::cerr << "Error : IKinectSensor::get_DepthFrameSource()" << std::endl;
			return;
		}

		hResult = pDepthSource->OpenReader(&pDepthReader);
		if (FAILED(hResult)) {
			std::cerr << "Error : IDepthFrameSource::OpenReader()" << std::endl;
			return;
		}

		hResult = pColorSource->OpenReader(&pColorReader);
		if (FAILED(hResult)) {
			std::cerr << "Error : IColorFrameSource::OpenReader()" << std::endl;
			return;
		}
		
		hResult = pBodySource->OpenReader(&pBodyReader);
		if (FAILED(hResult)) {
			std::cerr << "Error : IBodyFrameSource::OpenReader()" << std::endl;
			return;
		}

		hResult = pColorSource->get_FrameDescription(&pDescription);
		if (FAILED(hResult)) {
			std::cerr << "Error : IColorFrameSource::get_FrameDescription()" << std::endl;
			return;
		}

		IFrameDescription* pDepthDescription;
		hResult = pDepthSource->get_FrameDescription(&pDepthDescription);
		if (FAILED(hResult)) {
			std::cerr << "Error : IDepthFrameSource::get_FrameDescription()" << std::endl;
			return;
		}

		pDescription->get_Width(&width); // 1920
		pDescription->get_Height(&height); // 1080
		bufferSize = width * height * 4 * sizeof(unsigned char);
		//ofSetWindowShape(width, height);

		hResult = pSensor->get_CoordinateMapper(&pCoordinateMapper);
		if (FAILED(hResult)) {
			std::cerr << "Error : IKinectSensor::get_CoordinateMapper()" << std::endl;
			return;
		}

		// Color Table, gives each body its own color
		colors.push_back(ofColor(255, 0, 0));
		colors.push_back(ofColor(0, 0, 255));
		colors.push_back(ofColor(255, 255, 0));
		colors.push_back(ofColor(255, 0, 255));
		colors.push_back(ofColor(0, 255, 255));
	}

	void Kinect2552::coordinateMapper()
	{
#if 0
		unsigned short minDepth, maxDepth;
		pDepthSource->get_DepthMinReliableDistance(&minDepth);
		pDepthSource->get_DepthMaxReliableDistance(&maxDepth);

		while (1) {
			// Color Frame
			IColorFrame* pColorFrame = nullptr;
			HRESULT hResult = pColorReader->AcquireLatestFrame(&pColorFrame);
			if (SUCCEEDED(hResult)) {
				hResult = pColorFrame->CopyConvertedFrameDataToArray(colorBufferSize, reinterpret_cast<BYTE*>(colorBufferMat.data), ColorImageFormat::ColorImageFormat_Bgra);
				if (SUCCEEDED(hResult)) {
					//cv::resize(colorBufferMat, colorMat, cv::Size(), 0.5, 0.5);
				}
			}
			//SafeRelease( pColorFrame );

			// Depth Frame
			IDepthFrame* pDepthFrame = nullptr;
			hResult = pDepthReader->AcquireLatestFrame(&pDepthFrame);
			if (SUCCEEDED(hResult)) {
				hResult = pDepthFrame->AccessUnderlyingBuffer(&depthBufferSize, reinterpret_cast<UINT16**>(&depthBufferMat.data));
				if (SUCCEEDED(hResult)) {
					depthBufferMat.convertTo(depthMat, CV_8U, -255.0f / 8000.0f, 255.0f);
				}
			}
			//SafeRelease( pDepthFrame );

			// Mapping (Depth to Color)
			if (SUCCEEDED(hResult)) {
				std::vector<ColorSpacePoint> colorSpacePoints(depthWidth * depthHeight);
				hResult = pCoordinateMapper->MapDepthFrameToColorSpace(depthWidth * depthHeight, reinterpret_cast<UINT16*>(depthBufferMat.data), depthWidth * depthHeight, &colorSpacePoints[0]);
				if (SUCCEEDED(hResult)) {
					coordinateMapperMat = cv::Scalar(0, 0, 0, 0);
					for (int y = 0; y < depthHeight; y++) {
						for (int x = 0; x < depthWidth; x++) {
							unsigned int index = y * depthWidth + x;
							ColorSpacePoint point = colorSpacePoints[index];
							int colorX = static_cast<int>(std::floor(point.X + 0.5));
							int colorY = static_cast<int>(std::floor(point.Y + 0.5));
							unsigned short depth = depthBufferMat.at<unsigned short>(y, x);
							if ((colorX >= 0) && (colorX < colorWidth) && (colorY >= 0) && (colorY < colorHeight)/* && ( depth >= minDepth ) && ( depth <= maxDepth )*/) {
								coordinateMapperMat.at<cv::Vec4b>(y, x) = colorBufferMat.at<cv::Vec4b>(colorY, colorX);
							}
						}
					}
				}
			}

			SafeRelease(pColorFrame);
			SafeRelease(pDepthFrame);

			cv::imshow("Color", colorMat);
			cv::imshow("Depth", depthMat);
			cv::imshow("CoordinateMapper", coordinateMapperMat);

			if (cv::waitKey(30) == VK_ESCAPE) {
				break;
			}
		}

		SafeRelease(pColorSource);
		SafeRelease(pDepthSource);
		SafeRelease(pColorReader);
		SafeRelease(pDepthReader);
		SafeRelease(pColorDescription);
		SafeRelease(pDepthDescription);
		SafeRelease(pCoordinateMapper);
		if (pSensor) {
			pSensor->Close();
		}
		SafeRelease(pSensor);
		cv::destroyAllWindows();

		return 0;
	}
#endif
}
}