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

		bool checkPointer(IUnknown *p, string message);
		bool checkPointer(BaseClass2552 *p, string message);
		void logError(string error, char*file=__FILE__, int line=__LINE__);
		void logVerbose(string message) { logTrace(message, OF_LOG_VERBOSE); }; // promote trace, make it obvious and easy
		void logTrace(string message, ofLogLevel level = OF_LOG_NOTICE, char*file = __FILE__, int line = __LINE__);
		void logError(HRESULT hResult, string message = "", char*file = __FILE__, int line = __LINE__);

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

