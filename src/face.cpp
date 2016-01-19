#include "face.h"

namespace From2552Software {

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
	void KinectFaces::setup(IKinectSensor *sensor) {
		
		// we need a unique ID for bodies at this point, we are assuming that its an index but this could change, should be done in ofKinectLib maybe, not sure
		for (int i = 0; i < BODY_COUNT; ++i) {
			KinectFace face(i);

			HRESULT hResult = CreateFaceFrameSource(sensor, face.faceId, features, &face.pFaceSource);
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
		
		for (auto face : faces) {
			if (face.valid) {
				ofDrawCircle(400, 100, 30);
				// testing ofDrawCircle(400, 100, 30);
				if (face.faceProperty[FaceProperty_LeftEyeClosed] != DetectionResult_Yes)
				{
					ofDrawCircle(face.leftEye().X, face.leftEye().Y, 5);
				}
				ofDrawCircle(face.rightEye().X, face.rightEye().Y, 5);
				ofDrawCircle(face.nose().X, face.nose().Y, 5);
				float width = abs(face.mouthCornerRight().X - face.mouthCornerLeft().X);
				float height;
				if (face.faceProperty[FaceProperty_MouthOpen] != DetectionResult_No)
				{
					height = 20.0;
				}
				else
				{
					height = 5.0;
				}

				ofDrawEllipse(face.mouthCornerLeft().X, face.mouthCornerLeft().Y, width, height);

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

	// add faces to the bodies
	void KinectFaces::update(IBodyFrameReader*pBodyReader) {

		// Body Frame
		
		IBodyFrame* pBodyFrame = nullptr;
		HRESULT hResult = pBodyReader->AcquireLatestFrame(&pBodyFrame);
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
							//pFaceSource[count]->put_TrackingId(trackingId);
						}
					}
				}
			}
			for (int count = 0; count < BODY_COUNT; count++) {
				Kinect::SafeRelease(pBody[count]);
			}
		}
		Kinect::SafeRelease(pBodyFrame);

		// Face Frame
		for (int count = 0; count < BODY_COUNT; count++) {
			IFaceFrame* pFaceFrame = nullptr;
			hResult = faces[count].pFaceReader->AcquireLatestFrame(&pFaceFrame);
			if (SUCCEEDED(hResult) && pFaceFrame != nullptr) {
				BOOLEAN bFaceTracked = false;
				hResult = pFaceFrame->get_IsTrackingIdValid(&bFaceTracked);
				if (SUCCEEDED(hResult) && bFaceTracked) {
					IFaceFrameResult* pFaceResult = nullptr;
					hResult = pFaceFrame->get_FaceFrameResult(&pFaceResult);
					if (SUCCEEDED(hResult) && pFaceResult != nullptr) {
						hResult = pFaceResult->GetFacePointsInColorSpace(FacePointType::FacePointType_Count, faces[count].facePoint);
						// bugbug add error handling

						hResult = pFaceResult->GetFacePointsInColorSpace(FacePointType::FacePointType_Count, faces[count].facePoint);
						if (SUCCEEDED(hResult)) {
						}

						hResult = pFaceResult->get_FaceBoundingBoxInColorSpace(&faces[count].boundingBox);
						if (SUCCEEDED(hResult)) {
						}

						hResult = pFaceResult->get_FaceRotationQuaternion(&faces[count].faceRotation);
						if (SUCCEEDED(hResult)) {
						}

						hResult = pFaceResult->GetFaceProperties(FaceProperty::FaceProperty_Count, faces[count].faceProperty);
						if (SUCCEEDED(hResult)) {
						}

						faces[count].valid = true;

						//if (boundingBox.Left && boundingBox.Bottom) {
							//int offset = 30;
							//for (std::vector<std::string>::iterator it = result.begin(); it != result.end(); it++, offset += 30) {
						//		cv::putText(bufferMat, *it, cv::Point(boundingBox.Left, boundingBox.Bottom + offset), cv::FONT_HERSHEY_COMPLEX, 1.0f, static_cast<cv::Scalar>(color[count]), 2, CV_AA);
							//}
						//}
					}
					Kinect::SafeRelease(pFaceResult);
				}
			}
			Kinect::SafeRelease(pFaceFrame);
		}

#if old
		// keep bodies and faces in sync w/o changing the shared bodies lib
		for (auto body : bodies) {
			if (body.tracked) {
				// see if the body maps to a face
				for (std::vector<KinectFace>::iterator face = faces.begin(); face != faces.end(); ++face)
				{
					if ((*face).bodyId == body.bodyId) { // each body gets a face, any face would do just so it does not belong to any other body
						(*face).pFaceSource->put_TrackingId(body.trackingId); // make sure these are in sync
					}
					(*face).valid = false;
					IFaceFrame* pFaceFrame = nullptr;
					HRESULT hResult = (*face).pFaceReader->AcquireLatestFrame(&pFaceFrame); // try every frame
					if (SUCCEEDED(hResult) && pFaceFrame != nullptr) { //only gets past here if data is read, there fore AcquireLatestFrame us the bad guy
						BOOLEAN bFaceTracked = false;
						hResult = pFaceFrame->get_IsTrackingIdValid(&bFaceTracked);
						if (SUCCEEDED(hResult) && bFaceTracked) {
							IFaceFrameResult* pFaceResult = nullptr;
							hResult = pFaceFrame->get_FaceFrameResult(&pFaceResult);
							if (SUCCEEDED(hResult) && pFaceResult != nullptr) {
								// Face Point bugbug all these need some sort of error handling and safe state so data is not accessed on an error
								hResult = pFaceResult->GetFacePointsInColorSpace(FacePointType::FacePointType_Count, (*face).facePoint);
								// bugbug add error handling
								hResult = pFaceResult->get_FaceBoundingBoxInColorSpace(&(*face).boundingBox);
								hResult = pFaceResult->get_FaceRotationQuaternion(&(*face).faceRotation);
								hResult = pFaceResult->GetFaceProperties(FaceProperty::FaceProperty_Count, (*face).faceProperty);
								(*face).valid = true;
							}
							SafeRelease(pFaceResult);
						}
					}
					SafeRelease(pFaceFrame);
				}
			}
		}
#endif
	}

	void KinectFaces::drawProjected(int x, int y, int width, int height, ofxKFW2::ProjectionCoordinates proj) {
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
}
