#pragma once
// For M_PI and log definitions
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>


// Microsoft Speech Platform SDK 11
#include <sapi.h>
#include <sphelper.h> // SpFindBestToken()
#include <strsafe.h>
#include <intsafe.h>
#include <Synchapi.h>
// keep all MS files above ofmain.h
#include "ofMain.h"
#include "ofFileUtils.h"
#include "ole2.h"
#include "ofxAssimpModelLoader.h"
#include "ofMain.h"

#include "ofVboMesh.h"
#include "cAnimationController.h"

namespace From2552Software {

	// root class, basic and small but items every object needs.  Try to avoid adding data to keep it small
	class BaseClass2552 {
	public:
	private:
	};

	class Trace2552 {
	public:
		static bool checkPointer2(IUnknown *p, const string&  message, char*file = __FILE__, int line = __LINE__);
		static bool checkPointer2(BaseClass2552 *p, const string&  message, char*file = __FILE__, int line = __LINE__);

		static void logError2(const string& error, char*file, int line);
		static void logVerbose2(const string& message, char*file, int line) {
			if (ofGetLogLevel() >= OF_LOG_VERBOSE) {
				ofLog(OF_LOG_VERBOSE, buildString(message, file, line));
			}
		}
		static void logTrace2(const string& message, char*file, int line);
		static void logError2(HRESULT hResult, const string&  message, char*file, int line);
		static bool CheckHresult2(HRESULT hResult, const string& message, char*file, int line);
		static string buildString(const string& errorIn, char* file, int line);
		static std::string wstrtostr(const std::wstring &wstr);

		// get the right line number
#define logError(p1, p2) Trace2552::logError2(p1, p2, __FILE__, __LINE__)
#define logErrorString(p1) Trace2552::logError2(p1, __FILE__, __LINE__)
#define logVerbose(p1) Trace2552::logVerbose2(p1, __FILE__, __LINE__)
#define logTrace(p1) Trace2552::logTrace2(p1, __FILE__, __LINE__)
#define checkPointer(p1, p2) Trace2552::checkPointer2(p1, p2, __FILE__, __LINE__)
#define checkPointer(p1, p2) Trace2552::checkPointer2(p1, p2, __FILE__, __LINE__)
#define hresultFails(p1, p2) Trace2552::CheckHresult2(p1, p2, __FILE__, __LINE__)

	};

	template<class Interface> void SafeRelease(Interface *& pInterfaceToRelease)
	{
		if (pInterfaceToRelease != NULL) {
			pInterfaceToRelease->Release();
			pInterfaceToRelease = NULL;
		}
	}

	// 3d model
	class TheModel : public ofxAssimpModelLoader, public Trace2552 {
	public: 
		void setup(const string& name, ofPoint point) {
			loadModel(name, true);
			setPosition(point.x, point.y, point.z);
			setLoopStateForAllAnimations(OF_LOOP_NORMAL);
		}
		void setPlay(const aiString& myname) {
			for (unsigned int i = 0; i< getAnimationCount(); i++) {
				ofxAssimpAnimation &a = getAnimation(i);
				if (a.getAnimation()->mName == myname) {
					a.play();
				}
			}
		}
		void draw() {
			ofPushMatrix();
			//ofTranslate(getPosition().x + 100, getPosition().y, 0);
			//ofRotate(180, 1, 0, 0);
			//ofTranslate(-getPosition().x, -getPosition().y, 0);
			drawFaces();
			ofPopMatrix();
		}
		void setMesh() {
			
			ofPushMatrix();
			//ofTranslate(getPosition().x + 100, getPosition().y, 0);
			//ofRotate(180, 1, 0, 0);
			//ofTranslate(-getPosition().x, -getPosition().y, 0);

			ofxAssimpMeshHelper & meshHelper = getMeshHelper(0);

			ofMultMatrix(getModelMatrix());
			ofMultMatrix(meshHelper.matrix);

			ofMaterial & material = meshHelper.material;
			if (meshHelper.hasTexture()) {
				meshHelper.getTextureRef().bind();
			}
			material.begin();
			mesh.drawVertices();
			material.end();
			if (meshHelper.hasTexture()) {
				meshHelper.getTextureRef().unbind();
			}
			ofPopMatrix();
		}
	private:
		ofMesh mesh;
	};

	// drawing related items start here
	class BaseClass2552WithDrawing: public BaseClass2552, public Trace2552 {
	public: 
		BaseClass2552WithDrawing() { valid = false; }

		bool objectValid() { return valid; } // data is in a good state
		void setValid(bool b = true) { valid = b; };

	private:
		bool valid; // true when data is valid and ready to draw
	};

	class Sound {
	public:
		void test();
	};
}

