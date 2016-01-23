#include "2552software.h"
#include <istream>
#pragma comment( lib, "sapi.lib" )
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

	void Sound::test() {
		ISpVoice * pVoice = NULL;
		ISpObjectToken*        pVoiceToken=nullptr;
		IEnumSpObjectTokens*   pEnum;
		ULONG                  ulCount = 0;

		if (FAILED(::CoInitialize(NULL)))
		{
			return;
		}

		HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void     **)&pVoice);
		if (SUCCEEDED(hr)) {
			hr = SpEnumTokens(SPCAT_VOICES, NULL, NULL, &pEnum);
			if (SUCCEEDED(hr))
			{
				// Get the number of voices.
				hr = pEnum->GetCount(&ulCount);
			}

			// Obtain a list of available voice tokens, set
			// the voice to the token, and call Speak.
			while (SUCCEEDED(hr) && ulCount--)			{
				if (pVoiceToken != nullptr) {
					pVoiceToken->Release();
				}

				if (SUCCEEDED(hr))
				{
					hr = pEnum->Next(1, &pVoiceToken, NULL);
				}

				if (SUCCEEDED(hr))
				{
					hr = pVoice->SetVoice(pVoiceToken);
				}

				if (SUCCEEDED(hr))
				{
					hr = pVoice->Speak(L"How are you?", SPF_DEFAULT, NULL);
				}

			}
			/*
			if (SUCCEEDED(hr)) {
				hr = pEnum->Next(1, &pVoiceToken, NULL);
				if (SUCCEEDED(hr)) {
					hr = pVoice->SetVoice(pVoiceToken);
					// Set the output to the default audio device.
					if (SUCCEEDED(hr)) {
						hr = pVoice->SetOutput(NULL, TRUE);
						if (SUCCEEDED(hr)) {
							hr = pVoice->Speak(L"Hello, world!", SPF_DEFAULT, 0);
						}
					}
				}
			}
			*/
			pVoice->Release();
		}
		::CoUninitialize();
	}
}