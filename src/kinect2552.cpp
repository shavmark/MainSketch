#include "kinect2552.h"

//file:///C:/Users/mark/Downloads/KinectHIG.2.0.pdf
// box2d

namespace From2552Software {

	/// <summary>
	/// KinectAudioStream constructor. from msft sdk
	/// </summary>
	KinectAudioStream::KinectAudioStream(IStream *p32BitAudio) :
		m_cRef(1),
		m_p32BitAudio(p32BitAudio),
		m_SpeechActive(false)

	{
	}

	/// <summary>
	/// SetSpeechState method
	/// </summary>
	void KinectAudioStream::SetSpeechState(bool state)
	{
		m_SpeechActive = state;
	}

	/////////////////////////////////////////////
	// IStream methods
	STDMETHODIMP KinectAudioStream::Read(void *pBuffer, ULONG cbBuffer, ULONG *pcbRead)
	{
		if (pcbRead == NULL || cbBuffer == NULL)
		{
			return E_INVALIDARG;
		}

		HRESULT hr = S_OK;

		// 32bit -> 16bit conversion support
		INT16* p16Buffer = (INT16*)pBuffer;
		int factor = sizeof(float) / sizeof(INT16);

		// 32 bit read support
		float* p32Buffer = new float[cbBuffer / factor];
		byte* index = (byte*)p32Buffer;
		ULONG bytesRead = 0;
		ULONG bytesRemaining = cbBuffer * factor;

		// Speech reads at high frequency - this slows down the process
		int sleepDuration = 50;

		// Speech Service isn't tolerant of partial reads
		while (bytesRemaining > 0)
		{
			// Stop returning Audio data if Speech isn't active
			if (!m_SpeechActive)
			{
				*pcbRead = 0;
				hr = S_FALSE;
				goto exit;
			}

			hr = m_p32BitAudio->Read(index, bytesRemaining, &bytesRead);
			index += bytesRead;
			bytesRemaining -= bytesRead;

			// All Audio buffers drained - wait for buffers to fill
			if (bytesRemaining != 0)
			{
				Sleep(sleepDuration);
			}
		}

		// Convert float value [-1,1] to int16 [SHRT_MIN, SHRT_MAX] and copy to output butter
		for (UINT i = 0; i < cbBuffer / factor; i++)
		{
			float sample = p32Buffer[i];

			// Make sure it is in the range [-1, +1]
			if (sample > 1.0f)
			{
				sample = 1.0f;
			}
			else if (sample < -1.0f)
			{
				sample = -1.0f;
			}

			// Scale float to the range (SHRT_MIN, SHRT_MAX] and then
			// convert to 16-bit signed with proper rounding
			float sampleScaled = sample * (float)SHRT_MAX;
			p16Buffer[i] = (sampleScaled > 0.f) ? (INT16)(sampleScaled + 0.5f) : (INT16)(sampleScaled - 0.5f);
		}

		*pcbRead = cbBuffer;

	exit:
		delete p32Buffer;
		return hr;
	}

	STDMETHODIMP KinectAudioStream::Write(const void *, ULONG, ULONG *)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP KinectAudioStream::Seek(LARGE_INTEGER /* dlibMove */, DWORD /* dwOrigin */, ULARGE_INTEGER * /* plibNewPosition */)
	{
		// Speech seeks and expects a seek implementation - but the NUIAudio stream doesnt support seeking
		return S_OK;
	}

	STDMETHODIMP KinectAudioStream::SetSize(ULARGE_INTEGER)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP KinectAudioStream::CopyTo(IStream *, ULARGE_INTEGER, ULARGE_INTEGER *, ULARGE_INTEGER *)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP KinectAudioStream::Commit(DWORD)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP KinectAudioStream::Revert()
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP KinectAudioStream::LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP KinectAudioStream::UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP KinectAudioStream::Stat(STATSTG *, DWORD)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP KinectAudioStream::Clone(IStream **)
	{
		return E_NOTIMPL;
	}

	KinectBody::KinectBody(Kinect2552 *pKinect) {
		logVerbose("KinectBody");
		Kinect2552BaseClassBodyItems::setup(pKinect);
		leftHandState = HandState::HandState_Unknown;
		rightHandState = HandState::HandState_Unknown;
		setTalking(false);
	}

	void Kinect2552BaseClassBodyItems::setup(Kinect2552 *pKinectIn) {
		pKinect = pKinectIn;

#if _DEBUG
		// used to create direct, simple code for the obscure bugs kinect and openframeworks seem to have
		pCoordinateMapper2 = nullptr;
		pDescription2 = nullptr;
		pBodyReader2 = nullptr;
		pColorReader2 = nullptr;
		pBodySource2 = nullptr;
		pColorSource2 = nullptr;
		pSensor2 = nullptr;
		readfacereaders2 = false;
#endif
	};

	Kinect2552::Kinect2552() {
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
		pBodyIndexSource = nullptr;
		pBodyIndexReader = nullptr;

		// Color Table, gives each body its own color
		colors.push_back(ofColor(255, 0, 0));
		colors.push_back(ofColor(0, 0, 255));
		colors.push_back(ofColor(255, 255, 0));
		colors.push_back(ofColor(0, 255, 255));
		colors.push_back(ofColor(255, 0, 255));
		colors.push_back(ofColor(255, 255, 255));

	}

	Kinect2552::~Kinect2552() {
		if (pSensor) {
			pSensor->Close();
		}
		SafeRelease(pSensor);
		SafeRelease(pColorReader);
		SafeRelease(pBodyReader);
		SafeRelease(pDepthReader);
		SafeRelease(pDescription);
		SafeRelease(pDepthSource);
		SafeRelease(pColorSource);
		SafeRelease(pBodySource);
		SafeRelease(pCoordinateMapper);
		SafeRelease(pBodyIndexSource);
		SafeRelease(pBodyIndexReader);
	}
	void KinectBody::setTalking(int count) { 
		talking = count; 
	}

	bool KinectBody::isTalking() {
		if (talking > 0) {
			--talking;
		}
		return talking > 0;
	}
	void KinectBodies::setup(Kinect2552 *kinectInput) {

		if (usingFaces()) {
			KinectFaces::setup(kinectInput);
		}
		else {
			Kinect2552BaseClassBodyItems::setup(kinectInput); // skip the base class setup, its not needed here
		}

		if (usingAudio()) {
			audio.setup(kinectInput);
		}

		for (int i = 0; i < getKinect()->personCount; ++i) { 
			KinectBody body(getKinect());
			bodies.push_back(body);
		}
	}
	void KinectBody::draw(bool drawface) {

		if (objectValid()) {
			//ofDrawCircle(600, 100, 30);
			

			ColorSpacePoint colorSpacePoint = { 0 };
			//ofDrawCircle(400, 100, 30);
			HRESULT hResult = getKinect()->getCoordinateMapper()->MapCameraPointToColorSpace(joints[JointType::JointType_HandLeft].Position, &colorSpacePoint);
			// fails here
			if (SUCCEEDED(hResult)) {
				//ofDrawCircle(700, 100, 30);
				int x = static_cast<int>(colorSpacePoint.X);
				int y = static_cast<int>(colorSpacePoint.Y);
				if ((x >= 0) && (x < getKinect()->getFrameWidth()) && (y >= 0) && (y < getKinect()->getFrameHeight())) {
					if (leftHandState == HandState::HandState_Open) {
						ofDrawCircle(x, y, 30);
					}
					else if (leftHandState == HandState::HandState_Closed) {
						ofDrawCircle(x, y, 5);
					}
					else if (leftHandState == HandState::HandState_Lasso) {
						ofDrawCircle(x, y, 15);
					}
				}
			}
			colorSpacePoint = { 0 };
			hResult = getKinect()->getCoordinateMapper()->MapCameraPointToColorSpace(joints[JointType::JointType_HandRight].Position, &colorSpacePoint);
			if (SUCCEEDED(hResult)) {
				int x = static_cast<int>(colorSpacePoint.X);
				int y = static_cast<int>(colorSpacePoint.Y);
				if ((x >= 0) && (x < getKinect()->getFrameWidth()) && (y >= 0) && (y < getKinect()->getFrameHeight())) {
					if (rightHandState == HandState::HandState_Open) {
						ofDrawCircle(x, y, 30);
					}
					else if (rightHandState == HandState::HandState_Closed) {
						ofDrawCircle(x, y, 5);
					}
					else if (rightHandState == HandState::HandState_Lasso) {
						ofDrawCircle(x, y, 15);
					}
				}
			}
			// Joint
			for (int type = 0; type < JointType::JointType_Count; type++) {
				colorSpacePoint = { 0 };
				getKinect()->getCoordinateMapper()->MapCameraPointToColorSpace(joints[type].Position, &colorSpacePoint);
				int x = static_cast<int>(colorSpacePoint.X);
				int y = static_cast<int>(colorSpacePoint.Y);

				if (joints[type].JointType == JointType::JointType_Head) {
					if (isTalking()) {
						//bugbug todo go to this to get better 3d projection CameraSpacePoint
						ofDrawLine(colorSpacePoint.X+2, colorSpacePoint.Y-5, colorSpacePoint.X + 40, colorSpacePoint.Y-5);
						ofDrawLine(colorSpacePoint.X + 5, colorSpacePoint.Y, colorSpacePoint.X + 45, colorSpacePoint.Y);
						ofDrawLine(colorSpacePoint.X+2, colorSpacePoint.Y +5, colorSpacePoint.X + 40, colorSpacePoint.Y + 5);
					}
				}
				if (!drawface) {
					if ((joints[type].JointType == JointType::JointType_Head) | 
						(joints[type].JointType == JointType::JointType_Neck)) {
						continue;// assume face is drawn elsewhere
					}
				}
				if ((x >= 0) && (x < getKinect()->getFrameWidth()) && (y >= 0) && (y < getKinect()->getFrameHeight())) {
					ofDrawCircle(x, y, 10);
				}

			}

		}

	}
	void KinectBodies::draw() {

		for (int count = 0; count < bodies.size(); count++) {
			ofSetColor(getKinect()->getColor(count));
			bodies[count].draw(!usingFaces());
		}

		if (usingFaces()) {
			KinectFaces::draw();
		}
	}

	void KinectBodies::update() {
		IBodyFrame* pBodyFrame = nullptr;
		HRESULT hResult = getKinect()->getBodyReader()->AcquireLatestFrame(&pBodyFrame);
		if (!hresultFails(hResult, "AcquireLatestFrame")) {
			IBody* pBody[Kinect2552::personCount] = { 0 };

			hResult = pBodyFrame->GetAndRefreshBodyData(Kinect2552::personCount, pBody);
			if (!hresultFails(hResult, "GetAndRefreshBodyData")) {
				for (int count = 0; count < Kinect2552::personCount; count++) {
					bodies[count].setTalking(0);
					bodies[count].setValid(false);
					// breaks here
					BOOLEAN bTracked = false;
					hResult = pBody[count]->get_IsTracked(&bTracked);
					if (SUCCEEDED(hResult) && bTracked) {
						// Set TrackingID to Detect Face
						// LEFT OFF HERE
						UINT64 trackingId = _UI64_MAX;
						hResult = pBody[count]->get_TrackingId(&trackingId);
						if (hresultFails(hResult, "get_TrackingId")) {
							return;
						}

						setTrackingID(count, trackingId); //bugbug if this fails should we just set an error and return?

						if (usingAudio()) {
							// see if any audio there
							audio.getAudioCorrelation();
							if (audio.getTrackingID() == trackingId) {
								audio.setValid(); 
								logTrace("set talking");
								bodies[count].setTalking();
							}
						}

						// get joints
						hResult = pBody[count]->GetJoints(JointType::JointType_Count, bodies[count].getJoints());
						if (!hresultFails(hResult, "GetJoints")) {
							// Left Hand State
							hResult = pBody[count]->get_HandLeftState(bodies[count].leftHand());
							if (hresultFails(hResult, "get_HandLeftState")) {
								return;
							}
							hResult = pBody[count]->get_HandRightState(bodies[count].rightHand());
							if (hresultFails(hResult, "get_HandRightState")) {
								return;
							}
							// Lean
							hResult = pBody[count]->get_Lean(bodies[count].lean());
							if (hresultFails(hResult, "get_Lean")) {
								return;
							}
							bodies[count].setValid();
						}
					}
				}
			}
			for (int count = 0; count < Kinect2552::personCount; count++) {
				SafeRelease(pBody[count]);
			}
		}

		SafeRelease(pBodyFrame);

		if (usingFaces()) {
			aquireFaceFrame();
		}
	}


	bool Kinect2552::open()
	{
		
		HRESULT hResult = GetDefaultKinectSensor(&pSensor);
		if (hresultFails(hResult, "GetDefaultKinectSensor")) {
			return false;
		}

		hResult = pSensor->Open();
		if (hresultFails(hResult, "IKinectSensor::Open")) {
			return false;
		}
		
		hResult = pSensor->get_BodyIndexFrameSource(&pBodyIndexSource);
		if (hresultFails(hResult, "get_BodyIndexFrameSource")) {
			return false;
		}

		hResult = pSensor->get_ColorFrameSource(&pColorSource);
		if (hresultFails(hResult, "get_ColorFrameSource")) {
			return false;
		}
		
		hResult = pSensor->get_BodyFrameSource(&pBodySource);
		if (hresultFails(hResult, "get_BodyFrameSource")) {
			return false;
		}

		hResult = pBodyIndexSource->OpenReader(&pBodyIndexReader);
		if (hresultFails(hResult, "pBodyIndexSource OpenReader")) {
			return false;
		}

		hResult = pColorSource->OpenReader(&pColorReader);
		if (hresultFails(hResult, "pColorSource OpenReader")) {
			return false;
		}
		
		hResult = pBodySource->OpenReader(&pBodyReader);
		if (hresultFails(hResult, "pBodySource OpenReader")) {
			return false;
		}

		hResult = pColorSource->get_FrameDescription(&pDescription);
		if (hresultFails(hResult, "get_FrameDescription")) {
			return false;
		}

		pDescription->get_Width(&width);  
		pDescription->get_Height(&height);  

		hResult = pSensor->get_CoordinateMapper(&pCoordinateMapper);
		if (hresultFails(hResult, "get_CoordinateMapper")) {
			return false;
		}

		logTrace("Kinect signed on, life is good.");

		return true;
	}

	void Kinect2552::coordinateMapper()
	{
}
KinectFaces::KinectFaces() {

	logVerbose("KinectFaces");

	// features are the same for all faces
	features = FaceFrameFeatures::FaceFrameFeatures_BoundingBoxInColorSpace
		| FaceFrameFeatures::FaceFrameFeatures_PointsInColorSpace
		| FaceFrameFeatures::FaceFrameFeatures_RotationOrientation
		| FaceFrameFeatures::FaceFrameFeatures_Happy
		| FaceFrameFeatures::FaceFrameFeatures_RightEyeClosed
		| FaceFrameFeatures::FaceFrameFeatures_LeftEyeClosed
		| FaceFrameFeatures::FaceFrameFeatures_MouthOpen
		| FaceFrameFeatures::FaceFrameFeatures_MouthMoved
		| FaceFrameFeatures::FaceFrameFeatures_LookingAway
		| FaceFrameFeatures::FaceFrameFeatures_Glasses
		| FaceFrameFeatures::FaceFrameFeatures_FaceEngagement;



};

KinectFace::~KinectFace() {
}
void KinectFace::cleanup()
{
	// do not call in destructor as pointers are used, call when needed
	SafeRelease(pFaceReader);
	SafeRelease(pFaceSource);
}
KinectFaces::~KinectFaces() {

	for (int i = 0; i < faces.size(); ++i) {
		faces[i].cleanup(); 
	}

}
// get the face readers
void KinectFaces::setup(Kinect2552 *kinectInput) {

	Kinect2552BaseClassBodyItems::setup(kinectInput);

	for (int i = 0; i < Kinect2552::personCount; ++i) {
		KinectFace face(getKinect());

		HRESULT hResult = CreateFaceFrameSource(getKinect()->getSensor(), 0, features, &face.pFaceSource);
		if (hresultFails(hResult, "CreateFaceFrameSource")) {
			return;
		}

		hResult = face.pFaceSource->OpenReader(&face.pFaceReader);
		if (hresultFails(hResult, "face.pFaceSource->OpenReader")) {
			return;
		}
		faces.push_back(face);
	}

}

void KinectFace::draw()
{
	if (objectValid()) {
		//ofDrawCircle(400, 100, 30);

		if (faceProperty[FaceProperty_LeftEyeClosed] != DetectionResult_Yes)
		{
			ofDrawCircle(leftEye().X-15, leftEye().Y, 10);
		}
		if (faceProperty[FaceProperty_RightEyeClosed] != DetectionResult_Yes)
		{
			ofDrawCircle(rightEye().X+15, rightEye().Y, 10);
		}
		ofDrawCircle(nose().X, nose().Y, 5);
		if (faceProperty[FaceProperty_Happy] == DetectionResult_Yes || faceProperty[FaceProperty_Happy] == DetectionResult_Maybe || faceProperty[FaceProperty_Happy] == DetectionResult_Unknown) {
			// smile as much as possible
			ofDrawCurve(mouthCornerLeft().X- 70, mouthCornerLeft().Y-70, mouthCornerLeft().X, mouthCornerRight().Y+30, mouthCornerRight().X, mouthCornerRight().Y+30, mouthCornerRight().X+ 70, mouthCornerRight().Y - 70);
		}
		else {
			float height;
			float offset = 0;
			if (faceProperty[FaceProperty_MouthOpen] == DetectionResult_Yes || faceProperty[FaceProperty_MouthOpen] == DetectionResult_Maybe)
			{
				height = 60.0;
				offset = height/2;
			}
			else
			{
				height = 5.0;
				offset = 10;
			}
			float width = abs(mouthCornerRight().X - mouthCornerLeft().X);
			ofDrawEllipse(mouthCornerLeft().X-5, mouthCornerLeft().Y+ offset, width+5, height);
		}
	}

}
void KinectFaces::draw()
{
	//ofDrawCircle(400, 100, 30);
	for (int count = 0; count < faces.size(); count++) {
		ofSetColor(getKinect()->getColor(count));
		faces[count].draw();
	}


}
void KinectFaces::ExtractFaceRotationInDegrees(const Vector4* pQuaternion, int* pPitch, int* pYaw, int* pRoll)
{
	double x = pQuaternion->x;
	double y = pQuaternion->y;
	double z = pQuaternion->z;
	double w = pQuaternion->w;

	// convert face rotation quaternion to Euler angles in degrees
	*pPitch = static_cast<int>(std::atan2(2 * (y * z + w * x), w * w - x * x - y * y + z * z) / M_PI * 180.0f);
	*pYaw = static_cast<int>(std::asin(2 * (w * y - x * z)) / M_PI * 180.0f);
	*pRoll = static_cast<int>(std::atan2(2 * (x * y + w * z), w * w + x * x - y * y - z * z) / M_PI * 180.0f);
}

void  Kinect2552BaseClassBodyItems::aquireBodyFrame()
{

	IBodyFrame* pBodyFrame = nullptr;
	HRESULT hResult = getKinect()->getBodyReader()->AcquireLatestFrame(&pBodyFrame); // getKinect()->getBodyReader() was pBodyReader
	if (!hresultFails(hResult, "AcquireLatestFrame")) {
		IBody* pBody[Kinect2552::personCount] = { 0 };
		hResult = pBodyFrame->GetAndRefreshBodyData(Kinect2552::personCount, pBody);
		if (!hresultFails(hResult, "GetAndRefreshBodyData")) {
			for (int count = 0; count < Kinect2552::personCount; count++) {
				BOOLEAN bTracked = false;
				hResult = pBody[count]->get_IsTracked(&bTracked);
				if (SUCCEEDED(hResult) && bTracked) {

					// Set TrackingID to Detect Face etc
					UINT64 trackingId = _UI64_MAX;
					hResult = pBody[count]->get_TrackingId(&trackingId);
					if (!hresultFails(hResult, "get_TrackingId")) {
						setTrackingID(count, trackingId);
					}
				}
			}
		}
		for (int count = 0; count < Kinect2552::personCount; count++) {
			SafeRelease(pBody[count]);
		}
		SafeRelease(pBodyFrame);
	}

}
// add faces to the bodies
void KinectFaces::update() {
	
	while (1) {
		aquireBodyFrame();
		if (aquireFaceFrame())
			break;
	}

}

void KinectFaces::drawProjected(int x, int y, int width, int height) {
	return;
}
// return true if face found
bool KinectFaces::aquireFaceFrame()
{

	for (int count = 0; count < Kinect2552::personCount; count++) {
		IFaceFrame* pFaceFrame = nullptr;
		HRESULT hResult = faces[count].getFaceReader()->AcquireLatestFrame(&pFaceFrame); // faces[count].getFaceReader() was pFaceReader[count]
		if (SUCCEEDED(hResult) && pFaceFrame != nullptr) {
			BOOLEAN bFaceTracked = false;
			hResult = pFaceFrame->get_IsTrackingIdValid(&bFaceTracked);
			if (SUCCEEDED(hResult) && bFaceTracked) {
				IFaceFrameResult* pFaceResult = nullptr;
				hResult = pFaceFrame->get_FaceFrameResult(&pFaceResult);
				if (SUCCEEDED(hResult) && pFaceResult != nullptr) {
					logVerbose("aquireFaceFrame");
					
					hResult = pFaceResult->GetFacePointsInColorSpace(FacePointType::FacePointType_Count, faces[count].facePoint);
					hResult = pFaceResult->get_FaceRotationQuaternion(&faces[count].faceRotation);
					hResult = pFaceResult->GetFaceProperties(FaceProperty::FaceProperty_Count, faces[count].faceProperty);
					hResult = pFaceResult->get_FaceBoundingBoxInColorSpace(&faces[count].boundingBox);
					SafeRelease(pFaceResult);
					SafeRelease(pFaceFrame);
					faces[count].setValid();
					return true;

				}
				SafeRelease(pFaceResult);
			}
		}
		SafeRelease(pFaceFrame);
	}
	return false;
}

#if _DEBUG
// This code should always work, but draw() needs to be called too, it replaces update when we need to figure something out 
int KinectFaces::baseline()
{
	// Sensor
	HRESULT hResult = S_OK;
	if (false && pSensor2 == nullptr) // was pSensor
	{
		hResult = GetDefaultKinectSensor(&pSensor2);
		if (FAILED(hResult)) {
			std::cerr << "Error : GetDefaultKinectSensor" << std::endl;
			return -1;
		}

		hResult = pSensor2->Open();
		if (FAILED(hResult)) {
			std::cerr << "Error : IKinectSensor::Open()" << std::endl;
			return -1;
		}

		// Source
		hResult = pSensor2->get_ColorFrameSource(&pColorSource2);
		if (FAILED(hResult)) {
			std::cerr << "Error : IKinectSensor::get_ColorFrameSource()" << std::endl;
			return -1;
		}

		hResult = pSensor2->get_BodyFrameSource(&pBodySource2);
		if (FAILED(hResult)) {
			std::cerr << "Error : IKinectSensor::get_BodyFrameSource()" << std::endl;
			return -1;
		}

		// Reader
		hResult = pColorSource2->OpenReader(&pColorReader2);
		if (FAILED(hResult)) {
			std::cerr << "Error : IColorFrameSource::OpenReader()" << std::endl;
			return -1;
		}

		hResult = pBodySource2->OpenReader(&pBodyReader2);
		if (FAILED(hResult)) {
			std::cerr << "Error : IBodyFrameSource::OpenReader()" << std::endl;
			return -1;
		}

		// Description
		hResult = pColorSource2->get_FrameDescription(&pDescription2);
		if (FAILED(hResult)) {
			std::cerr << "Error : IColorFrameSource::get_FrameDescription()" << std::endl;
			return -1;
		}

		int width = 0;
		int height = 0;
		pDescription2->get_Width(&width); // 1920
		pDescription2->get_Height(&height); // 1080
		unsigned int bufferSize = width * height * 4 * sizeof(unsigned char);


		// Coordinate Mapper
		hResult = pSensor2->get_CoordinateMapper(&pCoordinateMapper2);
		if (FAILED(hResult)) {
			std::cerr << "Error : IKinectSensor::get_CoordinateMapper()" << std::endl;
			return -1;
		}


		DWORD features = FaceFrameFeatures::FaceFrameFeatures_BoundingBoxInColorSpace
			| FaceFrameFeatures::FaceFrameFeatures_PointsInColorSpace
			| FaceFrameFeatures::FaceFrameFeatures_RotationOrientation
			| FaceFrameFeatures::FaceFrameFeatures_Happy
			| FaceFrameFeatures::FaceFrameFeatures_RightEyeClosed
			| FaceFrameFeatures::FaceFrameFeatures_LeftEyeClosed
			| FaceFrameFeatures::FaceFrameFeatures_MouthOpen
			| FaceFrameFeatures::FaceFrameFeatures_MouthMoved
			| FaceFrameFeatures::FaceFrameFeatures_LookingAway
			| FaceFrameFeatures::FaceFrameFeatures_Glasses
			| FaceFrameFeatures::FaceFrameFeatures_FaceEngagement;

		for (int count = 0; count < Kinect2552::personCount; count++) {
			KinectFace face(getKinect());
			// Source
			hResult = CreateFaceFrameSource(pSensor2, 0, features, &pFaceSource2[count]);
			if (FAILED(hResult)) {
				std::cerr << "Error : CreateFaceFrameSource" << std::endl;
				return -1;
			}

			// Reader
			hResult = pFaceSource2[count]->OpenReader(&pFaceReader2[count]);
			if (FAILED(hResult)) {
				std::cerr << "Error : IFaceFrameSource::OpenReader()" << std::endl;
				return -1;
			}
			faces.push_back(face);
		}
		// Face Property Table
		std::string property[FaceProperty::FaceProperty_Count];
		property[0] = "Happy";
		property[1] = "Engaged";
		property[2] = "WearingGlasses";
		property[3] = "LeftEyeClosed";
		property[4] = "RightEyeClosed";
		property[5] = "MouthOpen";
		property[6] = "MouthMoved";
		property[7] = "LookingAway";
	}
	while (1) {
		aquireBodyFrame();
		if (aquireFaceFrame())
			break;
	}

	return 0;

}
#endif

void KinectAudio::getAudioCommands() {
	unsigned long waitObject = WaitForSingleObject(hSpeechEvent, 0);
	if (waitObject == WAIT_TIMEOUT) {
		logVerbose("signaled");
	}
	else if (waitObject == WAIT_OBJECT_0) {
		logTrace("nonsignaled");
		// Retrieved Event
		const float confidenceThreshold = 0.3f;
		SPEVENT eventStatus;
		//eventStatus.eEventId = SPEI_UNDEFINED;
		unsigned long eventFetch = 0;
		pSpeechContext->GetEvents(1, &eventStatus, &eventFetch);
		while (eventFetch > 0) {
			switch (eventStatus.eEventId) {
				// Speech Recognition Events
				//   SPEI_HYPOTHESIS  : Estimate
				//   SPEI_RECOGNITION : Recognition
			case SPEI_HYPOTHESIS:
			case SPEI_RECOGNITION:
				if (eventStatus.elParamType == SPET_LPARAM_IS_OBJECT) {
					// Retrieved Phrase
					ISpRecoResult* pRecoResult = reinterpret_cast<ISpRecoResult*>(eventStatus.lParam);
					SPPHRASE* pPhrase = nullptr;
					HRESULT hResult = pRecoResult->GetPhrase(&pPhrase);
					if (!hresultFails(hResult, "GetPhrase")) {
						if ((pPhrase->pProperties != nullptr) && (pPhrase->pProperties->pFirstChild != nullptr)) {
							// Compared with the Phrase Tag in the grammar file
							const SPPHRASEPROPERTY* pSemantic = pPhrase->pProperties->pFirstChild;
							switch (pSemantic->Confidence) {
							case SP_LOW_CONFIDENCE:
								logTrace("SP_LOW_CONFIDENCE: " + Trace2552::wstrtostr(pSemantic->pszValue));
								break;
							case SP_NORMAL_CONFIDENCE:
								logTrace("SP_NORMAL_CONFIDENCE: " + Trace2552::wstrtostr(pSemantic->pszValue));
								break;
							case SP_HIGH_CONFIDENCE:
								logTrace("SP_HIGH_CONFIDENCE: " + Trace2552::wstrtostr(pSemantic->pszValue));
								break;
							}
							if (pSemantic->SREngineConfidence > confidenceThreshold) {
								logTrace("SREngineConfidence > confidenceThreshold: " + Trace2552::wstrtostr(pSemantic->pszValue));
							}
						}
						CoTaskMemFree(pPhrase);
					}
				}
			}
			pSpeechContext->GetEvents(1, &eventStatus, &eventFetch);
		}
	}
	else {
		logTrace("other");
	}

}
void KinectAudio::update() { 
	getAudioBody();
	getAudioBeam(); 
	getAudioCommands();
}
//http://www.buildinsider.net/small/kinectv2cpp/07
KinectAudio::KinectAudio(Kinect2552 *pKinect) {
	Kinect2552BaseClassBodyItems::setup(pKinect);
	logVerbose("KinectAudio");
	audioTrackingId = NoTrackingID;
	trackingIndex = NoTrackingIndex;
	angle = 0.0f;
	confidence = 0.0f;
	correlationCount = 0;
	pAudioBeamList=nullptr;
	pAudioBeam = nullptr;
	pAudioStream = nullptr;
	pAudioSource = nullptr;
	pAudioBeamReader = nullptr;
	pSpeechStream = nullptr;
	pSpeechRecognizer = nullptr;
	pSpeechContext = nullptr;
	pSpeechGrammar = nullptr;
	hSpeechEvent = INVALID_HANDLE_VALUE;
	audioStream = nullptr;
}

KinectAudio::~KinectAudio(){
	if (pSpeechRecognizer != nullptr) {
		pSpeechRecognizer->SetRecoState(SPRST_INACTIVE_WITH_PURGE);
	}
	SafeRelease(pAudioSource);
	SafeRelease(pAudioBeamReader);
	SafeRelease(pAudioBeamList);
	SafeRelease(pAudioBeam);
	SafeRelease(pAudioStream);
	SafeRelease(pSpeechStream); 
	SafeRelease(pSpeechRecognizer);
	SafeRelease(pSpeechContext);
	SafeRelease(pSpeechGrammar);
	if (hSpeechEvent != INVALID_HANDLE_VALUE) {
		CloseHandle(hSpeechEvent);
	}
	CoUninitialize();
	if (audioStream != nullptr) {
		audioStream->SetSpeechState(false);
		delete audioStream;
		audioStream = nullptr;
	}

}

void KinectAudio::setup(Kinect2552 *pKinect) {
	
	Kinect2552BaseClassBodyItems::setup(pKinect); 

	HRESULT hResult = getKinect()->getSensor()->get_AudioSource(&pAudioSource);
	if (hresultFails(hResult, "get_AudioSource")) {
		return;
	}

	hResult = pAudioSource->OpenReader(&pAudioBeamReader);
	if (hresultFails(hResult, "IAudioSource::OpenReader")) {
		return;
	}

	hResult = pAudioSource->get_AudioBeams(&pAudioBeamList);
	if (hresultFails(hResult, "IAudioSource::get_AudioBeams")) {
		return;
	}

	hResult = pAudioBeamList->OpenAudioBeam(0, &pAudioBeam);
	if (hresultFails(hResult, "pAudioBeamList->OpenAudioBeam")) {
		return;
	}

	hResult = pAudioBeam->OpenInputStream(&pAudioStream);
	if (hresultFails(hResult, "IAudioSource::OpenInputStream")) {
		return;
	}

	audioStream = new KinectAudioStream(pAudioStream);

	hResult = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (hresultFails(hResult, "CoInitializeEx")) {
		return;
	}

	if (FAILED(setupSpeachStream())) {
		return;
	}
	if (FAILED(createSpeechRecognizer())) {
		return;
	}
	if (FAILED(findKinect())) {
		return;
	}
	if (FAILED(startSpeechRecognition())) {
		return;
	}

}

HRESULT KinectAudio::setupSpeachStream() {

	HRESULT hResult = CoCreateInstance(CLSID_SpStream, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpStream), (void**)&pSpeechStream);
	if (hresultFails(hResult, "CoCreateInstance( CLSID_SpStream )")) {
		return hResult;
	}

	WORD AudioFormat = WAVE_FORMAT_PCM;
	WORD AudioChannels = 1;
	DWORD AudioSamplesPerSecond = 16000;
	DWORD AudioAverageBytesPerSecond = 32000;
	WORD AudioBlockAlign = 2;
	WORD AudioBitsPerSample = 16;

	WAVEFORMATEX waveFormat = { AudioFormat, AudioChannels, AudioSamplesPerSecond, AudioAverageBytesPerSecond, AudioBlockAlign, AudioBitsPerSample, 0 };

	audioStream->SetSpeechState(true);
	hResult = pSpeechStream->SetBaseStream(audioStream, SPDFID_WaveFormatEx, &waveFormat);
	if (hresultFails(hResult, "ISpStream::SetBaseStream")) {
		return hResult;
	}

	return hResult;

}
HRESULT KinectAudio::findKinect() {

	ISpObjectTokenCategory* pTokenCategory = nullptr;
	HRESULT hResult = CoCreateInstance(CLSID_SpObjectTokenCategory, nullptr, CLSCTX_ALL, __uuidof(ISpObjectTokenCategory), reinterpret_cast<void**>(&pTokenCategory));
	if (hresultFails(hResult, "CoCreateInstance")) {
		return hResult;
	}

	hResult = pTokenCategory->SetId(SPCAT_RECOGNIZERS, false);
	if (hresultFails(hResult, "ISpObjectTokenCategory::SetId()")) {
		SafeRelease(pTokenCategory);
		return hResult;
	}

	IEnumSpObjectTokens* pEnumTokens = nullptr;
	hResult = CoCreateInstance(CLSID_SpMMAudioEnum, nullptr, CLSCTX_ALL, __uuidof(IEnumSpObjectTokens), reinterpret_cast<void**>(&pEnumTokens));
	if (hresultFails(hResult, "CoCreateInstance( CLSID_SpMMAudioEnum )")) {
		SafeRelease(pTokenCategory);
		return hResult;
	}

	// Find Best Token
	const wchar_t* pVendorPreferred = L"VendorPreferred";
	const unsigned long lengthVendorPreferred = static_cast<unsigned long>(wcslen(pVendorPreferred));
	unsigned long length;
	ULongAdd(lengthVendorPreferred, 1, &length);
	wchar_t* pAttribsVendorPreferred = new wchar_t[length];
	StringCchCopyW(pAttribsVendorPreferred, length, pVendorPreferred);

	hResult = pTokenCategory->EnumTokens(L"Language=409;Kinect=True", pAttribsVendorPreferred, &pEnumTokens); //  English "Language=409;Kinect=True"
	if (hresultFails(hResult, "pTokenCategory->EnumTokens")) {
		SafeRelease(pTokenCategory);
		return hResult;
	}

	SafeRelease(pTokenCategory);
	delete[] pAttribsVendorPreferred;

	ISpObjectToken* pEngineToken = nullptr;
	hResult = pEnumTokens->Next(1, &pEngineToken, nullptr);
	if (hresultFails(hResult, "ISpObjectToken Next")) {
		SafeRelease(pTokenCategory);
		return hResult;
	}
	if (hResult == S_FALSE) {
		//note this but continus things will still work, not sure it matters with the new sdk
		logVerbose("Kinect not found");
	}
	SafeRelease(pEnumTokens);
	SafeRelease(pTokenCategory);

	// Set Speech Recognizer
	hResult = pSpeechRecognizer->SetRecognizer(pEngineToken);
	if (hresultFails(hResult, "SetRecognizer")) {
		return hResult;
	}
	SafeRelease(pEngineToken);

	hResult = pSpeechRecognizer->CreateRecoContext(&pSpeechContext);
	if (hresultFails(hResult, "CreateRecoContext")) {
		return hResult;
	}

	hResult = pSpeechRecognizer->SetPropertyNum(L"AdaptationOn", 0);
	if (hresultFails(hResult, "SetPropertyNum")) {
		return hResult;
	}

	return hResult;

}
HRESULT KinectAudio::createSpeechRecognizer()
{
	// Create Speech Recognizer Instance
	HRESULT hResult = CoCreateInstance(CLSID_SpInprocRecognizer, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpRecognizer), (void**)&pSpeechRecognizer);
	if (hresultFails(hResult, "CLSID_SpInprocRecognizer")) {
		return hResult;
	}

	// Set Input Stream
	hResult = pSpeechRecognizer->SetInput(pSpeechStream, TRUE);
	if (hresultFails(hResult, "pSpeechRecognizer->SetInput")) {
		return hResult;
	}

	return hResult;
}

HRESULT KinectAudio::startSpeechRecognition()
{
	HRESULT hResult = pSpeechContext->CreateGrammar(1, &pSpeechGrammar);
	if (hresultFails(hResult, "CreateGrammar")) {
		return hResult;
	}

	hResult = pSpeechGrammar->LoadCmdFromFile(L"grammar.grxml", SPLO_STATIC); // http://www.w3.org/TR/speech-grammar/ (UTF-8/CRLF)
	if (hresultFails(hResult, "LoadCmdFromFile")) {
		return hResult;
	}

	// Specify that all top level rules in grammar are now active
	hResult = pSpeechGrammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);

	// Specify that engine should always be reading audio
	hResult = pSpeechRecognizer->SetRecoState(SPRST_ACTIVE_ALWAYS);

	// Specify that we're only interested in receiving recognition events
	hResult = pSpeechContext->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));

	// Ensure that engine is recognizing speech and not in paused state
	hResult = pSpeechContext->Resume(0);
	if (SUCCEEDED(hResult))
	{
		hSpeechEvent = pSpeechContext->GetNotifyEventHandle();
	}
	hSpeechEvent = pSpeechContext->GetNotifyEventHandle();

	return hResult;
}
void  KinectAudio::setTrackingID(int index, UINT64 trackingId) {
	logVerbose("KinectAudio::setTrackingID");

	if (trackingId == audioTrackingId) {
		trackingIndex = index;
		logTrace("set tracking id");
	}
}

// poll kenict to get audo and the body it came from
void KinectAudio::getAudioBody() {
	getAudioCorrelation();
	if (correlationCount != 0) {
		aquireBodyFrame();
	}
}
void KinectAudio::getAudioCorrelation() {
	correlationCount = 0;
	trackingIndex = NoTrackingIndex;
	audioTrackingId = NoTrackingID;

	IAudioBeamFrameList* pAudioBeamList = nullptr;
	HRESULT hResult = getAudioBeamReader()->AcquireLatestBeamFrames(&pAudioBeamList);
	if (!hresultFails(hResult, "getAudioCorrelation AcquireLatestBeamFrames")) {

		IAudioBeamFrame* pAudioBeamFrame = nullptr;
		hResult = pAudioBeamList->OpenAudioBeamFrame(0, &pAudioBeamFrame);
		if (!hresultFails(hResult, "OpenAudioBeamFrame")) {
			IAudioBeamSubFrame* pAudioBeamSubFrame = nullptr;
			hResult = pAudioBeamFrame->GetSubFrame(0, &pAudioBeamSubFrame);
		
			if (!hresultFails(hResult, "GetSubFrame")) {
				hResult = pAudioBeamSubFrame->get_AudioBodyCorrelationCount(&correlationCount);
				
				if (SUCCEEDED(hResult) && (correlationCount != 0)) {
					IAudioBodyCorrelation* pAudioBodyCorrelation = nullptr;
					hResult = pAudioBeamSubFrame->GetAudioBodyCorrelation(0, &pAudioBodyCorrelation);
					
					if (!hresultFails(hResult, "GetAudioBodyCorrelation")) {
						hResult = pAudioBodyCorrelation->get_BodyTrackingId(&audioTrackingId);
						SafeRelease(pAudioBodyCorrelation);
					}
				}
				SafeRelease(pAudioBeamSubFrame);
			}
			SafeRelease(pAudioBeamFrame);
		}
		SafeRelease(pAudioBeamList);
	}

}

// AudioBeam Frame https://masteringof.wordpress.com/examples/sounds/ https://masteringof.wordpress.com/projects-based-on-book/
void KinectAudio::getAudioBeam() {

	IAudioBeamFrameList* pAudioBeamList = nullptr;
	HRESULT hResult = getAudioBeamReader()->AcquireLatestBeamFrames(&pAudioBeamList);
	if (!hresultFails(hResult, "getAudioBeam AcquireLatestBeamFrames")) {
		//bugbug add error handling maybe other clean up
		UINT beamCount = 0;
		hResult = pAudioBeamList->get_BeamCount(&beamCount);
		// Only one audio beam is currently supported, but write the code in case this changes
		for (int beam = 0; beam < beamCount; ++beam) {
			angle = 0.0f;
			confidence = 0.0f;
			IAudioBeamFrame* pAudioBeamFrame = nullptr;
			hResult = pAudioBeamList->OpenAudioBeamFrame(beam, &pAudioBeamFrame);
			
			if (!hresultFails(hResult, "OpenAudioBeamFrame")) {
				// Get Beam Angle and Confidence
				IAudioBeam* pAudioBeam = nullptr;
				hResult = pAudioBeamFrame->get_AudioBeam(&pAudioBeam);
				if (!hresultFails(hResult, "get_AudioBeam")) {
					pAudioBeam->get_BeamAngle(&angle); // radian [-0.872665f, 0.872665f]
					pAudioBeam->get_BeamAngleConfidence(&confidence); // confidence [0.0f, 1.0f]
					SafeRelease(pAudioBeam);
				}
				SafeRelease(pAudioBeamFrame);
			}
		}
		SafeRelease(pAudioBeamList);
	}

}
}