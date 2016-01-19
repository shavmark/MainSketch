#pragma once
#include "ofMain.h"
#include "Kinect.Face.h"
#include "Body.h"
#include "kinect2552.h"

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

		void setup(Kinect2552 *);
		void update();
		void draw();
		void drawProjected(int x, int y, int width, int height);

	private:
		void ExtractFaceRotationInDegrees(const Vector4* pQuaternion, int* pPitch, int* pYaw, int* pRoll);

		std::string property[FaceProperty::FaceProperty_Count];

		DWORD features;

		vector<KinectFace> faces;

		Kinect2552 *pKinect;
	};

}