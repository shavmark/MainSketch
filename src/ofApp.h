#pragma once
#include <sapi.h>  
#include <sphelper.h> 
#include <Synchapi.h>
#include "2552software.h"
#include "kinect2552.h"

class ofApp : public ofBaseApp{

	public:
		ofApp():ofBaseApp(){};

		void setup();
		void update();
		void draw();

		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		From2552Software::KinectBodies bodies;
		From2552Software::KinectFaces faces;
		From2552Software::KinectAudio audio;
		From2552Software::Kinect2552 myKinect;
		
		From2552Software::TheModel model;
		From2552Software::TheModel model2;

		bool bAnimate;
		bool bAnimateMouse;
		float animationPosition;

		ofLight	light;
		ofEasyCam camera;
		SceneAnimator sa;
};
