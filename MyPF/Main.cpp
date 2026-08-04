#include "AppBase.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR param, int command)
{
	My::AppBase app;

	if (!app.Initialize())
	{
		std::cerr << "Initialization failed.\n";
		return -1;
	}

	return app.Run();
}