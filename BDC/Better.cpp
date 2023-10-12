#include "pch.h"
#include "SSDCtexlist.h"
#pragma warning(disable : 4996)

#define ReplacePVM(a, b) helperFunctions.ReplaceFile("system\\" a ".PVM", "system\\" b ".PVM");

HelperFunctions HelperFunctionsGlobal;

wchar_t* ConvertCharToWChar(const char* value)
{
	const size_t cSize = strlen(value) + 1;
	wchar_t* wcharVariable = new wchar_t[cSize];
	mbstowcs(wcharVariable, value, cSize);
	return wcharVariable;
}

void ReplaceCharacter(const char* iniFile, const char* modDirectory, const HelperFunctions& helper)
{
	std::string iniFilePathString = modDirectory + (std::string)"\\" + iniFile + ".ini";

	const char* iniFilePathChar = iniFilePathString.c_str();
	wchar_t* iniFileFullPath = ConvertCharToWChar(iniFilePathChar);
	wchar_t* modPath = ConvertCharToWChar(modDirectory);

	helper.LoadDLLData(iniFileFullPath, modPath);
	delete[] iniFileFullPath;
	delete[] modPath;
}

extern "C"
{
	__declspec(dllexport) void __cdecl Init(const char* path, const HelperFunctions& helperFunctions)
	{
		HMODULE DCcharacters = GetModuleHandle(L"SA1_Chars");
		
		if (DCcharacters)
		{
			ReplaceCharacter("sonic", path, helperFunctions);
			std::string fullPath = path + (std::string)"\\indices.ini";
			helperFunctions.RegisterCharacterWelds(Characters_Sonic, fullPath.c_str());
			WriteData<1>((int*)0x493730, 0xC3);
			WriteData((NJS_TEXLIST**)0x55E65C, SSAura01);
			WriteData((NJS_TEXLIST**)0x55E751, SSAura01);
			WriteData((NJS_TEXLIST**)0x55E712, SSAura02);
			WriteData((NJS_TEXLIST**)0x55E7CD, SSWaterThing);
			WriteData((NJS_TEXLIST**)0x55F2B3, SSHomingTex1);
			WriteData((NJS_TEXLIST**)0x55F1D1, SSHomingTex1);
			WriteData((NJS_TEXLIST**)0x55F1DC, SSHomingTex2);
			WriteData((NJS_TEXLIST**)0x55F2BE, SSHomingTex2);
			WriteData((NJS_TEXLIST**)0x55F677, SSHomingTex2);
			WriteData((NJS_TEXLIST**)0x55F669, SSHomingTex3);
			SUPERSONIC_TEXLIST = SS_PVM;
			ReplacePVM("sonic", "sonic_dc");
			ReplacePVM("supersonic", "supersonic_dc");
		}
	}

	__declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer };
}