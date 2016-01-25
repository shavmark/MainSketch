#include "kinect2552.h"

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
		if (SUCCEEDED(hResult)) {
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
#if 0
		unsigned short minDepth, maxDepth;
		pDepthSource->get_DepthMinReliableDistance(&minDepth);
		pDepthSource->get_DepthMaxReliableDistance(&maxDepth);

		while (1) {
			// Color Frame
			IColorFrame* pColorFrame = nullptr;
			HRESULT hResult = pColorReader->AcquireLatestFrame(&pColorFrame);
			if (SUCCEEDED(hResult)) {
				hResult = pColorFrame->CopyConvertedFrameDataToArray(colorBufferSize, reinterpret_cast<BYTE*>(colorBufferMat.data), ColorImageFormat::ColorImageFormat_Bgra);
				if (SUCCEEDED(hResult)) {
					//cv::resize(colorBufferMat, colorMat, cv::Size(), 0.5, 0.5);
				}
			}
			//SafeRelease( pColorFrame );

			// Depth Frame
			IDepthFrame* pDepthFrame = nullptr;
			hResult = pDepthReader->AcquireLatestFrame(&pDepthFrame);
			if (SUCCEEDED(hResult)) {
				hResult = pDepthFrame->AccessUnderlyingBuffer(&depthBufferSize, reinterpret_cast<UINT16**>(&depthBufferMat.data));
				if (SUCCEEDED(hResult)) {
					depthBufferMat.convertTo(depthMat, CV_8U, -255.0f / 8000.0f, 255.0f);
				}
			}
			//SafeRelease( pDepthFrame );

			// Mapping (Depth to Color)
			if (SUCCEEDED(hResult)) {
				std::vector<ColorSpacePoint> colorSpacePoints(depthWidth * depthHeight);
				hResult = pCoordinateMapper->MapDepthFrameToColorSpace(depthWidth * depthHeight, reinterpret_cast<UINT16*>(depthBufferMat.data), depthWidth * depthHeight, &colorSpacePoints[0]);
				if (SUCCEEDED(hResult)) {
					coordinateMapperMat = cv::Scalar(0, 0, 0, 0);
					for (int y = 0; y < depthHeight; y++) {
						for (int x = 0; x < depthWidth; x++) {
							unsigned int index = y * depthWidth + x;
							ColorSpacePoint point = colorSpacePoints[index];
							int colorX = static_cast<int>(std::floor(point.X + 0.5));
							int colorY = static_cast<int>(std::floor(point.Y + 0.5));
							unsigned short depth = depthBufferMat.at<unsigned short>(y, x);
							if ((colorX >= 0) && (colorX < colorWidth) && (colorY >= 0) && (colorY < colorHeight)/* && ( depth >= minDepth ) && ( depth <= maxDepth )*/) {
								coordinateMapperMat.at<cv::Vec4b>(y, x) = colorBufferMat.at<cv::Vec4b>(colorY, colorX);
							}
						}
					}
				}
			}

			SafeRelease(pColorFrame);
			SafeRelease(pDepthFrame);

			cv::imshow("Color", colorMat);
			cv::imshow("Depth", depthMat);
			cv::imshow("CoordinateMapper", coordinateMapperMat);

			if (cv::waitKey(30) == VK_ESCAPE) {
				break;
			}
		}

		SafeRelease(pColorSource);
		SafeRelease(pDepthSource);
		SafeRelease(pColorReader);
		SafeRelease(pDepthReader);
		SafeRelease(pColorDescription);
		SafeRelease(pDepthDescription);
		SafeRelease(pCoordinateMapper);
		if (pSensor) {
			pSensor->Close();
		}
		SafeRelease(pSensor);
		cv::destroyAllWindows();

		return 0;
	}
#endif
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


#ifdef learning
	for (int count = 0; count < BODY_COUNT; count++) {
		if (drawface[count]) {
			//ofSetColor(255, 255, 255);
			ofDrawCircle(400, 100, 30);
			break;
			ofDrawCircle(leftEye(count).X, leftEye(count).Y, 5);
			ofDrawCircle(rightEye(count).X, rightEye(count).Y, 5);
			ofDrawCircle(nose(count).X, nose(count).Y, 5);
			return;
			//ofDrawCircle(mouthCornerLeft(count).X, mouthCornerLeft(count).Y, 5);
			//ofDrawCircle(mouthCornerRight(count).X, mouthCornerRight(count).Y, 5);
			float width = abs(mouthCornerRight(count).X - mouthCornerLeft(count).X);
			float height;
			if (faceProperty[count][FaceProperty_MouthOpen] == DetectionResult_Yes || faceProperty[count][FaceProperty_MouthOpen] == DetectionResult_Maybe)
			{
				height = 20.0;
			}
			else
			{
				height = 5.0;
			}

			//ofDrawEllipse(mouthCornerLeft(count).X, mouthCornerLeft(count).Y, width, height);
			//ofDrawEllipse(mouthCornerLeft(count).X, mouthCornerLeft(count).Y, 
			//mouthCornerRight(count).X - mouthCornerLeft(count).X, mouthCornerLeft(count).Y - mouthCornerRight(count).Y);
			//cv::rectangle(bufferMat, cv::Rect(boundingBox.Left, boundingBox.Top, boundingBox.Right - boundingBox.Left, boundingBox.Bottom - boundingBox.Top), static_cast<cv::Scalar>(color[count]));
			//if (SUCCEEDED(hResult)) {
			//	int pitch, yaw, roll;
			//	ExtractFaceRotationInDegrees(&faceRotation, &pitch, &yaw, &roll);
			//	result.push_back("Pitch, Yaw, Roll : " + std::to_string(pitch) + ", " + std::to_string(yaw) + ", " + std::to_string(roll));
			//}
			/*
			if (SUCCEEDED(hResult)) {
			for (int count = 0; count < FaceProperty::FaceProperty_Count; count++) {
			switch (faceProperty[count]) {
			case DetectionResult::DetectionResult_Unknown:
			result.push_back(property[count] + " : Unknown");
			break;
			case DetectionResult::DetectionResult_Yes:
			result.push_back(property[count] + " : Yes");
			break;
			case DetectionResult::DetectionResult_No:
			result.push_back(property[count] + " : No");
			break;
			case DetectionResult::DetectionResult_Maybe:
			result.push_back(property[count] + " : Mayby");
			break;
			default:
			break;
			}
			}
			if (boundingBox.Left && boundingBox.Bottom) {
			int offset = 30;
			for (std::vector<std::string>::iterator it = result.begin(); it != result.end(); it++, offset += 30) {
			//cv::putText(bufferMat, *it, cv::Point(boundingBox.Left, boundingBox.Bottom + offset), cv::FONT_HERSHEY_COMPLEX, 1.0f, static_cast<cv::Scalar>(color[count]), 2, CV_AA);
			}
			}
			*/
		}
	}
#endif
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
	/*
	ofPushStyle();
	int w, h;
	switch (proj) {
	case ofxKFW2::ColorCamera: w = 1920; h = 1080; break;
	case ofxKFW2::DepthCamera: w = 512; h = 424; break;
	}

	const auto & bonesAtlas = Data::Body::getBonesAtlas();

	for (auto & body : bodies) {
	if (!body.tracked) continue;

	map<JointType, ofVec2f> jntsProj;

	for (auto & j : body.joints) {
	ofVec2f & p = jntsProj[j.second.getType()] = ofVec2f();

	TrackingState state = j.second.getTrackingState();
	if (state == TrackingState_NotTracked) continue;

	p.set(j.second.getProjected(coordinateMapper, proj));
	p.x = x + p.x / w * width;
	p.y = y + p.y / h * height;

	int radius = (state == TrackingState_Inferred) ? 2 : 8;
	ofSetColor(0, 255, 0);
	ofCircle(p.x, p.y, radius);
	}

	for (auto & bone : bonesAtlas) {
	drawProjectedBone(body.joints, jntsProj, bone.first, bone.second);
	}

	drawProjectedHand(body.leftHandState, jntsProj[JointType_HandLeft]);
	drawProjectedHand(body.rightHandState, jntsProj[JointType_HandRight]);
	}

	ofPopStyle();
	*/
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
void KinectAudio::update() { 
	getAudioBody();
	getAudioBeam(); 
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

int  KinectAudio::test(){
	// Sensor
	IKinectSensor* pSensor;
	HRESULT hResult = S_OK;
	hResult = GetDefaultKinectSensor(&pSensor);
	if (FAILED(hResult)) {
		std::cerr << "Error : GetDefaultKinectSensor" << std::endl;
		return -1;
	}

	hResult = pSensor->Open();
	if (FAILED(hResult)) {
		std::cerr << "Error : IKinectSensor::Open()" << std::endl;
		return -1;
	}

	// Source
	IAudioSource* pAudioSource;
	hResult = pSensor->get_AudioSource(&pAudioSource);
	if (FAILED(hResult)) {
		std::cerr << "Error : IKinectSensor::get_AudioSource()" << std::endl;
		return -1;
	}

	// Get Audio Beam List
	IAudioBeamList* pAudioBeamList;
	hResult = pAudioSource->get_AudioBeams(&pAudioBeamList);
	if (FAILED(hResult)) {
		std::cerr << "Error : IAudioSource::get_AudioBeams()" << std::endl;
		return -1;
	}

	// Open Audio Beam
	IAudioBeam* pAudioBeam;
	hResult = pAudioBeamList->OpenAudioBeam(0, &pAudioBeam);
	if (FAILED(hResult)) {
		std::cerr << "Error : IAudioBeamList::OpenAudioBeam()" << std::endl;
		return -1;
	}

	// Open Audio Input Stream 
	IStream* pAudioStream;
	hResult = pAudioBeam->OpenInputStream(&pAudioStream);
	if (FAILED(hResult)) {
		std::cerr << "Error : IAudioBeam::OpenInputStream()" << std::endl;
		return -1;
	}

	// Audio Stream Class from KinectAudioStream
	KinectAudioStream* audioStream = new KinectAudioStream(pAudioStream);

	// Initialize COM
	hResult = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hResult)) {
		std::cerr << "Error : CoInitializeEx()" << std::endl;
		return -1;
	}

	// Create Speech Stream Instance
	ISpStream* pSpeechStream;
	hResult = CoCreateInstance(CLSID_SpStream, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpStream), (void**)&pSpeechStream);
	if (FAILED(hResult)) {
		std::cerr << "Error : CoCreateInstance( CLSID_SpStream )" << std::endl;
		return -1;
	}

	// Initialize Speech Stream 
	WORD AudioFormat = WAVE_FORMAT_PCM;
	WORD AudioChannels = 1;
	DWORD AudioSamplesPerSecond = 16000;
	DWORD AudioAverageBytesPerSecond = 32000;
	WORD AudioBlockAlign = 2;
	WORD AudioBitsPerSample = 16;

	WAVEFORMATEX waveFormat = { AudioFormat, AudioChannels, AudioSamplesPerSecond, AudioAverageBytesPerSecond, AudioBlockAlign, AudioBitsPerSample, 0 };

	audioStream->SetSpeechState(true);
	hResult = pSpeechStream->SetBaseStream(audioStream, SPDFID_WaveFormatEx, &waveFormat);
	if (FAILED(hResult)) {
		std::cerr << "Error : ISpStream::SetBaseStream()" << std::endl;
		return -1;
	}

	/*** Create Speech Recognizer ***/
	std::cout << "Create Speech Recognizer" << std::endl;

	// Create Speech Recognizer Instance
	ISpRecognizer* pSpeechRecognizer;
	hResult = CoCreateInstance(CLSID_SpInprocRecognizer, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpRecognizer), (void**)&pSpeechRecognizer);
	if (FAILED(hResult)) {
		std::cerr << "Error : CoCreateInstance( CLSID_SpInprocRecognizer )" << std::endl;
		return -1;
	}

	// Set Input Stream
	hResult = pSpeechRecognizer->SetInput(pSpeechStream, TRUE);
	if (FAILED(hResult)) {
		std::cerr << "Error : ISpRecognizer::SetInput()" << std::endl;
		return -1;
	}

	/*
	// If can use ATL, easier to using SpFindBestToken(sphelper.h). When using Visual Studio Professional or more.
	// ISpObjectToken* pEngineToken = nullptr;
	// hResult = SpFindBestToken( SPCAT_RECOGNIZERS, L"Language=409;Kinect=True", NULL, &pEngineToken );
	*/

	///*
	// If can't use ATL, alternative to using SpFIndBestToken(sphelper.h). When using Visual Studio Express.
	// Create Token Category Instance
	ISpObjectTokenCategory* pTokenCategory = nullptr;
	hResult = CoCreateInstance(CLSID_SpObjectTokenCategory, nullptr, CLSCTX_ALL, __uuidof(ISpObjectTokenCategory), reinterpret_cast<void**>(&pTokenCategory));
	if (FAILED(hResult)) {
		std::cerr << "Error : CoCreateInstance( CLSID_SpObjectTokenCategory )" << std::endl;
		return -1;
	}

	// Set Recognizers Catedory ID
	hResult = pTokenCategory->SetId(SPCAT_RECOGNIZERS, false);
	if (FAILED(hResult)) {
		std::cerr << "Error : ISpObjectTokenCategory::SetId()" << std::endl;
		return -1;
	}

	// Create Enum Tokens Instance
	IEnumSpObjectTokens* pEnumTokens = nullptr;
	hResult = CoCreateInstance(CLSID_SpMMAudioEnum, nullptr, CLSCTX_ALL, __uuidof(IEnumSpObjectTokens), reinterpret_cast<void**>(&pEnumTokens));
	if (FAILED(hResult)) {
		std::cerr << "Error : CoCreateInstance( CLSID_SpMMAudioEnum )" << std::endl;
		return -1;
	}

	// Find Best Token
	const wchar_t* pVendorPreferred = L"VendorPreferred";
	const unsigned long lengthVendorPreferred = static_cast<unsigned long>(wcslen(pVendorPreferred));
	unsigned long length;
	ULongAdd(lengthVendorPreferred, 1, &length);
	wchar_t* pAttribsVendorPreferred = new wchar_t[length];
	StringCchCopyW(pAttribsVendorPreferred, length, pVendorPreferred);

	hResult = pTokenCategory->EnumTokens(L"Language=409;Kinect=True", pAttribsVendorPreferred, &pEnumTokens); // Japanese "Language=411;Kinect=True" English "Language=409;Kinect=True"
	if (FAILED(hResult)) {
		std::cerr << "Error : ISpObjectTokenCategory::EnumTokens()" << std::endl;
		return -1;
	}
	SafeRelease(pTokenCategory);
	delete[] pAttribsVendorPreferred;

	// Retrieved Speech Recognize Engine
	ISpObjectToken* pEngineToken = nullptr;
	hResult = pEnumTokens->Next(1, &pEngineToken, nullptr);
	if (FAILED(hResult)) {
		std::cerr << "Error : ISpObjectToken::Next()" << std::endl;
		return -1;
	}
	SafeRelease(pEnumTokens);

	// Set Speech Recognizer
	hResult = pSpeechRecognizer->SetRecognizer(pEngineToken);
	if (FAILED(hResult)) {
		std::cerr << "Error : ISpRecognizer::SetRecognizer()" << std::endl;
		return -1;
	}
	SafeRelease(pEngineToken);

	// Create Context
	//ISpRecoContext* pSpeechContext;
	hResult = pSpeechRecognizer->CreateRecoContext(&pSpeechContext);
	if (FAILED(hResult)) {
		std::cerr << "Error : ISpRecognizer::CreateRecoContext()" << std::endl;
		return -1;
	}

	// Set Property Name and Number
	hResult = pSpeechRecognizer->SetPropertyNum(L"AdaptationOn", 0);
	if (FAILED(hResult)) {
		std::cerr << "Error : ISpObjectToken::Next()" << std::endl;
		return -1;
	}

	/*** LoadSpeechGrammar ***/
	std::cout << "Load Speech Grammar" << std::endl;

	// Create Grammar
	ISpRecoGrammar* pSpeechGrammar;
	hResult = pSpeechContext->CreateGrammar(1, &pSpeechGrammar);
	if (FAILED(hResult)) {
		std::cerr << "Error : ISpRecoContext::CreateGrammar()" << std::endl;
		return -1;
	}

	// Load Grammar File (*.grxml)
	hResult = pSpeechGrammar->LoadCmdFromFile(L"grammar.grxml", SPLO_STATIC); // http://www.w3.org/TR/speech-grammar/ (UTF-8/CRLF)
	if (FAILED(hResult)) {
		std::cerr << "Error : ISpRecoGrammar::LoadCmdFromFile()" << std::endl;
		return -1;
	}

	/*** Start Speech Recognition ***/
	std::cout << "Start Speech Recognition" << std::endl;

	hResult = pSpeechGrammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);
	if (FAILED(hResult)) {
		std::cerr << "Error : ISpRecoGrammar::SetRuleState()" << std::endl;
		return -1;
	}

	hResult = pSpeechRecognizer->SetRecoState(SPRST_ACTIVE_ALWAYS);
	if (FAILED(hResult)) {
		std::cerr << "Error : ISpRecognizer::SetRecoState()" << std::endl;
		return -1;
	}

	hResult = pSpeechContext->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));
	if (FAILED(hResult)) {
		std::cerr << "Error : ISpRecoContext::SetInterest()" << std::endl;
		return -1;
	}

	hResult = pSpeechContext->Resume(0);
	if (FAILED(hResult)) {
		std::cerr << "Error : ISpRecoContext::Resume()" << std::endl;
		return -1;
	}

	hSpeechEvent = INVALID_HANDLE_VALUE;
	hSpeechEvent = pSpeechContext->GetNotifyEventHandle();
	//HANDLE hEvents[1] = { hSpeechEvent };
	return 0;

	
}
void KinectAudio::test2() {
	unsigned long waitObject = WaitForSingleObject(hSpeechEvent, 0);
	if (waitObject == WAIT_TIMEOUT)	{
		logVerbose("signaled");
	}
	else if (waitObject == WAIT_OBJECT_0){
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
					if (SUCCEEDED(hResult)) {
						if ((pPhrase->pProperties != nullptr) && (pPhrase->pProperties->pFirstChild != nullptr)) {
							// Compared with the Phrase Tag in the grammar file
							const SPPHRASEPROPERTY* pSemantic = pPhrase->pProperties->pFirstChild;
							if (pSemantic->SREngineConfidence > confidenceThreshold) {
								if (wcscmp(L"Red", pSemantic->pszValue) == 0) {
									std::cout << "Red" << std::endl;
								}
								else if (wcscmp(L"Green", pSemantic->pszValue) == 0) {
									std::cout << "Green" << std::endl;
								}
								else if (wcscmp(L"Blue", pSemantic->pszValue) == 0) {
									std::cout << "Blue" << std::endl;
								}
								else if (wcscmp(L"Exit", pSemantic->pszValue) == 0) {
									std::cout << "Exit" << std::endl;
									//exit = true;
								}
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
		return;
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
		//note this but continus things will still work
		logTrace("Kinect not found");
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
	hResult = pSpeechContext->SetNotifyWin32Event(); // needed?
	hSpeechEvent = pSpeechContext->GetNotifyEventHandle();
	hEvents[0] = hSpeechEvent;

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
	if (!hresultFails(hResult, "getAudioBeamReader")) {
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
	if (!hresultFails(hResult, "getAudioBeamReader")) {
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