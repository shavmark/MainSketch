#pragma once
#include "ofMain.h"
#include "ole2.h"

// For M_PI and log definitions
#define _USE_MATH_DEFINES
#include <math.h>

namespace From2552Software {
	class BaseClass2552 {
	public:
		BaseClass2552() { valid = false; }

		bool objectValid() { return valid; } // data is in a good state
		void setValid(bool b = true) { valid = b; };

		bool checkPointer(IUnknown *p, string message);
		bool checkPointer(BaseClass2552 *p, string message);
		void logError(string error);
		void logVerbose(string message) { logTrace(message, OF_LOG_VERBOSE); }; // promote trace, make it obvious and easy
		void logTrace(string message, ofLogLevel level = OF_LOG_NOTICE);
		void logError(HRESULT hResult, string message = "");

		template<class Interface> void SafeRelease(Interface *& pInterfaceToRelease)
		{
			if (pInterfaceToRelease != NULL) {
				pInterfaceToRelease->Release();
				pInterfaceToRelease = NULL;
			}
		}

	private:
		bool valid; // true when data is valid

	};
}

