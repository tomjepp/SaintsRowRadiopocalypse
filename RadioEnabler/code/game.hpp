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

#pragma once

#define GAMEVER_PRERELEASE -1
#define GAMEVER_STEAM_PATCH1 1
#define GAMEVER_STEAM_PATCH2 2
#define GAMEVER_GOG 100

BOOL GameAttach(void);
void GameDetach(void);
BOOL HookGame(void);
BOOL HookGame_Prerelease(void);
BOOL HookGame_SteamPatch1(void);
BOOL HookGame_SteamPatch2(void);
BOOL HookGame_GOG(void);