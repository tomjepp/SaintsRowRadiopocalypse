/* Copyright (c) 2011 Rick (rick 'at' gibbed 'dot' us)
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would
 *    be appreciated but is not required.
 * 
 * 2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 * 
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 */

#pragma unmanaged

#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>
#include <string.h>

#include "game.hpp"
#include "patch.hpp"
#include "gamedetect.hpp"
#include "lua.hpp"
#include "Log.hpp"
#include "PerFrame.hpp"
#include "config.hpp"
#include "DebugWindow.hpp"

static unsigned char radioFixup_Prerelease[] = {
	0x8A, 0x45, 0x08, // MOV al, [ebp+enable]
	0xA2, 0x6C, 0x7E, 0x79, 0x01, // MOV onfoot_radio_enabled, al
	0x90, 0x90, 0x90, 0x90, 0x90 // reserved for inserted JMP
};

static unsigned char radioFixup_Patch1[] = {
	0x8A, 0x45, 0x08, // MOV al, [ebp+enable]
	0xA2, 0x1C, 0xEC, 0x66, 0x01, // MOV onfoot_radio_enabled, al
	0x90, 0x90, 0x90, 0x90, 0x90 // reserved for inserted JMP
};

static unsigned char radioFixup_Patch2[] = {
	0x8A, 0x45, 0x08, // MOV al, [ebp+enable]
	0xA2, 0x1C, 0xEC, 0x66, 0x01, // MOV onfoot_radio_enabled, al
	0x90, 0x90, 0x90, 0x90, 0x90 // reserved for inserted JMP
};

static unsigned char radioFixup_GOG[] = {
	0x8A, 0x45, 0x08, // MOV al, [ebp+enable]
	0xA2, 0x4C, 0x0E, 0x74, 0x01, // MOV onfoot_radio_enabled, al
	0x90, 0x90, 0x90, 0x90, 0x90 // reserved for inserted JMP
};

BOOL HookGame(void)
{
	switch (SRVersion)
	{
		case GAMEVER_PRERELEASE:
			return HookGame_Prerelease();
			break;

		case GAMEVER_STEAM_PATCH1: // SR: GOOH Steam Patch 1
			return HookGame_SteamPatch1();
			break;
		case GAMEVER_STEAM_PATCH2: // SR: GOOH Steam Patch 2
			return HookGame_SteamPatch2();
			break;

		case GAMEVER_GOG: // GOG original release? (RE'd 2017-05-11)
			return HookGame_GOG();
			break;
	}

	return false;
}

BOOL HookGame_Prerelease(void)
{
	WriteToLog(L"HookGame", L"Loading hooks and patches for prerelease build:\n");
	BOOL success = false;

	WriteToLog(L"HookGame", L" - hooking game loop...\n");
	game_do_frame = (GAME_DO_FRAME)0x00789B40;
	success = PatchCall(0x0078C355, (unsigned int)&perFrameHook);
	if (!success)
		return false;

	Option_PauseOnFocusLost = (unsigned char*)0x017B33BC;

	if (HookLuaDebugPrint)
	{
		WriteToLog(L"HookGame", L" - setting up Lua hooks...\n");
		unsigned int luaDebugPrintAddress = (unsigned int)&Lua_DebugPrint;
		PatchCode(0x1178C30, &luaDebugPrintAddress, 4);
		lua_gettop = (LUA_GETTOP)0x012D1B00;
		lua_tolstring = (LUA_TOLSTRING)0x012D1FD0;
	}

	if (EnableRadio)
	{
		WriteToLog(L"HookGame", L" - patching radio...\n");

		DWORD old = 0;

		success = PatchJump(0x005DB2DB, (unsigned int)&radioFixup_Prerelease);
		if (!success)
			return false;

		success = PatchJump(((unsigned int)&radioFixup_Prerelease) + 0x08, 0x005DB2E2);
		if (!success)
			return false;

		success = VirtualProtect(&radioFixup_Prerelease, sizeof(radioFixup_Prerelease), PAGE_EXECUTE_READWRITE, &old);
		if (!success)
			return false;
	}

	if (DisableLoadingSRIVCharacter)
	{
		WriteToLog(L"HookGame", L" - disabling loading SRIV character...\n");
		success = PatchJump(0x00E2E828, 0x00E2E889);
		if (!success)
			return false;
	}

	if (PreventCharacterSwapOnCoopJoin)
	{
		WriteToLog(L"HookGame", L" - preventing character swap on co-op join...\n");
		success = PatchJump(0x00C0B9DF, 0x00C0B9F3);
		if (!success)
			return false;
	}

	WriteToLog(L"HookGame", L" - hooking window creation...\n");
	PatchCodeByte(0x011BCD10, 0x90); // Insert NOP
	success = PatchCall(0x011BCD11, (unsigned int)&SetForegroundWindow_Hook);
	if (!success)
		return false;

	return true;
}

BOOL HookGame_SteamPatch1(void)
{
	WriteToLog(L"HookGame", L"Loading hooks and patches for Steam patch #1:\n");
	BOOL success = false;
	
	WriteToLog(L"HookGame", L" - hooking game loop...\n");
	game_do_frame = (GAME_DO_FRAME)0x7133f0;
	success = PatchCall(0x00715B2C, (unsigned int)&perFrameHook);
	if (!success)
		return false;

	Option_PauseOnFocusLost = (unsigned char*)0x168A174;

	if (HookLuaDebugPrint)
	{
		WriteToLog(L"HookGame", L" - setting up Lua hooks...\n");
		unsigned int luaDebugPrintAddress = (unsigned int)&Lua_DebugPrint;
		PatchCode(0x1010020, &luaDebugPrintAddress, 4);
		lua_gettop = (LUA_GETTOP)0x109c820;
		lua_tolstring = (LUA_TOLSTRING)0x109cc10;
	}

	if (EnableRadio)
	{
		WriteToLog(L"HookGame", L" - patching radio...\n");

		DWORD old = 0;

		success = PatchJump(0x00591E2B, (unsigned int)&radioFixup_Patch1);
		if (!success)
			return false;

		success = PatchJump(((unsigned int)&radioFixup_Patch1) + 0x08, 0x00591E32);
		if (!success)
			return false;

		success = VirtualProtect(&radioFixup_Patch1, sizeof(radioFixup_Patch1), PAGE_EXECUTE_READWRITE, &old);
		if (!success)
			return false;
	}

	if (DisableLoadingSRIVCharacter)
	{
		WriteToLog(L"HookGame", L" - disabling loading SRIV character...\n");
		success = PatchJump(0x00D16F48, 0x00D16FA9);
		if (!success)
			return false;
	}

	if (PreventCharacterSwapOnCoopJoin)
	{
		WriteToLog(L"HookGame", L" - preventing character swap on co-op join...\n");
		success = PatchJump(0x00B23237, 0xB23244);
		if (!success)
			return false;
	}

	return true;
}

BOOL HookGame_SteamPatch2(void)
{
	WriteToLog(L"HookGame", L"Loading hooks and patches for Steam patch #2:\n");
	BOOL success = false;

	WriteToLog(L"HookGame", L" - hooking game loop...\n");
	game_do_frame = (GAME_DO_FRAME)0x712D70;
	success = PatchCall(0x0071549C, (unsigned int)&perFrameHook);
	if (!success)
		return false;

	Option_PauseOnFocusLost = (unsigned char*)0x168A1B4;

	if (HookLuaDebugPrint)
	{
		WriteToLog(L"HookGame", L" - setting up Lua hooks...\n");
		unsigned int luaDebugPrintAddress = (unsigned int)&Lua_DebugPrint;
		PatchCode(0x0100FEE0, &luaDebugPrintAddress, 4);
		lua_gettop = (LUA_GETTOP)0x0109C640;
		lua_tolstring = (LUA_TOLSTRING)0x0109CA30;
	}

	if (EnableRadio)
	{
		WriteToLog(L"HookGame", L" - patching radio...\n");

		DWORD old = 0;

		success = PatchJump(0x00591BEB, (unsigned int)&radioFixup_Patch2);
		if (!success)
			return false;

		success = PatchJump(((unsigned int)&radioFixup_Patch2) + 0x08, 0x00591BF2);
		if (!success)
			return false;

		success = VirtualProtect(&radioFixup_Patch2, sizeof(radioFixup_Patch2), PAGE_EXECUTE_READWRITE, &old);
		if (!success)
			return false;
	}

	if (DisableLoadingSRIVCharacter)
	{
		WriteToLog(L"HookGame", L" - disabling loading SRIV character...\n");
		success = PatchJump(0x00D16D98, 0x00D16DF9);
		if (!success)
			return false;
	}

	if (PreventCharacterSwapOnCoopJoin)
	{
		WriteToLog(L"HookGame", L" - preventing character swap on co-op join...\n");
		success = PatchJump(0x00B22F17, 0x00B22F24);
		if (!success)
			return false;
	}

	WriteToLog(L"HookGame", L" - hooking window creation...\n");
	PatchCodeByte(0x00FB5E50, 0x90); // Insert NOP
	success = PatchCall(0x00FB5E51, (unsigned int)&SetForegroundWindow_Hook);
	if (!success)
		return false;

	return true;
}

BOOL HookGame_GOG(void)
{
	WriteToLog(L"HookGame", L"Loading hooks and patches for GOG:\n");
	BOOL success = false;

	
	WriteToLog(L"HookGame", L" - hooking game loop...\n");
	game_do_frame = (GAME_DO_FRAME)0x00744BC0;
	success = PatchCall(0x07473C5, (unsigned int)&perFrameHook);
	if (!success)
		return false;

	Option_PauseOnFocusLost = (unsigned char*)0x0175C4E4;
	
	if (HookLuaDebugPrint)
	{
		WriteToLog(L"HookGame", L" - setting up Lua hooks...\n");
		unsigned int luaDebugPrintAddress = (unsigned int)&Lua_DebugPrint;
		PatchCode(0x1131C10, &luaDebugPrintAddress, 4);
		lua_gettop = (LUA_GETTOP)0x1289EC0;
		lua_tolstring = (LUA_TOLSTRING)0x0128A390;
	}

	
	if (EnableRadio)
	{
		WriteToLog(L"HookGame", L" - patching radio...\n");

		DWORD old = 0;

		success = PatchJump(0x005A0C6B, (unsigned int)&radioFixup_GOG);
		if (!success)
			return false;

		success = PatchJump(((unsigned int)&radioFixup_GOG) + 0x08, 0x005A0C72);
		if (!success)
			return false;

		success = VirtualProtect(&radioFixup_GOG, sizeof(radioFixup_GOG), PAGE_EXECUTE_READWRITE, &old);
		if (!success)
			return false;
	
	}

	/*
	 * This seems to be ripped out of the GOG release
	if (DisableLoadingSRIVCharacter)
	{
		WriteToLog(L"HookGame", L" - disabling loading SRIV character...\n");
		success = PatchJump(0x00E2E828, 0x00E2E889);
		if (!success)
			return false;
	}
	*/

	if (PreventCharacterSwapOnCoopJoin)
	{
		WriteToLog(L"HookGame", L" - preventing character swap on co-op join...\n");
		success = PatchJump(0x00BAAA47, 0x00BAAA54);
		if (!success)
			return false;
	}

	WriteToLog(L"HookGame", L" - hooking window creation...\n");
	PatchCodeByte(0x01222940, 0x90); // Insert NOP
	success = PatchCall(0x01222941, (unsigned int)&SetForegroundWindow_Hook);
	if (!success)
		return false;

	return true;
}

BOOL GameAttach(void)
{
	VersionDetect();

	return HookGame();
}

void GameDetach(void)
{
}
