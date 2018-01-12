
#include "gim_common.h"

namespace image {

void Png2Gim(LPCSTR strPngFile, LPCSTR strGimFile, GIM_IMAGE &oriimg)
{
	char szCmdLine[1024];
	wsprintfA(szCmdLine, "\"F:\\psp\\GimConv\\GimConv.exe\" \"%s\" -o \"%s\"", 
		strPngFile, strGimFile);

	if (oriimg.image->pixel_order == 1)
	{
		lstrcpyA(szCmdLine + lstrlenA(szCmdLine), " -N");
	}

	switch (oriimg.image->image_format)
	{
	case 0x04:
		lstrcpyA(szCmdLine + lstrlenA(szCmdLine), " -bpp4");
		break;
	case 0x05:
		lstrcpyA(szCmdLine + lstrlenA(szCmdLine), " -bpp8");
		break;
	default:
		if (oriimg.image->color_depth == 0x20)
		{
			lstrcpyA(szCmdLine + lstrlenA(szCmdLine), " -bpp8");
			break;
		}
		printf("unsupport image format.\n");
		MessageBoxA(NULL, "unsupport image format.", "Error", NULL);
		return;
	}

	PROCESS_INFORMATION piProcInfo = {0};
	STARTUPINFOA siStartInfo = {0};

	// Set up members of STARTUPINFO strUCture.
	siStartInfo.cb = sizeof(STARTUPINFOA); 

	// Create the child process.
	if (CreateProcessA(
			NULL,
			szCmdLine,
			NULL, // process security attributes
			NULL, // primary thread security attributes
			0, // handles are inherited
			0, // creation flags
			NULL, // use parent's environment
			"F:\\psp\\GimConv", // use parent's current Directory
			&siStartInfo, // STARTUPINFO pointer
			&piProcInfo)) // receives PROCESS_INFORMATION
	{
		// Wait for the processs to finish
		DWORD rc = WaitForSingleObject(
			piProcInfo.hProcess, // process handle
			INFINITE);
	} else {
		LPVOID lpMsgBuf;
		FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPSTR)&lpMsgBuf,
				0,
				NULL
				);
		printf("%s\n", lpMsgBuf);
		LocalFree(lpMsgBuf);
	}
}

}