#pragma once
#include "ModuleMain.h"
#include <Aurie/shared.hpp>
#include <YYToolkit/YYTK_Shared.hpp>

struct TASCommand
{
	bool TASCommandTypePressCharge;
	bool TASCommandTypeReleaseCharge;
	bool TASCommandTypePressLeft;
	bool TASCommandTypeReleaseLeft;
	bool TASCommandTypePressRight;
	bool TASCommandTypeReleaseRight;
	bool TASCommandTypeSetCheckpoint;
	bool TASCommandTypeLoadCheckpoint;

	TASCommand() : TASCommandTypePressCharge(false), TASCommandTypeReleaseCharge(false), TASCommandTypePressLeft(false),
		TASCommandTypeReleaseLeft(false), TASCommandTypePressRight(false), TASCommandTypeReleaseRight(false), TASCommandTypeSetCheckpoint(false),
		TASCommandTypeLoadCheckpoint(false)
	{
	}
};

void parseTASCommands(std::unordered_map<int, TASCommand>& commandMap, int& maxFrame, std::string fileName);

void PlayerPlatformerCreateAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void InputManagerStepBefore(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void InputManagerStepAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void HoloHouseManagerStepBefore(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void HoloHouseManagerStepAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);