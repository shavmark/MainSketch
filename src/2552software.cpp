#include "2552software.h"
#include <istream>
#pragma comment( lib, "sapi.lib" )

namespace From2552Software {
	bool Trace2552::checkPointer2(IUnknown *p, const string&  message, char*file, int line) {
		logVerbose2(message, file, line); // should give some good trace
		if (p == nullptr) {
			logError2("invalid pointer " + message, file, line);
			return false;
		}
		return true;
	}
	bool Trace2552::checkPointer2(BaseClass2552 *p, const string&  message, char*file, int line) {
		logVerbose2(message, file, line); // should give some good trace
		if (p == nullptr) {
			logError2("invalid pointer " + message, file, line);
			return false;
		}
		return true;
	}
	string Trace2552::buildString(const string& text, char* file, int line) {
		string s = text + " ";
		s += file;
		s += ": ";
		s += std::to_string(line);
		return s;
	}
	void Trace2552::logError2(const string& errorIn, char* file, int line ) {
		ofLog(OF_LOG_FATAL_ERROR, buildString(errorIn, file,  line));
	}
	void Trace2552::logError2(HRESULT hResult, const string&  message, char*file, int line) {

		std::ostringstream stringStream;
		stringStream << message;
		stringStream << ":  ";
		stringStream << std::hex << hResult; //todo example this to text bugbug

		logError2(stringStream.str(), file, line);

	}
	void Trace2552::logTrace2(const string& message,  char*file, int line) {
		if (ofGetLogLevel() >= OF_LOG_NOTICE) {
			ofLog(OF_LOG_NOTICE, buildString(message, file, line));
		}
	}
	bool Trace2552::CheckHresult2(HRESULT hResult, const string& message, char*file, int line) {
		if (FAILED(hResult)) {
			if (hResult != E_PENDING) {
				logError2(hResult, message, file, line);
			}
			return true; // error found
		}
		logVerbose2(message, file, line);
		return false; // no error
	}
	std::string Trace2552::wstrtostr(const std::wstring &wstr)	{
		std::string strTo;
		char *szTo = new char[wstr.length() + 1];
		szTo[wstr.size()] = '\0';
		WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, szTo, (int)wstr.length(), NULL, NULL);
		strTo = szTo;
		delete[] szTo;
		return strTo;
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
		HRESULT hr = S_OK;

		// Find the best matching installed en-us recognizer.
		CComPtr<ISpObjectToken> cpRecognizerToken;

		if (SUCCEEDED(hr))
		{
			hr = SpFindBestToken(SPCAT_RECOGNIZERS, L"language=409", NULL, &cpRecognizerToken);
		}

		// Create the in-process recognizer and immediately set its state to inactive.
		CComPtr<ISpRecognizer> cpRecognizer;

		if (SUCCEEDED(hr))
		{
			hr = cpRecognizer.CoCreateInstance(CLSID_SpInprocRecognizer);
		}

		if (SUCCEEDED(hr))
		{
			hr = cpRecognizer->SetRecognizer(cpRecognizerToken);
		}

		if (SUCCEEDED(hr))
		{
			hr = cpRecognizer->SetRecoState(SPRST_INACTIVE);
		}

		// Create a new recognition context from the recognizer.
		CComPtr<ISpRecoContext> cpContext;

		if (SUCCEEDED(hr))
		{
			hr = cpRecognizer->CreateRecoContext(&cpContext);
		}

		// Subscribe to the speech recognition event and end stream event.
		if (SUCCEEDED(hr))
		{
			ULONGLONG ullEventInterest = SPFEI(SPEI_RECOGNITION);
			hr = cpContext->SetInterest(ullEventInterest, ullEventInterest);
		}

		// Establish a Win32 event to signal when speech events are available.
		HANDLE hSpeechNotifyEvent = INVALID_HANDLE_VALUE;

		if (SUCCEEDED(hr))
		{
			hr = cpContext->SetNotifyWin32Event();
		}

		if (SUCCEEDED(hr))
		{
			hSpeechNotifyEvent = cpContext->GetNotifyEventHandle();

			if (INVALID_HANDLE_VALUE == hSpeechNotifyEvent)
			{
				// Notification handle unsupported.
				hr = E_NOINTERFACE;
			}
		}

		// Initialize an audio object to use the default audio input of the system and set the recognizer to use it.
		CComPtr<ISpAudio> cpAudioIn;

		if (SUCCEEDED(hr))
		{
			hr = cpAudioIn.CoCreateInstance(CLSID_SpMMAudioIn);
		}

		if (SUCCEEDED(hr))
		{
			hr = cpRecognizer->SetInput(cpAudioIn, TRUE);
		}

		// Populate a WAVEFORMATEX struct with our desired output audio format. information.
		WAVEFORMATEX* pWfexCoMemRetainedAudioFormat = NULL;
		GUID guidRetainedAudioFormat = GUID_NULL;

		if (SUCCEEDED(hr))
		{
			hr = SpConvertStreamFormatEnum(SPSF_16kHz16BitMono, &guidRetainedAudioFormat, &pWfexCoMemRetainedAudioFormat);
		}

		// Instruct the recognizer to retain the audio from its recognition results.
		if (SUCCEEDED(hr))
		{
			hr = cpContext->SetAudioOptions(SPAO_RETAIN_AUDIO, &guidRetainedAudioFormat, pWfexCoMemRetainedAudioFormat);
		}

		if (NULL != pWfexCoMemRetainedAudioFormat)
		{
			CoTaskMemFree(pWfexCoMemRetainedAudioFormat);
		}

		// Create a new grammar and load an SRGS grammar from file.
		CComPtr<ISpRecoGrammar> cpGrammar;

		if (SUCCEEDED(hr))
		{
			hr = cpContext->CreateGrammar(0, &cpGrammar);
		}

		if (SUCCEEDED(hr))
		{
			hr = cpGrammar->LoadCmdFromFile(L"grammar.grxml", SPLO_STATIC);
		}

		// Set all top-level rules in the new grammar to the active state.
		if (SUCCEEDED(hr))
		{
			hr = cpGrammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);
		}

		// Set the recognizer state to active to begin recognition.
		if (SUCCEEDED(hr))
		{
			hr = cpRecognizer->SetRecoState(SPRST_ACTIVE_ALWAYS);
		}

		// Establish a separate Win32 event to signal the event loop exit.
		HANDLE hExitEvent = CreateEventW(NULL, FALSE, FALSE, NULL);

		// Collect the events listened for to pump the speech event loop.
		HANDLE rghEvents[] = { hSpeechNotifyEvent, hExitEvent };

		// Speech recognition event loop.
		BOOL fContinue = TRUE;

		while (fContinue && SUCCEEDED(hr))
		{
			// Wait for either a speech event or an exit event, with a 15 second timeout.
			DWORD dwMessage = WaitForMultipleObjects(sp_countof(rghEvents), rghEvents, FALSE, 15000);

			switch (dwMessage)
			{
				// With the WaitForMultipleObjects call above, WAIT_OBJECT_0 is a speech event from hSpeechNotifyEvent.
			case WAIT_OBJECT_0:
			{
				// Sequentially grab the available speech events from the speech event queue.
				CSpEvent spevent;

				while (S_OK == spevent.GetFrom(cpContext))
				{
					switch (spevent.eEventId)
					{
					case SPEI_RECOGNITION:
					{
						// Retrieve the recognition result and output the text of that result.
						ISpRecoResult* pResult = spevent.RecoResult();

						LPWSTR pszCoMemResultText = NULL;
						hr = pResult->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &pszCoMemResultText, NULL);

						if (SUCCEEDED(hr))
						{
							wprintf(L"Recognition event received, text=\"%s\"\r\n", pszCoMemResultText);
						}

						// Also retrieve the retained audio we requested.
						CComPtr<ISpStreamFormat> cpRetainedAudio;

						if (SUCCEEDED(hr))
						{
							hr = pResult->GetAudio(0, 0, &cpRetainedAudio);
						}

						// To demonstrate, we'll speak the retained audio back using ISpVoice.
						CComPtr<ISpVoice> cpVoice;

						if (SUCCEEDED(hr))
						{
							hr = cpVoice.CoCreateInstance(CLSID_SpVoice);
						}

						if (SUCCEEDED(hr))
						{
							hr = cpVoice->SpeakStream(cpRetainedAudio, SPF_DEFAULT, 0);
						}

						if (NULL != pszCoMemResultText)
						{
							CoTaskMemFree(pszCoMemResultText);
						}

						break;
					}
					}
				}

				break;
			}
			case WAIT_OBJECT_0 + 1:
			case WAIT_TIMEOUT:
			{
				// Exit event or timeout; discontinue the speech loop.
				fContinue = FALSE;
				//break;
			}
			}
		}

	CoUninitialize();

		CComPtr <ISpVoice>		cpVoice;
		CComPtr <ISpStream>		cpStream;
		CSpStreamFormat			cAudioFmt;

		//Create a SAPI Voice
		hr = cpVoice.CoCreateInstance(CLSID_SpVoice);

		//Set the audio format
		if (SUCCEEDED(hr))
		{
			hr = cAudioFmt.AssignFormat(SPSF_22kHz16BitMono);
		}

		//Call SPBindToFile, a SAPI helper method,  to bind the audio stream to the file
		if (SUCCEEDED(hr))
		{

			hr = SPBindToFile(L"c:\\ttstemp.wav", SPFM_CREATE_ALWAYS,
				&cpStream, &cAudioFmt.FormatId(), cAudioFmt.WaveFormatExPtr());
		}

		//set the output to cpStream so that the output audio data will be stored in cpStream
		if (SUCCEEDED(hr))
		{
			hr = cpVoice->SetOutput(cpStream, TRUE);
		}

		//Speak the text "hello world" synchronously
		if (SUCCEEDED(hr))
		{
			hr = cpVoice->Speak(L"Hello World", SPF_DEFAULT, NULL);
		}

		//close the stream
		if (SUCCEEDED(hr))
		{
			hr = cpStream->Close();
		}

		//Release the stream and voice object
		cpStream.Release();
		cpVoice.Release();

		CComPtr<ISpGrammarBuilder>    cpGrammarBuilder;
		SPSTATEHANDLE                 hStateTravel;
		SPSTATEHANDLE                 hStateTravel_Second;
		SPSTATEHANDLE                 hStateMethod;
		SPSTATEHANDLE                 hStateDest;
		// Create (if rule does not already exist)
		// top-level Rule, defaulting to Active.
		hr = cpGrammarBuilder->GetRule(L"Travel", 0, SPRAF_TopLevel | SPRAF_Active, TRUE, &hStateTravel);

		// Approach 1: List all possible phrases.
		// This is the most intuitive approach, and it does not sacrifice efficiency
		// because the grammar builder will merge shared sub-phrases when possible.
		// There is only one root state, hStateTravel, and the terminal NULL state,
		// and there are six unique transitions between root state and NULL state.

		/* XML Approximation:
		<rule id="Travel">
		<item> fly to Seattle </item>
		<item> fly to New York </item>
		<item> fly to Washington DC </item>
		<item> drive to Seattle </item>
		<item> drive to New York </item>
		<item> drive to Washington DC </item>
		</rule>
		*/

		// Create set of peer phrases, each containing complete phrase.
		// Note: the word delimiter is set as " ", so that the text we
		// attach to the transition can be multiple words (for example,
		// "fly to Seattle" is implicitly "fly" + "to" + "Seattle"):
		if (SUCCEEDED(hr))
		{
			hr = cpGrammarBuilder->AddWordTransition(hStateTravel, NULL, L"fly to Seattle", L" ", SPWT_LEXICAL, 1, NULL);
		}
		if (SUCCEEDED(hr))
		{
			hr = cpGrammarBuilder->AddWordTransition(hStateTravel, NULL, L"fly to New York", L" ", SPWT_LEXICAL, 1, NULL);
		}
		if (SUCCEEDED(hr))
		{
			hr = cpGrammarBuilder->AddWordTransition(hStateTravel, NULL, L"fly to Washington DC", L" ", SPWT_LEXICAL, 1, NULL);
		}
		if (SUCCEEDED(hr))
		{
			hr = cpGrammarBuilder->AddWordTransition(hStateTravel, NULL, L"drive to Seattle", L" ", SPWT_LEXICAL, 1, NULL);
		}
		if (SUCCEEDED(hr))
		{
			hr = cpGrammarBuilder->AddWordTransition(hStateTravel, NULL, L"drive to New York", L" ", SPWT_LEXICAL, 1, NULL);
		}
		if (SUCCEEDED(hr))
		{
			hr = cpGrammarBuilder->AddWordTransition(hStateTravel, NULL, L"drive to Washington DC", L" ", SPWT_LEXICAL, 1, NULL);
		}
		// Find the best matching installed en-US recognizer.
		//CComPtr<ISpObjectToken> cpRecognizerToken;

		if (SUCCEEDED(hr))
		{
			hr = SpFindBestToken(SPCAT_RECOGNIZERS, L"language=409", NULL, &cpRecognizerToken);
		}

		// Create the in-process recognizer and immediately set its state to inactive.
		//CComPtr<ISpRecognizer> cpRecognizer;

		if (SUCCEEDED(hr))
		{
			hr = cpRecognizer.CoCreateInstance(CLSID_SpInprocRecognizer);
		}

		if (SUCCEEDED(hr))
		{
			hr = cpRecognizer->SetRecognizer(cpRecognizerToken);
		}

		if (SUCCEEDED(hr))
		{
			hr = cpRecognizer->SetRecoState(SPRST_INACTIVE);
		}

		// Create a new recognition context from the recognizer.
		//CComPtr<ISpRecoContext> cpContext;

		if (SUCCEEDED(hr))
		{
			hr = cpRecognizer->CreateRecoContext(&cpContext);
		}

		// Subscribe to the speech recognition event and end stream event.
		if (SUCCEEDED(hr))
		{
			ULONGLONG ullEventInterest = SPFEI(SPEI_RECOGNITION) | SPFEI(SPEI_END_SR_STREAM);
			hr = cpContext->SetInterest(ullEventInterest, ullEventInterest);
		}

		// Establish a Win32 event to signal when speech events are available.
		//HANDLE hSpeechNotifyEvent = INVALID_HANDLE_VALUE;

		if (SUCCEEDED(hr))
		{
			hr = cpContext->SetNotifyWin32Event();
		}

		if (SUCCEEDED(hr))
		{
			hr = cpContext->SetNotifyWin32Event();
		}

		if (SUCCEEDED(hr))
		{
			hSpeechNotifyEvent = cpContext->GetNotifyEventHandle();

			if (INVALID_HANDLE_VALUE == hSpeechNotifyEvent)
			{
				// Notification handle unsupported
				//hr = SPERR_UNITIALIZED;
			}
		}
		// Set up an audio input stream using a .wav file and set the recognizer's input.
		CComPtr<ISpStream> cpInputStream;

		if (SUCCEEDED(hr))
		{
			hr = SPBindToFile(L"Test.wav", SPFM_OPEN_READONLY, &cpInputStream);
		}

		if (SUCCEEDED(hr))
		{
			hr = cpRecognizer->SetInput(cpInputStream, TRUE);
		}

		// Create a new grammar and load an SRGS grammar from file.
		//CComPtr<ISpRecoGrammar> cpGrammar;

		if (SUCCEEDED(hr))
		{
			hr = cpContext->CreateGrammar(0, &cpGrammar);
		}

		if (SUCCEEDED(hr))
		{
			hr = cpGrammar->LoadCmdFromFile(L"grammar.grxml", SPLO_STATIC);
		}

		// Set all top-level rules in the new grammar to the active state.
		if (SUCCEEDED(hr))
		{
			hr = cpGrammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);
		}

		// Finally, set the recognizer state to active to begin recognition.
		if (SUCCEEDED(hr))
		{
			hr = cpRecognizer->SetRecoState(SPRST_ACTIVE_ALWAYS);
		}

		 hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void     **)&pVoice);
		if (SUCCEEDED(hr)) {
			hr = SpEnumTokens(SPCAT_VOICES, L"Gender=Female", NULL, &pEnum);
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
					wchar_t* start = L"<?xml version=\"1.0\" encoding=\"ISO - 8859 - 1\"?><speak version = \"1.0\" xmlns = \"http://www.w3.org/2001/10/synthesis\"	xml:lang = \"en-US\">";
					wchar_t* end = L"</speak>";
					const wchar_t *xml = L"<voice required = \"Gender=Male\"> hi! <prosody pitch=\"fast\"> This is low pitch. </prosody><prosody volume=\"x - loud\"> This is extra loud volume. </prosody>";
					wstring s = start;
					s += xml;
					s += end;
					
					hr = pVoice->Speak(xml, SPF_IS_XML| SPF_ASYNC, 0);
					//hr = pVoice->Speak(L"How are you?", SPF_DEFAULT, NULL);
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