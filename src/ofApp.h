#pragma once
#include "ofMain.h"

#include "kinect2552.h"


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		From2552Software::KinectBodies bodies;
		From2552Software::KinectFaces faces;
		From2552Software::KinectAudio audio;
		From2552Software::Kinect2552 myKinect;
		
};
