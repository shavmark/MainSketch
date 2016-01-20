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

	class KinectFace : public Kinect2552BaseClass {
	public:
		KinectFace(Kinect2552 *pKinect=nullptr) {
			Kinect2552BaseClass::setup(pKinect);
		}

		PointF leftEye() { return facePoint[FacePointType_EyeLeft]; };
		PointF rightEye() { return facePoint[FacePointType_EyeRight]; };
		PointF nose() { return facePoint[FacePointType_Nose]; };
		PointF mouthCornerLeft() { return facePoint[FacePointType_MouthCornerLeft]; };
		PointF mouthCornerRight() { return facePoint[FacePointType_MouthCornerRight]; };

		friend class KinectFaces;

	protected:
		PointF facePoint[FacePointType::FacePointType_Count];
		DetectionResult faceProperty[FaceProperty::FaceProperty_Count];
		RectI boundingBox;
		Vector4 faceRotation;
		IFaceFrameReader* pFaceReader;
		IFaceFrameSource* pFaceSource;
	};

	class KinectFaces : Kinect2552BaseClass {
	public:
		KinectFaces();

		void setup(Kinect2552 *);
		void update();
		void draw();
		void drawProjected(int x, int y, int width, int height);
		void invalidate();
		int baseline();
	private:
		void ExtractFaceRotationInDegrees(const Vector4* pQuaternion, int* pPitch, int* pYaw, int* pRoll);

		std::string property[FaceProperty::FaceProperty_Count];

		DWORD features;

		vector<KinectFace> faces;
		ICoordinateMapper* pCoordinateMapper;
		IFrameDescription* pDescription;
		IBodyFrameReader* pBodyReader;
		IColorFrameReader* pColorReader;
		IBodyFrameSource* pBodySource;
		IColorFrameSource* pColorSource;
		IKinectSensor* pSensor;
		IFaceFrameReader* pFaceReader[BODY_COUNT];
		IFaceFrameSource* pFaceSource[BODY_COUNT];
		bool readfacereaders;
		
	};

}