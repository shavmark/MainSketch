#include "ofApp.h"
#include "2552software.h"

// follow this https://github.com/openframeworks/openFrameworks/wiki/oF-code-style, but let them w/o bugs cast the first flames

//http://openframeworks.cc/ofBook/chapters/c++11.html\

//http://openframeworks.cc/tutorials/graphics/generativemesh.html

//https://github.com/Kinect/Docs

//--------------------------------------------------------------
void ofApp::setup(){
	
	ofSetFrameRate(30);

	ofBackground(50, 0);

	ofDisableArbTex(); // we need GL_TEXTURE_2D for our models coords.

	bAnimate = true;
	bAnimateMouse = false;
	animationPosition = 0;
	//http://ogldev.atspace.co.uk/www/tutorial38/tutorial38.html
	// from http://www.meshfactory.com/shop/catalog/free05.php
	// 3ds max and unreal game engine work 
	//http://mathinfo.univ-reims.fr/image/dxMesh/extra/d3dx_skinnedmesh.pdf
	// works well model.loadModel("C:\\Users\\mark\\Documents\\iclone\\01.Chuck_Base2.fbx", true);
	// obj draws perfect but no animation, fbx does animation but is easy to break drawing
	/*
	sa.Init(model.getAssimpScene());
	ofShader shader;
	std::vector<glm::mat4> v = sa.GetTransforms(1);
	v = sa.GetTransforms(2);
	v = sa.GetTransforms(5);
	v = sa.GetTransforms(11);
	v = sa.GetTransforms(22);

	//const ofShader * getVideoShader(const ofBaseVideoDraws & video) const;
	aiAnimation *a2;
	aiNodeAnim* p;
	aiVectorKey mPositionKeys;
	int c = model.getAnimationCount();
	for (int i = 0; i < c; ++i) {
	ofxAssimpAnimation & a = model.getAnimation(i);
	a2 = a.getAnimation();
	for (int j = 0; j < a2->mNumChannels; ++j) {
	p = a2->mChannels[j];
	for (int k = 0; k < p->mNumPositionKeys; ++k) {
	mPositionKeys = p->mPositionKeys[k];
	}
	}
	}
	*/
	// cc_standard is ok, but the foot is off person1_motion0.fbx
	model.setup("C:\\Users\\mark\\Documents\\iclone\\heidi_motion0.fbx", ofPoint(ofGetWidth() * 0.5, (float)ofGetHeight() * 0.5, 0));
	model2.setup("C:\\Users\\mark\\Documents\\iclone\\guy1_motion0.fbx", ofPoint(0, 0, 0));
	// draw 1) default custom, all in one file - fail, 2) remove unlink mesh fail, went to 3ds max

	// now trying multiple files, set all to 30/sec, unreal engine, works but does not draw hair thing, 

	// now trying h2_catwalk.fbx and its drawing perfect, let me rexport and see (h3) custom default works well but shoes are too small

	// now trying h3_catwalk.fbx and its drawing perfect, let me rexport and see (h3) unreal works well but shoes maybe a bit too smal

	// now try the horse

	

	model.setPlay(aiString("AnimStack::motion0"));
	model2.setPlay(aiString("AnimStack::motion0"));

	//model.playAllAnimations();
	//ofSetGlobalAmbientColor(ofColor(0, 255, 0));
	ofSetSmoothLighting(true);
	//ofSetLogLevel(OF_LOG_VERBOSE);

	//From2552Software::Sound sound;
	//sound.test();

	//enable to debug etc ofSetLogLevel(OF_LOG_VERBOSE);
	//ofBackground(0);
	ofFill();

	myKinect.open();
	audio.setup(&myKinect);
	//audio.setup(&myKinect);
	//audio.setup(&myKinect);
	
	//faces.setup(&myKinect);
	//bodies.useFaces();
	//bodies.setup(&myKinect);

	//ofSetWindowShape(640 * 2, 480 * 2);
}

//--------------------------------------------------------------
void ofApp::update(){
	model.update();
	model2.update();
	//faces.baseline(); //use to debug, can do what ever needed to get things to work, to create a working base line
	//faces.update();
	//bodies.update();
	//audio.update();
	audio.update();
#if sample	
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
#endif
}

//--------------------------------------------------------------
void ofApp::draw(){

	//ofShader shader =  getCurrentShader();

	//ofSetColor(255);

	// ofEnableBlendMode(OF_BLENDMODE_ALPHA);

	ofEnableDepthTest();
#ifndef TARGET_PROGRAMMABLE_GL    
	glShadeModel(GL_SMOOTH); //some model / light stuff
#endif
	camera.begin();
	light.enable();
	/*
	camera.begin();
	//spotlight is drawn in the camera frame and oriented relative to the focus of the camera
	camera.setDistance(3.0f);
	camera.setNearClip(0.01f);
	camera.setFarClip(500.0f);
	camera.setPosition(0.4f, 0.2f, 0.8f);
	camera.lookAt(ofVec3f(0.0f, 0.0f, 0.0f));
	camera.end();
	*/
	//ofEnableSeparateSpecularLight();

	model.draw();

	model2.draw();


#ifndef TARGET_PROGRAMMABLE_GL    
	//glEnable(GL_NORMALIZE);
#endif

	model.setMesh();

	model2.setMesh();

	ofDisableDepthTest();
	light.disable();
	ofDisableLighting();
	ofDisableSeparateSpecularLight();
	ofSetColor(255, 255, 255);
	ofDrawBitmapString("fps: " + ofToString(ofGetFrameRate(), 2), 10, 15);
	ofDrawBitmapString("keys 1-5 load models, spacebar to trigger animation", 10, 30);
	ofDrawBitmapString("drag to control animation with mouseY", 10, 45);
	ofDrawBitmapString("num animations for this model: " + ofToString(model.getAnimationCount()), 10, 60);

	light.disable();
	ofDisableLighting();
	ofSetColor(light.getDiffuseColor());
	light.draw();
	camera.end();


	/*
	kinect.getDepthSource()->draw(0, 0, previewWidth, previewHeight);  // note that the depth texture is RAW so may appear dark

																	   // Color is at 1920x1080 instead of 512x424 so we should fix aspect ratio

	kinect.getColorSource()->draw(previewWidth, 0 + colorTop, previewWidth, colorHeight);
	kinect.getBodySource()->drawProjected(previewWidth, 0 + colorTop, previewWidth, colorHeight);

	kinect.getInfraredSource()->draw(0, previewHeight, previewWidth, previewHeight);

	kinect.getBodyIndexSource()->draw(previewWidth, previewHeight, previewWidth, previewHeight);
	kinect.getBodySource()->drawProjected(previewWidth, previewHeight, previewWidth, previewHeight, ofxKFW2::ProjectionCoordinates::DepthCamera);
	*/
	//float colorHeight = previewWidth * (kinect.getColorSource()->getHeight() / kinect.getColorSource()->getWidth());
	//float colorTop = (previewHeight - colorHeight) / 2.0;
	//kinect.getBodySource()->drawProjected(previewWidth, 0 + colorTop, previewWidth, colorHeight, ofxKFW2::ProjectionCoordinates::DepthCamera);
	//faces.drawProjected(kinect.getBodySource()->getBodies(), previewWidth, 0 + colorTop, previewWidth, colorHeight, ofxKFW2::ProjectionCoordinates::DepthCamera);
	//faces.draw();
	//bodies.draw();

}

void ofApp::mouseDragged(int x, int y, int button) {
	// scrub through aninations manually.
	animationPosition = y / (float)ofGetHeight();
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	// pause all animations, so we can scrub through them manually.
	model.setPausedForAllAnimations(true);
	animationPosition = y / (float)ofGetHeight();
	bAnimateMouse = true;
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	// unpause animations when finished scrubbing.
	if (bAnimate) {
		model.setPausedForAllAnimations(false);
	}
	bAnimateMouse = false;
}
//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

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
