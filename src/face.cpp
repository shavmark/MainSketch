#include "face.h"

namespace From2552Software {

	KinectFace::KinectFace() {
		drawface = false;
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
	void KinectFace::setup(IKinectSensor *sensor) {
		
		for (int count = 0; count < BODY_COUNT; count++) {
			// Source
			HRESULT hResult = CreateFaceFrameSource(sensor, 0, features, &pFaceSource[count]);
			if (FAILED(hResult)) {
				std::cerr << "Error : CreateFaceFrameSource" << std::endl;
				return; //bugbug add error handling
			}

			// Reader
			hResult = pFaceSource[count]->OpenReader(&pFaceReader[count]);
			if (FAILED(hResult)) {
				std::cerr << "Error : IFaceFrameSource::OpenReader()" << std::endl;
				return;
			}
		}
	}

	void KinectFace::draw()
	{
		if (drawface) {
			ofDrawCircle(facePoint[0].X, facePoint[0].Y, 5);
			ofDrawCircle(facePoint[1].X, facePoint[1].Y, 5);
			ofDrawCircle(facePoint[2].X, facePoint[2].Y, 5);
			ofDrawCircle(facePoint[3].X, facePoint[3].Y, 5);
			ofDrawCircle(facePoint[4].X, facePoint[4].Y, 5);
		}
	}
	void KinectFace::ExtractFaceRotationInDegrees(const Vector4* pQuaternion, int* pPitch, int* pYaw, int* pRoll)
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

	void KinectFace::update(IColorFrameReader*colorReader, IBodyFrameReader* bodyReader) {
		HRESULT hResult;

		//while (1) {
		// Color Frame
		// acquire frame
		IColorFrame * pColorFrame = NULL;
		if (SUCCEEDED(colorReader->AcquireLatestFrame(&pColorFrame))) {
			// Description
			IFrameDescription* pDescription;
			hResult = pColorFrame->get_FrameDescription(&pDescription);
			if (SUCCEEDED(hResult)) {
				int width = 0;
				int height = 0;
				pDescription->get_Width(&width);
				pDescription->get_Height(&height); 
				SafeRelease(pDescription);
				if (width != pixels.getWidth()) {
					//if (width != this->pixels.getWidth() || height != this->texture.getHeight()) {
						pixels.allocate(width, height, OF_PIXELS_BGRA);
					//bugbug figure out texture this->texture.allocate(this->pixels);
				}
				hResult = pColorFrame->CopyConvertedFrameDataToArray(pixels.size(), reinterpret_cast<BYTE*>(pixels.getData()), ColorImageFormat::ColorImageFormat_Bgra);
				if (!SUCCEEDED(hResult)) {
					std::cerr << "Couldn't pull pixel buffer" << std::endl;
				}
			}
			else {
				std::cerr << "Error : IColorFrameSource::get_FrameDescription()" << std::endl;
			}
		}
		SafeRelease(pColorFrame);

		// Body Frame
		//cv::Point point[BODY_COUNT];
		IBodyFrame* pBodyFrame = nullptr;
		while (1)
		{
			hResult = bodyReader->AcquireLatestFrame(&pBodyFrame);
			if (hResult != E_PENDING)
				break;
			Sleep(0);
		}
			if (SUCCEEDED(hResult)) {
				IBody* pBody[BODY_COUNT] = { 0 };
				hResult = pBodyFrame->GetAndRefreshBodyData(BODY_COUNT, pBody);
				if (SUCCEEDED(hResult)) {
					for (int count = 0; count < BODY_COUNT; count++) {
						BOOLEAN bTracked = false;
						hResult = pBody[count]->get_IsTracked(&bTracked);
						if (SUCCEEDED(hResult) && bTracked) {
							/*// Joint
							Joint joint[JointType::JointType_Count];
							hResult = pBody[count]->GetJoints( JointType::JointType_Count, joint );
							if( SUCCEEDED( hResult ) ){
							for( int type = 0; type < JointType::JointType_Count; type++ ){
							ColorSpacePoint colorSpacePoint = { 0 };
							pCoordinateMapper->MapCameraPointToColorSpace( joint[type].Position, &colorSpacePoint );
							int x = static_cast<int>( colorSpacePoint.X );
							int y = static_cast<int>( colorSpacePoint.Y );
							if( ( x >= 0 ) && ( x < width ) && ( y >= 0 ) && ( y < height ) ){
							cv::circle( bufferMat, cv::Point( x, y ), 5, static_cast<cv::Scalar>( color[count] ), -1, CV_AA );
							}
							}
							}*/

							// Set TrackingID to Detect Face
							UINT64 trackingId = _UI64_MAX;
							hResult = pBody[count]->get_TrackingId(&trackingId);
							if (SUCCEEDED(hResult)) {
								pFaceSource[count]->put_TrackingId(trackingId);
							}
						}
					}
				}
				for (int count = 0; count < BODY_COUNT; count++) {
					SafeRelease(pBody[count]);
				}
				//break;
			}
		//}
		SafeRelease(pBodyFrame);

		// Face Frame
		//std::system("cls");
		for (int count = 0; count < BODY_COUNT; count++) {
			IFaceFrame* pFaceFrame = nullptr;
			hResult = pFaceReader[count]->AcquireLatestFrame(&pFaceFrame);
			if (SUCCEEDED(hResult) && pFaceFrame != nullptr) {
				BOOLEAN bFaceTracked = false;
				hResult = pFaceFrame->get_IsTrackingIdValid(&bFaceTracked);
				
				if (SUCCEEDED(hResult) && bFaceTracked) {
					IFaceFrameResult* pFaceResult = nullptr;
					hResult = pFaceFrame->get_FaceFrameResult(&pFaceResult);
					if (SUCCEEDED(hResult) && pFaceResult != nullptr) {
						std::vector<std::string> result;

						// Face Point
						hResult = pFaceResult->GetFacePointsInColorSpace(FacePointType::FacePointType_Count, facePoint);
						if (SUCCEEDED(hResult)) {
							drawface = true;
							/*
							cv::circle(bufferMat, cv::Point(static_cast<int>(facePoint[0].X), static_cast<int>(facePoint[0].Y)), 5, static_cast<cv::Scalar>(color[count]), -1, CV_AA); // Eye (Left)
							cv::circle(bufferMat, cv::Point(static_cast<int>(facePoint[1].X), static_cast<int>(facePoint[1].Y)), 5, static_cast<cv::Scalar>(color[count]), -1, CV_AA); // Eye (Right)
							cv::circle(bufferMat, cv::Point(static_cast<int>(facePoint[2].X), static_cast<int>(facePoint[2].Y)), 5, static_cast<cv::Scalar>(color[count]), -1, CV_AA); // Nose
							cv::circle(bufferMat, cv::Point(static_cast<int>(facePoint[3].X), static_cast<int>(facePoint[3].Y)), 5, static_cast<cv::Scalar>(color[count]), -1, CV_AA); // Mouth (Left)
							cv::circle(bufferMat, cv::Point(static_cast<int>(facePoint[4].X), static_cast<int>(facePoint[4].Y)), 5, static_cast<cv::Scalar>(color[count]), -1, CV_AA); // Mouth (Right)
							*/
						}

						// Face Bounding Box test
						RectI boundingBox;
						hResult = pFaceResult->get_FaceBoundingBoxInColorSpace(&boundingBox);
						if (SUCCEEDED(hResult)) {
							//cv::rectangle(bufferMat, cv::Rect(boundingBox.Left, boundingBox.Top, boundingBox.Right - boundingBox.Left, boundingBox.Bottom - boundingBox.Top), static_cast<cv::Scalar>(color[count]));
						}

						// Face Rotation
						Vector4 faceRotation;
						hResult = pFaceResult->get_FaceRotationQuaternion(&faceRotation);
						if (SUCCEEDED(hResult)) {
							int pitch, yaw, roll;
							ExtractFaceRotationInDegrees(&faceRotation, &pitch, &yaw, &roll);
							result.push_back("Pitch, Yaw, Roll : " + std::to_string(pitch) + ", " + std::to_string(yaw) + ", " + std::to_string(roll));
						}

						// Face Property
						DetectionResult faceProperty[FaceProperty::FaceProperty_Count];
						hResult = pFaceResult->GetFaceProperties(FaceProperty::FaceProperty_Count, faceProperty);
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
						}

						if (boundingBox.Left && boundingBox.Bottom) {
							int offset = 30;
							for (std::vector<std::string>::iterator it = result.begin(); it != result.end(); it++, offset += 30) {
								//cv::putText(bufferMat, *it, cv::Point(boundingBox.Left, boundingBox.Bottom + offset), cv::FONT_HERSHEY_COMPLEX, 1.0f, static_cast<cv::Scalar>(color[count]), 2, CV_AA);
							}
						}
					}
					SafeRelease(pFaceResult);
				}
			}
			SafeRelease(pFaceFrame);
		}

	}


}
