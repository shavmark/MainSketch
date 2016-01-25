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
#include "ole2.h"

namespace From2552Software {
	// root class, basic and small but items every object needs.  Try to avoid adding data to keep it small
	class BaseClass2552 {
	public:

		static bool checkPointer2(IUnknown *p, string message, char*file = __FILE__, int line = __LINE__);
		static bool checkPointer2(BaseClass2552 *p, string message, char*file = __FILE__, int line = __LINE__);

		static void logError2(string error, char*file=__FILE__, int line=__LINE__);
		static void logVerbose2(string message, char*file = __FILE__, int line = __LINE__) { logTrace2(message, OF_LOG_VERBOSE, file, line); }; // promote trace, make it obvious and easy
		static void logTrace2(string message, ofLogLevel level = OF_LOG_NOTICE, char*file = __FILE__, int line = __LINE__);
		static void logError2(HRESULT hResult, string message = "", char*file = __FILE__, int line = __LINE__);

// get the right line number
#define logError(p1, p2) BaseClass2552::logError2(p1, p2, __FILE__, __LINE__)
#define logErrorString(p1) BaseClass2552::logError2(p1, __FILE__, __LINE__)
#define logVerbose(p1) BaseClass2552::logVerbose2(p1, __FILE__, __LINE__)
#define logTrace(p1) BaseClass2552::logTrace2(p1, OF_LOG_NOTICE, __FILE__, __LINE__)
#define checkPointer(p1, p2) BaseClass2552::checkPointer2(p1, p2, __FILE__, __LINE__)

		template<class Interface> void SafeRelease(Interface *& pInterfaceToRelease)
		{
			if (pInterfaceToRelease != NULL) {
				pInterfaceToRelease->Release();
				pInterfaceToRelease = NULL;
			}
		}

	private:

	};

	// drawing related items start here
	class BaseClass2552WithDrawing: public BaseClass2552 {
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

