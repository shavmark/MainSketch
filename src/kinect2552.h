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
			width = 0;
			height = 0;
			pColorReader = nullptr;
			pBodyReader = nullptr;
			pDepthReader = nullptr;
			pDescription = nullptr;
			pDepthSource = nullptr;
			pColorSource = nullptr;
			pBodySource = nullptr;
			pCoordinateMapper = nullptr;
		}
		void open();
		
		IKinectSensor* getSensor() {
			assert(pSensor);
			return pSensor;
		}
		ICoordinateMapper* getCoordinateMapper() {
			assert(pCoordinateMapper);
			return pCoordinateMapper;
		}
		IBodyFrameReader* getBodyReader() {
			assert(pBodyReader);
			return pBodyReader;
		}
		void coordinateMapper();
		friend class KinectFaces;
		friend class KinectFace;
		friend class KinectBodies;
		friend class KinectBody;

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

		int width;
		int height;

		unsigned int bufferSize;

		// Color Table
		vector<ofColor> colors;

		

	};
	//base class
	class Kinect2552BaseClass {
	public:
		void setup(Kinect2552 *pKinectIn) { pKinect = pKinectIn; valid = false;	};
		Kinect2552 *getKinect() { assert(pKinect); return pKinect; }
		void draw() {};
		bool ObjectValid() { return valid; } // data is in a good state
		void SetValid(bool b = true) { valid = b; };

		template<class Interface> void SafeRelease(Interface *& pInterfaceToRelease)
		{
			if (pInterfaceToRelease != NULL) {
				pInterfaceToRelease->Release();
				pInterfaceToRelease = NULL;
			}
		}
	private:
		Kinect2552 *pKinect;
		bool valid; // true when data is valid

	};
}
