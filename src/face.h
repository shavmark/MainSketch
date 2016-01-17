#pragma once
#include "ofMain.h"
#include "ofxKinectForWindows2.h"
#include "Kinect.Face.h"

#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace From2552Software {

	class KinectFace {
	public:
		KinectFace();

		void setup(IKinectSensor *sensor);
		void update(IColorFrameReader*colorReader, IBodyFrameReader* bodyReader);
		void draw();

		bool drawFace() { return drawface; }
		ofPixels pixels;

	private:
		void ExtractFaceRotationInDegrees(const Vector4* pQuaternion, int* pPitch, int* pYaw, int* pRoll);

		bool drawface;

		PointF facePoint[FacePointType::FacePointType_Count];
		
		IFaceFrameReader* pFaceReader[BODY_COUNT];
		IFaceFrameSource* pFaceSource[BODY_COUNT];
		std::string property[FaceProperty::FaceProperty_Count];

		DWORD features;

		template<class Interface> void SafeRelease(Interface *& pInterfaceToRelease)
		{
			if (pInterfaceToRelease != NULL) {
				pInterfaceToRelease->Release();
				pInterfaceToRelease = NULL;
			}
		}
	};

}