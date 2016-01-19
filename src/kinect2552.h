#pragma once
#include "ofMain.h"
#include "Kinect.Face.h"

//https://github.com/Vangos, http://www.cs.princeton.edu/~edwardz/tutorials/kinect2/kinect0_sdl.html, https://github.com/UnaNancyOwen
//http://www.pointclouds.org/

namespace From2552Software {

	class Kinect2552 {
	public:
		Kinect2552() {
			pSensor = nullptr;
		}
		void open();
		IKinectSensor* getSensor() {
			return pSensor;
		}
		void coordinateMapper();
		friend class KinectFaces;
		friend class KinectFace;

	private:
		IKinectSensor* pSensor;
		IColorFrameReader* pColorReader;
		IBodyFrameReader*  pBodyReader;
		IDepthFrameReader* pDepthReader;
		IFrameDescription* pDescription;
		IDepthFrameSource* pDepthSource;
		IColorFrameSource* pColorSource;
		IBodyFrameSource*  pBodySource;


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
