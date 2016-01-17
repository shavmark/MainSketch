#include "ofApp.h"
#include "utils.h"
#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
int previewWidth = 640;
int previewHeight = 480;

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


template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL) {
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}
inline void ExtractFaceRotationInDegrees(const Vector4* pQuaternion, int* pPitch, int* pYaw, int* pRoll)
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
void ofApp::initFace() {
	for (int count = 0; count < BODY_COUNT; count++) {
		// Source
		HRESULT hResult = CreateFaceFrameSource(kinect.getSensor(), 0, features, &pFaceSource[count]);
		if (FAILED(hResult)) {
			std::cerr << "Error : CreateFaceFrameSource" << std::endl;
			return;
		}

		// Reader
		hResult = pFaceSource[count]->OpenReader(&pFaceReader[count]);
		if (FAILED(hResult)) {
			std::cerr << "Error : IFaceFrameSource::OpenReader()" << std::endl;
			return;
		}
	}
	// Face Property Table
	property[0] = "Happy";
	property[1] = "Engaged";
	property[2] = "WearingGlasses";
	property[3] = "LeftEyeClosed";
	property[4] = "RightEyeClosed";
	property[5] = "MouthOpen";
	property[6] = "MouthMoved";
	property[7] = "LookingAway";
}
//--------------------------------------------------------------
void ofApp::setup(){
	kinect.open();
	kinect.initDepthSource();
	kinect.initColorSource();
	kinect.initInfraredSource();
	kinect.initBodySource();
	kinect.initBodyIndexSource();
	initFace();
	ofSetWindowShape(previewWidth * 2, previewHeight * 2);
}

//--------------------------------------------------------------
void ofApp::update(){
	kinect.update();

	//while (1) {
		// Color Frame
		// acquire frame
		IColorFrame * pColorFrame = NULL;
		if (SUCCEEDED(kinect.getColorSource()->getReader()->AcquireLatestFrame(&pColorFrame))) {
			if (!SUCCEEDED(pColorFrame->CopyConvertedFrameDataToArray(kinect.getColorSource()->getPixels().size(), kinect.getColorSource()->getPixels(), ColorImageFormat_Bgra))) {
				std::cerr << "Couldn't pull pixel buffer" << std::endl;
			}
		}
		SafeRelease(pColorFrame);

		// Body Frame
		//cv::Point point[BODY_COUNT];
		IBodyFrame* pBodyFrame = nullptr;
		HRESULT hResult;
		while (1)
		{
			hResult = kinect.getBodySource()->getReader()->AcquireLatestFrame(&pBodyFrame);
			if (hResult == E_PENDING)
				continue;
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
				break;
			}
		}
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
						PointF facePoint[FacePointType::FacePointType_Count];
						hResult = pFaceResult->GetFacePointsInColorSpace(FacePointType::FacePointType_Count, facePoint);
						if (SUCCEEDED(hResult)) {
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
	

	//--
	//Getting joint positions (skeleton tracking)
	//--
	//
	{
		auto bodies = kinect.getBodySource()->getBodies();
		for (auto body : bodies) {
			for (auto joint : body.joints) {
				//now do something with the joints
			}
		}
	}
	//
	//--



	//--
	//Getting bones (connected joints)
	//--
	//
	{
		// Note that for this we need a reference of which joints are connected to each other.
		// We call this the 'boneAtlas', and you can ask for a reference to this atlas whenever you like
		auto bodies = kinect.getBodySource()->getBodies();
		auto boneAtlas = ofxKinectForWindows2::Data::Body::getBonesAtlas();

		for (auto body : bodies) {
			for (auto bone : boneAtlas) {
				auto firstJointInBone = body.joints[bone.first];
				auto secondJointInBone = body.joints[bone.second];

				//now do something with the joints
			}
		}
	}
	//
	//--
}

//--------------------------------------------------------------
void ofApp::draw(){
	kinect.getDepthSource()->draw(0, 0, previewWidth, previewHeight);  // note that the depth texture is RAW so may appear dark

																	   // Color is at 1920x1080 instead of 512x424 so we should fix aspect ratio
	float colorHeight = previewWidth * (kinect.getColorSource()->getHeight() / kinect.getColorSource()->getWidth());
	float colorTop = (previewHeight - colorHeight) / 2.0;

	kinect.getColorSource()->draw(previewWidth, 0 + colorTop, previewWidth, colorHeight);
	kinect.getBodySource()->drawProjected(previewWidth, 0 + colorTop, previewWidth, colorHeight);

	kinect.getInfraredSource()->draw(0, previewHeight, previewWidth, previewHeight);

	kinect.getBodyIndexSource()->draw(previewWidth, previewHeight, previewWidth, previewHeight);
	kinect.getBodySource()->drawProjected(previewWidth, previewHeight, previewWidth, previewHeight, ofxKFW2::ProjectionCoordinates::DepthCamera);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
