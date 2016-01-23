#include "2552software.h"

namespace From2552Software {
	bool BaseClass2552::checkPointer(IUnknown *p, string message) {
		logVerbose(message); // should give some good trace
		if (p == nullptr) {
			logError("in valid pointer " + message);
			return false;
		}
		return true;
	}
	bool BaseClass2552::checkPointer(BaseClass2552 *p, string message) {
		logVerbose(message); // should give some good trace
		if (p == nullptr) {
			logError("in valid pointer " + message);
			return false;
		}
		return true;
	}

	void BaseClass2552::logError(string errorIn) {
		string error = "Error " + errorIn + " ";
		error += __FILE__;
		error += ": " + __LINE__;
		ofLog(OF_LOG_FATAL_ERROR, error);
	}
	void BaseClass2552::logError(HRESULT hResult, string message) {

		std::ostringstream stringStream;
		stringStream << message;
		stringStream << ":  ";
		stringStream << std::hex << hResult; //todo example this to text bugbug

		logError(stringStream.str());

	}

	void BaseClass2552::logTrace(string message, ofLogLevel level) {
		ofLog(level, message);
	}

}