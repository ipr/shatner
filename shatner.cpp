// shatner.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "shatner.h"

#include "MemoryMappedFile.h"

#include <tchar.h>
#include <shellapi.h>

#include "sha.h"

#include <string>

void hexEncode(const unsigned char *pBuffer, const size_t nLen, std::wstring &szOutput)
{
	if (pBuffer == NULL
		|| pBuffer == nullptr)
	{
		return;
	}
	
	//wchar_t hextable[] = _T("0123456789ABCDEF");
	TCHAR hextable[] = _T("0123456789ABCDEF");

	// reserve 2*iLen space in output buffer first
	// if output should be larger than reserved string-space
	// (should improve efficiency slightly..)
	//
	// check remaining capacity against to be added
	size_t nSize = szOutput.size();
	size_t nCapacity = szOutput.capacity();
	if ((nCapacity - nSize) < nLen*2)
	{
		// reserve larger string-buffer
		// to fix existing and to be added
		szOutput.reserve(nSize + (nLen*2));
	}
	
	// determine half-bytes of each byte 
	// and appropriate character representing value of it
	// for hex-encoded string
	for ( size_t y = 0; y < nLen; y++ )
	{
		unsigned char upper;
		unsigned char lower;

		upper = lower = pBuffer[y];

		lower = lower & 0xF;

		upper = upper >> 4;
		upper = upper & 0xF;

		// C++ STL string grows automatically so we just push new
		// characters at the end, same way with reserve().
		//
		szOutput += hextable[upper]; szOutput += hextable[lower];
	}
}


// Global Variables:
HINSTANCE g_hInst;                                // current instance

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	g_hInst = hInstance;

	int argc = 0;
	LPWSTR *args = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (args == NULL || argc < 2)
	{
		return -1;
	}

	LPWSTR file = NULL;

	USHAContext ctx;

	if (argc == 2)
	{
		// just select default mode, expect file
		ctx.whichSha = SHA512;
		file = args[1];
	}
	else if (argc > 2)
	{
		std::wstring mode(args[1]);

		// check mode
		if (mode == L"-1")
		{
			ctx.whichSha = SHA1;
		}
		else if (mode == L"-2")
		{
			ctx.whichSha = SHA224;
		}
		else if (mode == L"-3")
		{
			ctx.whichSha = SHA256;
		}
		else if (mode == L"-4")
		{
			ctx.whichSha = SHA384;
		}
		else if (mode == L"-5")
		{
			ctx.whichSha = SHA512;
		}
		else
		{
			return -2;
		}

		file = args[2];
	}

	CMemoryMappedFile mmFile(file);
	if (mmFile.IsCreated() == false)
	{
		return -2;
	}
	if (USHAReset(&ctx, ctx.whichSha) != 0)
	{
		return -3;
	}
	if (USHAInput(&ctx, (uint8_t*)mmFile.GetView(), mmFile.GetSize()) != 0)
	{
		return -4;
	}

	int hashSize = USHAHashSize(ctx.whichSha);
	uint8_t Message_Digest[USHAMaxHashSize];
	if (USHAResult(&ctx, Message_Digest) != 0)
	{
		return -5;
	}

	std::wstring digest;
	hexEncode(Message_Digest, hashSize, digest);

	::OutputDebugString(file);
	::OutputDebugString(L" ");
	::OutputDebugString(digest.c_str());
	::OutputDebugString(L"\r\n");

	/*
	for (int i = 1; i < argc; i++)
	{
		::OutputDebugString(args[i]);
		::OutputDebugString(L"\r\n");
	}
	*/

	LocalFree(args);
	return 0;
}

