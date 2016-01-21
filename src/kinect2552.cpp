#include "kinect2552.h"

namespace From2552Software {

	void Kinect2552BaseClass::setup(Kinect2552 *pKinectIn) {
		pKinect = pKinectIn;
		valid = false;
#if _DEBUG
		pCoordinateMapper = nullptr;
		pDescription = nullptr;
		pBodyReader = nullptr;
		pColorReader = nullptr;
		pBodySource = nullptr;
		pColorSource = nullptr;
		pSensor = nullptr;
		readfacereaders = false;
#endif
	};
	
	void KinectBodies::setup(Kinect2552 *kinectInput) {

		Kinect2552BaseClass::setup(kinectInput);

		for (int i = 0; i < BODY_COUNT; ++i) {
			KinectBody body(getKinect());
			bodies.push_back(body);
		}
	}

	void KinectBodies::draw() {

		ofBackground(0);
		ofSetColor(0, 0, 255);
		ofFill();

		for (auto body : bodies) {
			ofDrawCircle(400, 100, 30);
			if (body.ObjectValid()) {
				ofDrawCircle(600, 100, 30);

				ColorSpacePoint colorSpacePoint = { 0 };
				ofDrawCircle(400, 100, 30);
				HRESULT hResult = getKinect()->getCoordinateMapper()->MapCameraPointToColorSpace(body.joints[JointType::JointType_HandLeft].Position, &colorSpacePoint);
				// fails here
				if (SUCCEEDED(hResult)) {
					ofDrawCircle(700, 100, 30);
					int x = static_cast<int>(colorSpacePoint.X);
					int y = static_cast<int>(colorSpacePoint.Y);
					if ((x >= 0) && (x < getKinect()->width) && (y >= 0) && (y < getKinect()->height)) {
						if (body.leftHandState == HandState::HandState_Open) {
							ofDrawCircle(x, y, 50);
						}
						else if (body.leftHandState == HandState::HandState_Closed) {
							ofDrawCircle(x, y, 5);
						}
						else if (body.leftHandState == HandState::HandState_Lasso) {
							ofDrawCircle(x, y, 25);
						}
					}
				}
				colorSpacePoint = { 0 };
				hResult = getKinect()->getCoordinateMapper()->MapCameraPointToColorSpace(body.joints[JointType::JointType_HandRight].Position, &colorSpacePoint);
				if (SUCCEEDED(hResult)) {
					int x = static_cast<int>(colorSpacePoint.X);
					int y = static_cast<int>(colorSpacePoint.Y);
					if ((x >= 0) && (x < getKinect()->width) && (y >= 0) && (y < getKinect()->height)) {
						if (body.rightHandState == HandState::HandState_Open) {
							ofDrawCircle(x, y, 50);
						}
						else if (body.rightHandState == HandState::HandState_Closed) {
							ofDrawCircle(x, y, 5);
						}
						else if (body.rightHandState == HandState::HandState_Lasso) {
							ofDrawCircle(x, y, 25);
						}
					}
				}
				// Joint
				for (int type = 0; type < JointType::JointType_Count; type++) {
					colorSpacePoint = { 0 };
					getKinect()->getCoordinateMapper()->MapCameraPointToColorSpace(body.joints[type].Position, &colorSpacePoint);
					int x = static_cast<int>(colorSpacePoint.X);
					int y = static_cast<int>(colorSpacePoint.Y);
					if ((x >= 0) && (x < getKinect()->width) && (y >= 0) && (y < getKinect()->height)) {
						//cv::circle(bufferMat, cv::Point(x, y), 5, static_cast<cv::Scalar>(color[count]), -1, CV_AA);
						ofDrawCircle(x, y, 100);
					}
				}


			}
		}
	}

	void KinectBodies::update() {


		//works here bodies[0].SetValid();  return;

		IBodyFrame* pBodyFrame = nullptr;
		HRESULT hResult = getKinect()->getBodyReader()->AcquireLatestFrame(&pBodyFrame);
		if (SUCCEEDED(hResult)) {
			IBody* pBody[BODY_COUNT] = { 0 };
			//breaks here

			hResult = pBodyFrame->GetAndRefreshBodyData(BODY_COUNT, pBody);
			if (SUCCEEDED(hResult)) {
				for (int count = 0; count < BODY_COUNT; count++) {
					bodies[count].SetValid(false);
					// breaks here
					BOOLEAN bTracked = false;
					hResult = pBody[count]->get_IsTracked(&bTracked);
					if (SUCCEEDED(hResult) && bTracked) {
						// Set TrackingID to Detect Face
						// LEFT OFF HERE
						UINT64 trackingId = _UI64_MAX;
						hResult = pBody[count]->get_TrackingId(&trackingId);
						if (SUCCEEDED(hResult)) {
							//faces[count].pFaceSource->put_TrackingId(trackingId);
						}
						// failing here
						// get joints
						hResult = pBody[count]->GetJoints(JointType::JointType_Count, bodies[count].joints);
						if (SUCCEEDED(hResult)) {
							// Left Hand State
							hResult = pBody[count]->get_HandLeftState(&bodies[count].leftHandState);
							if (SUCCEEDED(hResult)) {
							}

							// Right Hand State
							hResult = pBody[count]->get_HandRightState(&bodies[count].rightHandState);
							if (SUCCEEDED(hResult)) {
							}
							// Lean
							hResult = pBody[count]->get_Lean(&bodies[count].leanAmount);
							if (SUCCEEDED(hResult)) {
							}
							bodies[count].SetValid();
						}

						/*// Activity
						IColorFrame* pColorFrame = nullptr;
						HRESULT hResult = getKinect()->pColorReader->AcquireLatestFrame(&pColorFrame);
						if (SUCCEEDED(hResult)) {
						ofPixels pixels;
						pixels.allocate(getKinect()->width, getKinect()->height, OF_PIXELS_BGRA);
						hResult = pColorFrame->CopyConvertedFrameDataToArray(getKinect()->bufferSize, reinterpret_cast<BYTE*>(pixels.getData()), ColorImageFormat::ColorImageFormat_Bgra);
						if (SUCCEEDED(hResult)) {
						}
						}
						UINT capacity = 0;
						DetectionResult detectionResults = DetectionResult::DetectionResult_Unknown;
						hResult = pBody[count]->GetActivityDetectionResults( capacity, &detectionResults );
						if( SUCCEEDED( hResult ) ){
						if( detectionResults == DetectionResult::DetectionResult_Yes ){
						switch( capacity ){
						case Activity::Activity_EyeLeftClosed:
						std::cout << "Activity_EyeLeftClosed" << std::endl;
						break;
						case Activity::Activity_EyeRightClosed:
						std::cout << "Activity_EyeRightClosed" << std::endl;
						break;
						case Activity::Activity_MouthOpen:
						std::cout << "Activity_MouthOpen" << std::endl;
						break;
						case Activity::Activity_MouthMoved:
						std::cout << "Activity_MouthMoved" << std::endl;
						break;
						case Activity::Activity_LookingAway:
						std::cout << "Activity_LookingAway" << std::endl;
						break;
						default:
						break;
						}
						}
						}
						else{
						std::cerr << "Error : IBody::GetActivityDetectionResults()" << std::endl;
						}*/

						/*// Appearance
						capacity = 0;
						detectionResults = DetectionResult::DetectionResult_Unknown;
						hResult = pBody[count]->GetAppearanceDetectionResults( capacity, &detectionResults );
						if( SUCCEEDED( hResult ) ){
						if( detectionResults == DetectionResult::DetectionResult_Yes ){
						switch( capacity ){
						case Appearance::Appearance_WearingGlasses:
						std::cout << "Appearance_WearingGlasses" << std::endl;
						break;
						default:
						break;
						}
						}
						}
						else{
						std::cerr << "Error : IBody::GetAppearanceDetectionResults()" << std::endl;
						}*/

						/*// Expression
						capacity = 0;
						detectionResults = DetectionResult::DetectionResult_Unknown;
						hResult = pBody[count]->GetExpressionDetectionResults( capacity, &detectionResults );
						if( SUCCEEDED( hResult ) ){
						if( detectionResults == DetectionResult::DetectionResult_Yes ){
						switch( capacity ){
						case Expression::Expression_Happy:
						std::cout << "Expression_Happy" << std::endl;
						break;
						case Expression::Expression_Neutral:
						std::cout << "Expression_Neutral" << std::endl;
						break;
						default:
						break;
						}
						}
						}
						else{
						std::cerr << "Error : IBody::GetExpressionDetectionResults()" << std::endl;
						}*/

					}
				}
				//cv::resize(bufferMat, bodyMat, cv::Size(), 0.5, 0.5);
			}
			for (int count = 0; count < BODY_COUNT; count++) {
				SafeRelease(pBody[count]);
			}
		}

		//SafeRelease(pColorFrame);
		SafeRelease(pBodyFrame);
	}


	void Kinect2552::open()
	{
		
		HRESULT hResult = GetDefaultKinectSensor(&pSensor);
		if (FAILED(hResult)) {
			std::cerr << "Error : GetDefaultKinectSensor" << std::endl;
			return; //bugbug add error/exception handling
		}

		hResult = pSensor->Open();
		if (FAILED(hResult)) {
			std::cerr << "Error : IKinectSensor::Open()" << std::endl;
		}
		
		hResult = pSensor->get_ColorFrameSource(&pColorSource);
		if (FAILED(hResult)) {
			std::cerr << "Error : IKinectSensor::get_ColorFrameSource()" << std::endl;
			return;
		}
		
		hResult = pSensor->get_BodyFrameSource(&pBodySource);
		if (FAILED(hResult)) {
			std::cerr << "Error : IKinectSensor::get_BodyFrameSource()" << std::endl;
			return;
		}

		hResult = pColorSource->OpenReader(&pColorReader);
		if (FAILED(hResult)) {
			std::cerr << "Error : IColorFrameSource::OpenReader()" << std::endl;
			return;
		}
		
		hResult = pBodySource->OpenReader(&pBodyReader);
		if (FAILED(hResult)) {
			std::cerr << "Error : IBodyFrameSource::OpenReader()" << std::endl;
			return;
		}

		hResult = pColorSource->get_FrameDescription(&pDescription);
		if (FAILED(hResult)) {
			std::cerr << "Error : IColorFrameSource::get_FrameDescription()" << std::endl;
			return;
		}

		pDescription->get_Width(&width); // 1920
		pDescription->get_Height(&height); // 1080
		bufferSize = width * height * 4 * sizeof(unsigned char);
		//ofSetWindowShape(width, height);

		hResult = pSensor->get_CoordinateMapper(&pCoordinateMapper);
		if (FAILED(hResult)) {
			std::cerr << "Error : IKinectSensor::get_CoordinateMapper()" << std::endl;
			return;
		}

		// Color Table, gives each body its own color
		colors.push_back(ofColor(255, 0, 0));
		colors.push_back(ofColor(0, 0, 255));
		colors.push_back(ofColor(255, 255, 0));
		colors.push_back(ofColor(255, 0, 255));
		colors.push_back(ofColor(0, 255, 255));
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

	// Face Property Table
	property[0] = "Happy";
	property[1] = "Engaged";
	property[2] = "WearingGlasses";
	property[3] = "LeftEyeClosed";
	property[4] = "RightEyeClosed";
	property[5] = "MouthOpen";
	property[6] = "MouthMoved";
	property[7] = "LookingAway";


};


// get the face readers
void KinectFaces::setup(Kinect2552 *kinectInput) {

	Kinect2552BaseClass::setup(kinectInput);

	for (int i = 0; i < BODY_COUNT; ++i) {
		KinectFace face(getKinect());

		HRESULT hResult = CreateFaceFrameSource(getKinect()->getSensor(), 0, features, &face.pFaceSource);
		if (FAILED(hResult)) {
			std::cerr << "Error : CreateFaceFrameSource" << std::endl;
			return; //bugbug add error handling
		}

		hResult = face.pFaceSource->OpenReader(&face.pFaceReader);
		if (FAILED(hResult)) {
			std::cerr << "Error : IFaceFrameSource::OpenReader()" << std::endl;
			return;
		}
		faces.push_back(face);
	}

}

void KinectFaces::draw()
{
	ofBackground(0);
	ofSetColor(0, 0, 255);
	ofFill();
	//ofDrawCircle(400, 100, 30);

	for (int count = 0; count < BODY_COUNT; count++) {
		if (faces[count].ObjectValid()) {
			ofDrawCircle(400, 100, 30);
			if (faces[count].faceProperty[FaceProperty_LeftEyeClosed] != DetectionResult_Yes)
			{
				ofDrawCircle(faces[count].leftEye().X, faces[count].leftEye().Y, 5);
			}
			if (faces[count].faceProperty[FaceProperty_RightEyeClosed] != DetectionResult_Yes)
			{
				ofDrawCircle(faces[count].rightEye().X, faces[count].rightEye().Y, 5);
			}
			ofDrawCircle(faces[count].nose().X, faces[count].nose().Y, 5);
			float width = abs(faces[count].mouthCornerRight().X - faces[count].mouthCornerLeft().X);
			float height;
			if (faces[count].faceProperty[FaceProperty_MouthOpen] == DetectionResult_Yes)
			{
				height = 50.0;
			}
			else
			{
				height = 1.0;
			}

			ofDrawEllipse(faces[count].mouthCornerLeft().X, faces[count].mouthCornerLeft().Y, width, height);

		}
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

void  Kinect2552BaseClass::aquireBodyFrame()
{

	IBodyFrame* pBodyFrame = nullptr;
	HRESULT hResult = getKinect()->getBodyReader()->AcquireLatestFrame(&pBodyFrame); // getKinect()->getBodyReader() was pBodyReader
	if (SUCCEEDED(hResult)) {
		IBody* pBody[BODY_COUNT] = { 0 };
		hResult = pBodyFrame->GetAndRefreshBodyData(BODY_COUNT, pBody);
		if (SUCCEEDED(hResult)) {
			for (int count = 0; count < BODY_COUNT; count++) {
				BOOLEAN bTracked = false;
				hResult = pBody[count]->get_IsTracked(&bTracked);
				if (SUCCEEDED(hResult) && bTracked) {

					// Set TrackingID to Detect Face etc
					UINT64 trackingId = _UI64_MAX;
					hResult = pBody[count]->get_TrackingId(&trackingId);
					if (SUCCEEDED(hResult)) {
						setTrackingID(count, trackingId);
					}
				}
			}
		}
		for (int count = 0; count < BODY_COUNT; count++) {
			SafeRelease(pBody[count]);
		}
	}
	SafeRelease(pBodyFrame);
}
// add faces to the bodies
void KinectFaces::update() {
	
	while (1) {
		aquireBodyFrame();
		if (aquireFaceFrame())
			break;
	}
#if findthebug
	// loop may take some time at first call while kinect warms up
	while (1) {

		IBodyFrame* pBodyFrame = nullptr;
		HRESULT hResult = getKinect()->getBodyReader()->AcquireLatestFrame(&pBodyFrame);
		if (SUCCEEDED(hResult)) {
			IBody* pBody[BODY_COUNT] = { 0 };
			hResult = pBodyFrame->GetAndRefreshBodyData(BODY_COUNT, pBody);
			if (SUCCEEDED(hResult)) {
				for (int count = 0; count < BODY_COUNT; count++) {
					BOOLEAN bTracked = false;
					hResult = pBody[count]->get_IsTracked(&bTracked);
					if (SUCCEEDED(hResult) && bTracked) {
						// Set TrackingID to Detect Face
						UINT64 trackingId = _UI64_MAX;
						hResult = pBody[count]->get_TrackingId(&trackingId);
						if (SUCCEEDED(hResult)) {
							faces[count].pFaceSource->put_TrackingId(trackingId);
						}
					}
				}
			}
			for (int count = 0; count < BODY_COUNT; count++) {
				SafeRelease(pBody[count]);
			}
		}

		SafeRelease(pBodyFrame);
		// Face Frame
		for (int count = 0; count < BODY_COUNT; count++) {
			IFaceFrame* pFaceFrame = nullptr;
			faces[count].SetValid(false);
			hResult = faces[count].getFaceReader()->AcquireLatestFrame(&pFaceFrame);
			if (SUCCEEDED(hResult) && pFaceFrame != nullptr) {
				BOOLEAN bFaceTracked = false;
				hResult = pFaceFrame->get_IsTrackingIdValid(&bFaceTracked);
				if (SUCCEEDED(hResult) && bFaceTracked) {
					IFaceFrameResult* pFaceResult = nullptr;
					hResult = pFaceFrame->get_FaceFrameResult(&pFaceResult);
					if (SUCCEEDED(hResult) && pFaceResult != nullptr) {
						hResult = pFaceResult->GetFacePointsInColorSpace(FacePointType::FacePointType_Count, faces[count].facePoint);
						if (SUCCEEDED(hResult)) {
						}

						hResult = pFaceResult->get_FaceRotationQuaternion(&faces[count].faceRotation);
						if (SUCCEEDED(hResult)) {
						}

						hResult = pFaceResult->GetFaceProperties(FaceProperty::FaceProperty_Count, faces[count].faceProperty);
						if (SUCCEEDED(hResult)) {
						}

						hResult = pFaceResult->get_FaceBoundingBoxInColorSpace(&faces[count].boundingBox);
						if (SUCCEEDED(hResult)) {
							faces[count].SetValid();
							SafeRelease(pFaceResult);
							SafeRelease(pFaceFrame);
							return;
						}

						//if (boundingBox.Left && boundingBox.Bottom) {
						//int offset = 30;
						//for (std::vector<std::string>::iterator it = result.begin(); it != result.end(); it++, offset += 30) {
						//		cv::putText(bufferMat, *it, cv::Point(boundingBox.Left, boundingBox.Bottom + offset), cv::FONT_HERSHEY_COMPLEX, 1.0f, static_cast<cv::Scalar>(color[count]), 2, CV_AA);
						//}
						//}
					}
					SafeRelease(pFaceResult);
				}
			}
			SafeRelease(pFaceFrame);
		}
		return;
	}
#endif

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

	for (int count = 0; count < BODY_COUNT; count++) {
		IFaceFrame* pFaceFrame = nullptr;
		HRESULT hResult = faces[count].getFaceReader()->AcquireLatestFrame(&pFaceFrame); // faces[count].getFaceReader() was pFaceReader[count]
		if (SUCCEEDED(hResult) && pFaceFrame != nullptr) {
			BOOLEAN bFaceTracked = false;
			hResult = pFaceFrame->get_IsTrackingIdValid(&bFaceTracked);
			if (SUCCEEDED(hResult) && bFaceTracked) {
				IFaceFrameResult* pFaceResult = nullptr;
				hResult = pFaceFrame->get_FaceFrameResult(&pFaceResult);
				if (SUCCEEDED(hResult) && pFaceResult != nullptr) {

					// Face Point
					hResult = pFaceResult->GetFacePointsInColorSpace(FacePointType::FacePointType_Count, faces[count].facePoint);
					if (SUCCEEDED(hResult)) {
					}

					hResult = pFaceResult->get_FaceRotationQuaternion(&faces[count].faceRotation);
					if (SUCCEEDED(hResult)) {
					}

					hResult = pFaceResult->GetFaceProperties(FaceProperty::FaceProperty_Count, faces[count].faceProperty);
					if (SUCCEEDED(hResult)) {
					}

					// Face Bounding Box
					hResult = pFaceResult->get_FaceBoundingBoxInColorSpace(&faces[count].boundingBox);
					if (SUCCEEDED(hResult)) {
						SafeRelease(pFaceResult);
						SafeRelease(pFaceFrame);
						faces[count].SetValid();
						return true;
					}

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
	if (false && pSensor == nullptr) // was pSensor
	{
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
		hResult = pSensor->get_ColorFrameSource(&pColorSource);
		if (FAILED(hResult)) {
			std::cerr << "Error : IKinectSensor::get_ColorFrameSource()" << std::endl;
			return -1;
		}

		hResult = pSensor->get_BodyFrameSource(&pBodySource);
		if (FAILED(hResult)) {
			std::cerr << "Error : IKinectSensor::get_BodyFrameSource()" << std::endl;
			return -1;
		}

		// Reader
		hResult = pColorSource->OpenReader(&pColorReader);
		if (FAILED(hResult)) {
			std::cerr << "Error : IColorFrameSource::OpenReader()" << std::endl;
			return -1;
		}

		hResult = pBodySource->OpenReader(&pBodyReader);
		if (FAILED(hResult)) {
			std::cerr << "Error : IBodyFrameSource::OpenReader()" << std::endl;
			return -1;
		}

		// Description
		hResult = pColorSource->get_FrameDescription(&pDescription);
		if (FAILED(hResult)) {
			std::cerr << "Error : IColorFrameSource::get_FrameDescription()" << std::endl;
			return -1;
		}

		int width = 0;
		int height = 0;
		pDescription->get_Width(&width); // 1920
		pDescription->get_Height(&height); // 1080
		unsigned int bufferSize = width * height * 4 * sizeof(unsigned char);


		// Coordinate Mapper
		hResult = pSensor->get_CoordinateMapper(&pCoordinateMapper);
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

		for (int count = 0; count < BODY_COUNT; count++) {
			KinectFace face(getKinect());
			// Source
			hResult = CreateFaceFrameSource(pSensor, 0, features, &pFaceSource[count]);
			if (FAILED(hResult)) {
				std::cerr << "Error : CreateFaceFrameSource" << std::endl;
				return -1;
			}

			// Reader
			hResult = pFaceSource[count]->OpenReader(&pFaceReader[count]);
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

}