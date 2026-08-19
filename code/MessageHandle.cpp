#include "imgui_includes.h"
#ifdef _WIN32 
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <string.h>

#ifndef MSG_WINDOW
#include "MessageHandle.h"
#endif

#ifndef _WIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#endif

char title[256];
ErrorPrompt errorPromptParams;

void FireMessage(const char* message, ErrorType severity) {

	ImGui::DebugLog("FIREMESSAGE - %s\n", message);

	strcpy(errorPromptParams.errorMessage, message);
	errorPromptParams.errorSeverity = severity;

	memset(title, 0, 256);
	if (severity == ErrorType_None) {
		CloseMessage();
		return;
	}

	if (severity == ErrorType_Info) {
		strcpy(title, "Informational Message");
		//MessageBeep(MB_ICONINFORMATION);
	}

	if (severity == ErrorType_Warn) {
		strcpy(title, "Warning Message");
		//MessageBeep(MB_ICONEXCLAMATION);
	}

	if (severity == ErrorType_Error) {
		strcpy(title, "Error Message");
		//MessageBeep(MB_ICONERROR);
	}

	if (severity == ErrorType_Critical) {
		strcpy(title, "Critical Message");
		//MessageBeep(MB_ICONERROR);
	}


	errorPromptParams.showErrorPrompt = true;
}

void ResetMessage() {
	memset(errorPromptParams.errorMessage, 0, 1024);
	errorPromptParams.errorSeverity = ErrorType_None;

	errorPromptParams.showErrorPrompt = true;
}

void CloseMessage() {
	ResetMessage();

	errorPromptParams.showErrorPrompt = false;
}

void DisplayMessageWindow() {

	//ImGui::DebugLog("DISPLAY - %s\n", errorPromptParams.errorMessage);

	if (ImGui::Begin(title, NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
		ImGui::Text(errorPromptParams.errorMessage);
		ImGui::Spacing();
		if (ImGui::Button("Okay")) {
			CloseMessage();
			ImGui::CloseCurrentPopup();
		}
		ImGui::End();
	}
}

bool IsMessagePromptShowing() { return errorPromptParams.showErrorPrompt; }
char* GetMessagePromptMessage() { return errorPromptParams.errorMessage; }
ErrorType GetMessagePromptSeverity() { return errorPromptParams.errorSeverity; }
