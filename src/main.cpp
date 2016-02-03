#include <sapi.h>  
#include <sphelper.h>  
#include <Synchapi.h>
#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
	
	ofGLWindowSettings settings;
	settings.setGLVersion(4, 5);
	ofCreateWindow(settings);     // this kicks off the running of my app    

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
