#pragma once
#include "ofMain.h"
#include "ofxKinectForWindows2.h"
#include "Kinect.Face.h"
#include "Body.h"

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
		void update(vector<ofxKinectForWindows2::Data::Body>);
		void draw();

		bool drawFace() { return drawface; }

		PointF leftEye(int index) {return facePoint[index][FacePointType_EyeLeft];};
		PointF rightEye(int index) { return facePoint[index][FacePointType_EyeRight]; };
		PointF nose(int index) { return facePoint[index][FacePointType_Nose]; };
		PointF mouthCornerLeft(int index) { return facePoint[index][FacePointType_MouthCornerLeft]; };
		PointF mouthCornerRight(int index) { return facePoint[index][FacePointType_MouthCornerRight]; };

		ofPixels pixels;

	private:
		void ExtractFaceRotationInDegrees(const Vector4* pQuaternion, int* pPitch, int* pYaw, int* pRoll);

		bool drawface[BODY_COUNT];

		PointF facePoint[BODY_COUNT][FacePointType::FacePointType_Count];
		DetectionResult faceProperty[BODY_COUNT][FaceProperty::FaceProperty_Count];
		RectI boundingBox[BODY_COUNT];
		Vector4 faceRotation[BODY_COUNT];
		IFaceFrameReader* pFaceReader[BODY_COUNT];
		IFaceFrameSource* pFaceSource[BODY_COUNT];
		std::string property[FaceProperty::FaceProperty_Count];

		// Color Table
		ofColor color[BODY_COUNT];

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