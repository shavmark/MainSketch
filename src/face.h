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
		KinectFace(int bodyID = -1) { 
			faceId = bodyID; // uses the same id but would not have to
			bodyId = bodyID;
			valid = false; 
		}

		PointF leftEye() { return facePoint[FacePointType_EyeLeft]; };
		PointF rightEye() { return facePoint[FacePointType_EyeRight]; };
		PointF nose() { return facePoint[FacePointType_Nose]; };
		PointF mouthCornerLeft() { return facePoint[FacePointType_MouthCornerLeft]; };
		PointF mouthCornerRight() { return facePoint[FacePointType_MouthCornerRight]; };

		friend class KinectFaces;
		bool valid; // true when data is valid

	protected:
		PointF facePoint[FacePointType::FacePointType_Count];
		DetectionResult faceProperty[FaceProperty::FaceProperty_Count];
		RectI boundingBox;
		Vector4 faceRotation;
		IFaceFrameReader* pFaceReader;
		IFaceFrameSource* pFaceSource;
		int faceId;
		int bodyId; // map face to body
	};

	class KinectFaces {
	public:
		KinectFaces();

		void setup(IKinectSensor *sensor);
		void update(vector<ofxKinectForWindows2::Data::Body>);
		void draw(vector<ofxKinectForWindows2::Data::Body> bodies);

	private:
		void ExtractFaceRotationInDegrees(const Vector4* pQuaternion, int* pPitch, int* pYaw, int* pRoll);

		std::string property[FaceProperty::FaceProperty_Count];

		// Color Table
		vector<ofColor> colors;

		DWORD features;

		template<class Interface> void SafeRelease(Interface *& pInterfaceToRelease)
		{
			if (pInterfaceToRelease != NULL) {
				pInterfaceToRelease->Release();
				pInterfaceToRelease = NULL;
			}
		}
		vector<KinectFace> faces;

	};

}