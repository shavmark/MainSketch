#pragma once
#include "ofMain.h"
#include "Kinect.Face.h"

namespace From2552Software {

	class Kinect {
	public:
		Kinect() {
			pSensor = nullptr;
		}
		void open();
		IKinectSensor* getSensor() {
			return pSensor;
		}

		friend class KinectFaces;
		friend class KinectFace;

	private:
		IKinectSensor* pSensor;
		IColorFrameSource* pColorSource;
		IBodyFrameSource* pBodySource;
		IColorFrameReader* pColorReader;
		IBodyFrameReader* pBodyReader;
		IFrameDescription* pDescription;
		ICoordinateMapper* pCoordinateMapper;
		// Color Table
		vector<ofColor> colors;

		template<class Interface> static void SafeRelease(Interface *& pInterfaceToRelease)
		{
			if (pInterfaceToRelease != NULL) {
				pInterfaceToRelease->Release();
				pInterfaceToRelease = NULL;
			}
		}

	};
}
