#include "kinect.h"

namespace From2552Software {

	void Kinect::open()
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

		int width = 0;
		int height = 0;
		pDescription->get_Width(&width); // 1920
		pDescription->get_Height(&height); // 1080
		unsigned int bufferSize = width * height * 4 * sizeof(unsigned char);

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
}