#pragma once
#define Foundation_UnWindows_INCLUDED
#include "ofMain.h"
#include "Kinect.Face.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//https://github.com/Vangos, http://www.cs.princeton.edu/~edwardz/tutorials/kinect2/kinect0_sdl.html, https://github.com/UnaNancyOwen
//http://www.pointclouds.org/

namespace From2552Software {

	//base class for things like faces
	class Kinect2552BaseClass {
	public:
		Kinect2552BaseClass() { valid = false; }
		
		bool objectValid() { return valid; } // data is in a good state
		void setValid(bool b = true) { valid = b; };

		bool checkPointer(IUnknown *p, string message);
		bool checkPointer(Kinect2552BaseClass *p, string message);
		void logError(string error);
		void logVerbose(string message) { logTrace(message, OF_LOG_VERBOSE); }; // promote trace, make it obvious and easy
		void logTrace(string message, ofLogLevel level = OF_LOG_NOTICE);
		void logError(HRESULT hResult, string message = "");

		template<class Interface> void SafeRelease(Interface *& pInterfaceToRelease)
		{
			if (pInterfaceToRelease != NULL) {
				pInterfaceToRelease->Release();
				pInterfaceToRelease = NULL;
			}
		}

	private:
		bool valid; // true when data is valid



	};
	
	class Kinect2552 : public Kinect2552BaseClass {
	public:
		Kinect2552();
		~Kinect2552();
		bool open();
		
		IKinectSensor* getSensor() {
			checkPointer(pSensor, "getSensor");
			return pSensor;
		}
		ICoordinateMapper* getCoordinateMapper() {
			checkPointer(pCoordinateMapper, "getCoordinateMapper");
			return pCoordinateMapper;
		}
		IBodyFrameReader* getBodyReader() {
			checkPointer(pBodyReader, "getBodyReader");
			return pBodyReader;
		}
		ofColor getColor(int index) {
			return colors[index];
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

	//base class for things like faces
	class Kinect2552BaseClassBodyItems : public Kinect2552BaseClass {
	public:
		void setup(Kinect2552 *pKinectIn);
		Kinect2552 *getKinect() { checkPointer(pKinect, "getKinect"); return pKinect; }
		void draw() {};


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

	protected:
		virtual void setTrackingID(int index, UINT64 trackingId) {};
		void aquireBodyFrame();

	private:
		
		Kinect2552 *pKinect;
	};

	class KinectBody : public Kinect2552BaseClassBodyItems {
	public:
		KinectBody(Kinect2552 *pKinect = nullptr) {
			Kinect2552BaseClassBodyItems::setup(pKinect);
			leftHandState = HandState::HandState_Unknown;
			rightHandState = HandState::HandState_Unknown;
			logVerbose("KinectBody");
		}
		void draw(bool drawface=true);
		friend class KinectBodies;
	private:
		Joint joints[JointType::JointType_Count];
		HandState leftHandState;
		HandState rightHandState;
		PointF leanAmount;
	};

	

	class KinectFace : public Kinect2552BaseClassBodyItems {
	public:
		KinectFace(Kinect2552 *pKinect = nullptr) {
			Kinect2552BaseClassBodyItems::setup(pKinect);
			logVerbose("KinectFace");
		}
		~KinectFace();
		void cleanup();

		IFaceFrameReader* getFaceReader() {
			checkPointer(pFaceReader, "getFaceReader");
			return pFaceReader;
		}
		IFaceFrameSource* getFaceSource() {
			checkPointer(pFaceSource, "getFaceSource");
			return pFaceSource;
		}
		void draw();

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

	class KinectFaces : public Kinect2552BaseClassBodyItems {
	public:
		KinectFaces();
		~KinectFaces();

		void setup(Kinect2552 *);
		void update();
		void draw();
		void drawProjected(int x, int y, int width, int height);
		
		int baseline();
	protected:
		vector<KinectFace> faces;
		bool aquireFaceFrame();
		void ExtractFaceRotationInDegrees(const Vector4* pQuaternion, int* pPitch, int* pYaw, int* pRoll);
		void setTrackingID(int index, UINT64 trackingId) { faces[index].pFaceSource->put_TrackingId(trackingId); };

		DWORD features;


	};


	class KinectBodies : public KinectFaces {
	public:
		KinectBodies(bool usefaces = false) : KinectFaces(){ useFaces(usefaces); }
		void update();
		void draw();
		void setup(Kinect2552 *kinectInput);

		void useFaces(bool usefaces = true)  { includeFaces = usefaces; }
		bool usingFaces() { return includeFaces; }
	private:
		bool includeFaces;
		void setTrackingID(int index, UINT64 trackingId) {
			if (usingFaces()) { faces[index].getFaceSource()->put_TrackingId(trackingId); };
		}
		vector<KinectBody> bodies;
	};
}
