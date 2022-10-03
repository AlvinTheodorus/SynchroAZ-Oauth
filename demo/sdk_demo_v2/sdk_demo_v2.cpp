#include "stdafx.h"
#include "sdk_demo_app.h"
#include "oauth_code.h"

using namespace DuiLib;
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow)
{
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	saz::oauth2::PassOAuthCodeToMainProcess();

	g_demoApp.Run(hInstance);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (WM_QUIT == msg.message)
		{
			break;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	g_demoApp.Cleanup();
	return 0;
}