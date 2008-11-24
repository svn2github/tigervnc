// wsconfig.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "wsconfig.h"
#include "ConfigDialog.h"

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPTSTR lpCmdLine, int nCmdShow)
{
  ConfigDialog configDialog;
  configDialog.showModal();
  return 0;
}