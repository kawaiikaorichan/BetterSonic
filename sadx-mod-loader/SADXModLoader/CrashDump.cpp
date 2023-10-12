#include "stdafx.h"
#include <dbghelp.h> 
#include <windows.h>
#include <direct.h>
#include <Psapi.h>
#include <shlwapi.h>
#include <time.h>

using namespace std;

const string texCrashMsg = "Texture error: the game failed to apply one or more textures. This could be a mod conflict.\nIf you are making a mod, make sure all your textures are loaded\n ";
const string charCrashMsg = "Character Crash: The game crashed in one of the character's main function.\nYou most likely have a mod order conflict.\nMods that edit gameplay should be loaded last.";

static const unordered_map<intptr_t, string> crashes_addresses_map = {
	{ 0x78CF24, texCrashMsg},
	{ 0x78D149, texCrashMsg },
	{ 0x7B293A85, "DirectX error: You most likely reached a tex ID out of range."},
	{ 0x434614, "Camera error: the game failed to load a cam file for the stage."},
	{ 0x787148, "Landtable error: The game crashed on the eval flag check.\nIf you are making a level mod, make sure all your meshes have the flag \"Skip Children\" checked."}
};

struct addressRange
{
	intptr_t start = 0;
	intptr_t end = 0;
	string crashMsg = "";
};

static const addressRange Range_Addresses_list[] =
{
	{ (intptr_t)SonicTheHedgehog, 0x49BFBB, charCrashMsg },
	{ (intptr_t)Eggman, 0x7B52DE, charCrashMsg} ,
	{ (intptr_t)MilesTalesPrower, 0x4624CB, charCrashMsg },
	{ (intptr_t)KnucklesTheEchidna, 0x47B504, charCrashMsg },
	{ (intptr_t)Tikal, 0x7B43E0, charCrashMsg },
	{ (intptr_t)AmyRose, 0x48B5B6, charCrashMsg },
	{ (intptr_t)E102, 0x4841CA, charCrashMsg },
	{ (intptr_t)BigTheCat, 0x491438, charCrashMsg },
};

static const string GetRangeAddressesCrash(const intptr_t address)
{
	for (uint8_t i = 0; i < LengthOfArray(Range_Addresses_list); i++)
	{
		if (address >= Range_Addresses_list[i].start && address <= Range_Addresses_list[i].end)
		{
			return Range_Addresses_list[i].crashMsg;
		}
	}

	return "NULL";
}


static string getErrorMSG(intptr_t address)
{
	if ((crashes_addresses_map.find(address) == crashes_addresses_map.end()))
	{
		return "NULL";
	}

	return crashes_addresses_map.find(address)->second; //return a custom error message if the address is known
}

void SetErrorMessage(string& fullMsg, const string address, const string dllName, const intptr_t crashID)
{
	string errorCommon = getErrorMSG(crashID); //get error message if the crash address is common
	fullMsg = "SADX has crashed at " + address + " (" + dllName + ").\n";

	if (errorCommon != "NULL")
	{
		fullMsg += errorCommon + "\n"; //add the common error message if it exists
	}
	else
	{
		//if the crash isn't in the list, check if it's a common crash from addresses from a whole function...
		auto charcrash = GetRangeAddressesCrash(crashID);

		if (charcrash != "NULL")
		{
			fullMsg += charcrash + "\n";
		}
	}

	fullMsg += "\nA minidump has been created in your SADX folder.\n";
}

void CopyAndRename_ModLoaderIni()
{
	char timeStr[255];
	time_t t = time(NULL);
	tm tM;
	localtime_s(&tM, &t);
	strftime(timeStr, 255, "_%d_%m_%Y_%H_%M_%S", &tM);
	char tmp[256];
	string directory = getcwd(tmp, 256);

	const string quote = "\"";
	string fullLine = "xcopy " + quote + directory + "\\mods\\SADXModLoader.ini" + quote + " " + quote + directory + "\\CrashDump" + quote;
	int copyState = system(fullLine.c_str());

	if (copyState != -1) {
		string rename = "ren " + quote + directory + "\\CrashDump\\SADXModLoader.ini" + quote + " " + quote + "ModList" + timeStr + ".ini" + quote;
		system(rename.c_str());
		PrintDebug("CrashDump: Successfully copied SADXModLoader.ini to the CrashDump Folder.\n");
	}
	else
	{
		PrintDebug("CrashDump: Failed to copy SADXModLoader.ini to the CrashDump Folder.\n");
	}
}

bool IsPathExist(const string& s)
{
	struct stat buffer;
	return (stat(s.c_str(), &buffer) == 0);
}

#pragma comment(lib, "dbghelp.lib") 
#pragma comment(lib, "Psapi.lib")
LONG WINAPI HandleException(struct _EXCEPTION_POINTERS* apExceptionInfo)
{
	char timeStr[255];
	time_t t = time(NULL);
	tm tM;
	localtime_s(&tM, &t);
	strftime(timeStr, 255, "CrashDump_%d_%m_%Y_%H_%M_%S.dmp", &tM);
	string s = "CrashDump\\";
	s.append(timeStr);

	const char* crashFolder = "CrashDump";

	if (!IsPathExist(crashFolder))
	{
		_mkdir(crashFolder);
	}

	//generate crash dump
	HANDLE hFile = CreateFileA(
		s.c_str(),
		GENERIC_WRITE | GENERIC_READ,
		0,
		NULL,
		CREATE_ALWAYS,
		0,
		NULL
	);

	HANDLE hProcess = GetCurrentProcess();
	PrintDebug("SADX HAS CRASHED!\n");

	if (hFile != NULL)
	{
		PrintDebug("Generating Crash Dump file...\n");
		MINIDUMP_EXCEPTION_INFORMATION info =
		{
		 GetCurrentThreadId(),
		 apExceptionInfo,
		 TRUE
		};

		MiniDumpWriteDump(
			hProcess,
			GetCurrentProcessId(),
			hFile,
			MiniDumpWithIndirectlyReferencedMemory,
			&info,
			NULL,
			NULL
		);

		CloseHandle(hFile);

		PrintDebug("Done.\n");

		//get crash address
		intptr_t crashID = (intptr_t)info.ExceptionPointers->ExceptionRecord->ExceptionAddress;
		char hex[MAX_PATH];
		sprintf_s(hex, "%x", crashID);
		string address = hex;

		PrintDebug("Get fault module name...\n");

		string dllName = "";
		HMODULE handle;

		if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)crashID, &handle))
		{
			char path[MAX_PATH];
			if (GetModuleFileNameA(handle, path, MAX_PATH))
			{
				auto filename = PathFindFileNameA(path);
				dllName = filename;
			}
		}

		string fullMsg = "";
		SetErrorMessage(fullMsg, address, dllName, crashID);
		CopyAndRename_ModLoaderIni(); //copy ModLoaderIni file to the Crash Dump folder so we know what mod and cheat were used
		string text = "Crash Address: " + address + "\n";
		PrintDebug("\nFault module name: %s \n", dllName.c_str());
		PrintDebug(text.c_str());

		PrintDebug("Crash Dump Done.\n");
		MessageBoxA(0, fullMsg.c_str(), "SADX Error", MB_ICONERROR);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

void initCrashDump()
{
	SetUnhandledExceptionFilter(HandleException);
}