#pragma once
#include "ofMain.h"
#include "Kinect.Face.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

#if _DEBUG
		// here just to support the baseline() function which allows if needed one function to prove out ideas etc
		// normally these items would be the Kinect2552 class
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
#endif

	private:
		Kinect2552 *pKinect;
		bool valid; // true when data is valid



	};

	class KinectBody : public Kinect2552BaseClass {
	public:
		KinectBody(Kinect2552 *pKinect = nullptr) {
			Kinect2552BaseClass::setup(pKinect);
			leftHandState = HandState::HandState_Unknown;
			rightHandState = HandState::HandState_Unknown;
		}
		friend class KinectBodies;
	private:
		Joint joints[JointType::JointType_Count];
		HandState leftHandState;
		HandState rightHandState;
		PointF leanAmount;
	};

	class KinectBodies : public Kinect2552BaseClass {
	public:
		void update();
		void draw();
		void setup(Kinect2552 *kinectInput);

		vector<KinectBody> bodies;
	};

	class KinectFace : public Kinect2552BaseClass {
	public:
		KinectFace(Kinect2552 *pKinect = nullptr) {
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
		
		int baseline();
	private:
		void ExtractFaceRotationInDegrees(const Vector4* pQuaternion, int* pPitch, int* pYaw, int* pRoll);

		std::string property[FaceProperty::FaceProperty_Count];

		DWORD features;

		vector<KinectFace> faces;


	};

}
