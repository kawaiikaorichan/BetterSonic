#include "stdafx.h"
#include "Trampoline.h"
#include "uiscale.h"
#include "bgscale.h"

using namespace uiscale;

static Trampoline* TutorialBackground_Display_t;
static Trampoline* RecapBackground_Main_t;
static Trampoline* EndBG_Display_t;
static Trampoline* DrawTiledBG_AVA_BACK_t;
static Trampoline* DrawAVA_TITLE_BACK_t;
static Trampoline* DisplayLogoScreen_t;
static Trampoline* DisplayFadePoly2_t;
static Trampoline* dsDispFadeWhite_t;
static Trampoline* dsDispFadeLoading_t;
static Trampoline* DispTitle_t;
static Trampoline* DisplayTitle_ss;
static Trampoline* DisplaySelectingCharacter_t;
static Trampoline* DisplayHint1_t;
static Trampoline* DisplayHint2_t;

static void __cdecl DrawAVA_TITLE_BACK_r(float depth)
{
	scale_trampoline(Align::Align_Center, true, DrawAVA_TITLE_BACK_r, DrawAVA_TITLE_BACK_t, depth);
}

static void __cdecl DrawTiledBG_AVA_BACK_r(float depth)
{
	scale_trampoline(Align::Align_Center, true, DrawTiledBG_AVA_BACK_r, DrawTiledBG_AVA_BACK_t, depth);
}

static void __cdecl RecapBackground_Main_r(ObjectMaster* a1)
{
	scale_trampoline(Align::Align_Center, true, RecapBackground_Main_r, RecapBackground_Main_t, a1);
}

static void __cdecl DisplayLogoScreen_r(Uint8 index)
{
	scale_trampoline(Align::Align_Center, true, DisplayLogoScreen_r, DisplayLogoScreen_t, index);
}

static void __cdecl TutorialBackground_Display_r(ObjectMaster* a1)
{
	auto orig = bg_fill;
	bg_fill = FillMode_Fit;
	scale_trampoline(Align::Align_Center, true, TutorialBackground_Display_r, TutorialBackground_Display_t, a1);
	bg_fill = orig;
}

static void __cdecl EndBG_Display_r(ObjectMaster* a1)
{
	auto orig = bg_fill;
	bg_fill = FillMode_Fit;
	scale_trampoline(Align::Align_Center, true, EndBG_Display_r, EndBG_Display_t, a1);
	bg_fill = orig;
}

static void __cdecl DisplayFadePoly2_r()
{
	scale_trampoline(Align::Align_Center, true, DisplayFadePoly2_r, DisplayFadePoly2_t);
}

static void __cdecl dsDispFadeWhite_r()
{
	scale_trampoline(Align::Align_Center, true, dsDispFadeWhite_r, dsDispFadeWhite_t);
}

static void __cdecl DispTitle_r()
{
	scale_trampoline(Align::Align_Center, true, DispTitle_r, DispTitle_t);
}

static void __cdecl DispTitle2_r()
{
	scale_trampoline(Align::Align_Center, true, DispTitle2_r, DisplayTitle_ss);
}

static void __cdecl dsDispFadeLoading_r()
{
	scale_trampoline(Align::Align_Center, true, dsDispFadeLoading_r, dsDispFadeLoading_t);
}

static void __cdecl DisplaySelectingCharacter_r()
{
	scale_trampoline(Align::Align_Center, true, DisplaySelectingCharacter_r, DisplaySelectingCharacter_t);
}

static void __cdecl DisplayHint1_r()
{
	scale_trampoline(Align::Align_Center, true, DisplayHint1_r, DisplayHint1_t);
}

static void __cdecl DisplayHint2_r()
{
	scale_trampoline(Align::Align_Center, true, DisplayHint2_r, DisplayHint2_t);
}

void bgscale::initialize()
{
	DispTitle_t                  = new Trampoline(0x0042D880, 0x0042D889, DispTitle_r); // TGS title screen
	DisplayTitle_ss              = new Trampoline(0x0042DD50, 0x0042DD58, DispTitle2_r); // TGS title screen (level select)
	DisplayHint1_t               = new Trampoline(0x0042DA40, 0x0042DA4A, DisplayHint1_r); // TGS character select hint
	DisplayHint2_t               = new Trampoline(0x0042DAC0, 0x0042DACA, DisplayHint2_r); // TGS character select hint
	dsDispFadeWhite_t            = new Trampoline(0x0042E4E0, 0x0042E4E6, dsDispFadeWhite_r); // TGS fade white
	dsDispFadeLoading_t          = new Trampoline(0x0042E5F0, 0x0042E5F9, dsDispFadeLoading_r); // TGS fade with "Loading"
	DisplaySelectingCharacter_t  = new Trampoline(0x0042D8F0, 0x0042D8F5, DisplaySelectingCharacter_r); // TGS character select tutorial 1
	DisplayFadePoly2_t           = new Trampoline(0x005133C0, 0x005133C7, DisplayFadePoly2_r); // Egg Carrier FMV fadeout
	TutorialBackground_Display_t = new Trampoline(0x006436B0, 0x006436B7, TutorialBackground_Display_r);
	RecapBackground_Main_t       = new Trampoline(0x00643C90, 0x00643C95, RecapBackground_Main_r);
	EndBG_Display_t              = new Trampoline(0x006414A0, 0x006414A7, EndBG_Display_r);
	DrawTiledBG_AVA_BACK_t       = new Trampoline(0x00507BB0, 0x00507BB5, DrawTiledBG_AVA_BACK_r);
	DrawAVA_TITLE_BACK_t         = new Trampoline(0x0050BA90, 0x0050BA96, DrawAVA_TITLE_BACK_r);
	DisplayLogoScreen_t          = new Trampoline(0x0042CB20, 0x0042CB28, DisplayLogoScreen_r);
}
