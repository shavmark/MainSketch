#pragma once
#define Foundation_UnWindows_INCLUDED
#include "ofMain.h"
#include "Kinect.Face.h"
#include "2552software.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//https://github.com/Vangos, http://www.cs.princeton.edu/~edwardz/tutorials/kinect2/kinect0_sdl.html, https://github.com/UnaNancyOwen
//http://www.pointclouds.org/
//https://books.google.com/books?id=CVUpCgAAQBAJ&pg=PA167&lpg=PA167&dq=GetAudioBodyCorrelation&source=bl&ots=UENPsaMG_J&sig=5RHwdiXn4T7gst6lVt9SFvp2ahw&hl=en&sa=X&ved=0ahUKEwjK9umrqL7KAhUIVz4KHVL-BEYQ6AEIPTAG#v=onepage&q=GetAudioBodyCorrelation&f=false

namespace From2552Software {

	//base class for things like faces
	class Kinect2552BaseClass : public BaseClass2552 {
	public:
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
		IBodyIndexFrameReader* getBodyIndexReader() {
			checkPointer(pBodyIndexReader, "getBodyIndexReader");
			return pBodyIndexReader;

		}
		ofColor getColor(int index) {
			return colors[index];
		}
		int getFrameWidth() { return width; }
		int getFrameHeight() { return height; }
		static const int personCount = BODY_COUNT;
		void coordinateMapper();

	private:
		IKinectSensor*     pSensor;
		IColorFrameReader* pColorReader;
		IBodyFrameReader*  pBodyReader;
		IDepthFrameReader* pDepthReader;
		IFrameDescription* pDescription;
		IDepthFrameSource* pDepthSource;
		IColorFrameSource* pColorSource;
		IBodyFrameSource*  pBodySource;

		IBodyIndexFrameSource* pBodyIndexSource;
		IBodyIndexFrameReader* pBodyIndexReader;

		ICoordinateMapper* pCoordinateMapper;

		int width; // size of the kinect frames
		int height;

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
		ICoordinateMapper* pCoordinateMapper2;
		IFrameDescription* pDescription2;
		IBodyFrameReader* pBodyReader2;
		IColorFrameReader* pColorReader2;
		IBodyFrameSource* pBodySource2;
		IColorFrameSource* pColorSource2;
		IKinectSensor* pSensor2;
		IFaceFrameReader* pFaceReader2[BODY_COUNT];
		IFaceFrameSource* pFaceSource2[BODY_COUNT];
		bool readfacereaders2;
#endif

	protected:
		virtual void setTrackingID(int index, UINT64 trackingId) {};
		void aquireBodyFrame();

	private:
		
		Kinect2552 *pKinect;
	};

	class KinectBody : public Kinect2552BaseClassBodyItems {
	public:
		KinectBody(Kinect2552 *pKinect = nullptr);
		void draw(bool drawface=true);
		bool isTalking();
		void setTalking(int count = 2); // let it linger just a bit
		Joint* getJoints() { return joints; }
		HandState* leftHand() { return &leftHandState; };
		HandState* rightHand() { return &rightHandState; };
		PointF *lean() { return &leanAmount; }

	private:
		Joint joints[JointType::JointType_Count];
		HandState leftHandState;
		HandState rightHandState;
		PointF leanAmount;
		int talking; // person is talking, this is a count down bool, each check reduces the count so things can disappear over time
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

	class KinectAudio : public Kinect2552BaseClassBodyItems {
	public:
		KinectAudio(Kinect2552 *pKinect = nullptr);
		~KinectAudio();

		const UINT64 NoTrackingID = _UI64_MAX - 1;
		const UINT64 NoTrackingIndex = -1;

		void setup(Kinect2552 *pKinect);
		void update();
		void draw() {};

		IAudioBeamFrameReader* getAudioBeamReader() {
			checkPointer(pAudioBeamReader, "getAudioBeamReader");
			return pAudioBeamReader;
		}
		IAudioSource* getAudioSource() {
			checkPointer(pAudioSource, "getAudioSource");
			return pAudioSource;
		}
		bool confident() { return  getConfidence() > 0.5f; }
		float getAngle() { return angle; }
		float getConfidence() { return confidence; }
		void getAudioBeam();
		void getAudioBody();
		void getAudioCorrelation();
		int  getTrackingBodyIndex() {return trackingIndex;}
		UINT64 getTrackingID() { return audioTrackingId; }

	protected:
		virtual void setTrackingID(int index, UINT64 trackingId);
		IAudioSource*		   pAudioSource;
		IAudioBeamFrameReader* pAudioBeamReader;
		UINT64 audioTrackingId;
		int trackingIndex;
		float angle;
		float confidence;
		UINT32 correlationCount;

	};

	class KinectBodies : public KinectFaces {
	public:
		KinectBodies() : KinectFaces() { useFaces(); useAudio(); }
		void update();
		void draw();
		void setup(Kinect2552 *kinectInput);

		void useFaces(bool usefaces = true)  { includeFaces = usefaces; }
		bool usingFaces() { return includeFaces; }

		void useAudio(bool useaudio = true) { includeAudio = useaudio; }
		bool usingAudio() { return includeAudio; }

	private:
		bool includeFaces;
		bool includeAudio;
		void setTrackingID(int index, UINT64 trackingId) {
			if (usingFaces()) { faces[index].getFaceSource()->put_TrackingId(trackingId); };
		}
		vector<KinectBody> bodies;
		KinectAudio audio;
	};
}
