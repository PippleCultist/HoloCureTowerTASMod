#include <Aurie/shared.hpp>
#include <YYToolkit/YYTK_Shared.hpp>
#include "ScriptFunctions.h"
#include "CommonFunctions.h"
#include "CodeEvents.h"

extern YYTKInterface* g_ModuleInterface;
extern CallbackManagerInterface* callbackManagerInterfacePtr;
extern PFUNC_YYGMLScript origConfirmedHoloHouseManagerCreateScript;
extern std::unordered_map<int, TASCommand> tasCommandMap;
extern int curTASFrame;
extern int maxTASFrame;
extern int maxOverrideFrame;
extern int objPlayerPlatformerIndex;
extern int objHoloHouseManagerIndex;
extern bool isInTower;
extern bool isRunningTAS;
extern bool isRunningOverride;

std::unordered_map<int, TASCommand> overrideCommandMap;

RValue& ConfirmedHoloHouseManagerCreateBefore(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue canControl = g_ModuleInterface->CallBuiltin("variable_instance_get", { Self, "canControl" });
	RValue quitConfirm = g_ModuleInterface->CallBuiltin("variable_instance_get", { Self, "quitConfirm" });
	RValue towerMode = g_ModuleInterface->CallBuiltin("variable_instance_get", { Self, "towerMode" });
	RValue towerWin = g_ModuleInterface->CallBuiltin("variable_instance_get", { Self, "towerWin" });
	RValue pauseOption = g_ModuleInterface->CallBuiltin("variable_instance_get", { Self, "pauseOption" });
	int pauseOptionNum = static_cast<int>(lround(pauseOption.ToDouble()));
	if (isInTower)
	{
		if (pauseOptionNum == 1)
		{
			if (!isRunningTAS)
			{
				// Check for override file and see if it can be loaded
				maxOverrideFrame = -1;
				parseTASCommands(overrideCommandMap, maxOverrideFrame, "TASRun/overrideData.txt");
				if (maxOverrideFrame != -1)
				{
					curTASFrame = maxTASFrame + 1;
					RValue playerPlatformer = g_ModuleInterface->CallBuiltin("instance_find", { objPlayerPlatformerIndex, 0 });
					RValue towerCheckPoint = g_ModuleInterface->CallBuiltin("variable_global_get", { "towerCheckPoint" });
					setInstanceVariable(playerPlatformer, GML_x, towerCheckPoint[0]);
					setInstanceVariable(playerPlatformer, GML_y, towerCheckPoint[1]);
					setInstanceVariable(playerPlatformer, GML_canControl, true);
					setInstanceVariable(Self, GML_paused, false);
					isRunningOverride = true;
					callbackManagerInterfacePtr->CancelOriginalFunction();
				}
			}
		}
		else if (pauseOptionNum == 3)
		{
			callbackManagerInterfacePtr->CancelOriginalFunction();
		}
	}
	return ReturnValue;
}

bool isRightPressed = false;
bool isLeftPressed = false;

RValue& InputCheckBefore(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	if (isInTower)
	{
		RValue holoHouseManager = g_ModuleInterface->CallBuiltin("instance_find", { objHoloHouseManagerIndex, 0 });
		RValue paused = getInstanceVariable(holoHouseManager, GML_paused);
		if (paused.ToBoolean())
		{
			callbackManagerInterfacePtr->CancelOriginalFunction();
			return ReturnValue;
		}
	}

	if (!isRunningTAS && !isRunningOverride)
	{
		return ReturnValue;
	}

	std::unordered_map<int, TASCommand>* commandMap = nullptr;

	if (isRunningTAS)
	{
		commandMap = &tasCommandMap;
	}
	else if (isRunningOverride)
	{
		commandMap = &overrideCommandMap;
	}

	if (Args[0]->ToString().compare("right") == 0)
	{
		if (commandMap->find(curTASFrame) != commandMap->end())
		{
			if ((*commandMap)[curTASFrame].TASCommandTypePressRight)
			{
				isRightPressed = true;
			}
			else if ((*commandMap)[curTASFrame].TASCommandTypeReleaseRight)
			{
				isRightPressed = false;
			}
		}
		ReturnValue = isRightPressed;
		callbackManagerInterfacePtr->CancelOriginalFunction();
	}
	else if (Args[0]->ToString().compare("left") == 0)
	{
		if (commandMap->find(curTASFrame) != commandMap->end())
		{
			if ((*commandMap)[curTASFrame].TASCommandTypePressLeft)
			{
				isLeftPressed = true;
			}
			else if ((*commandMap)[curTASFrame].TASCommandTypeReleaseLeft)
			{
				isLeftPressed = false;
			}
		}
		ReturnValue = isLeftPressed;
		callbackManagerInterfacePtr->CancelOriginalFunction();
	}
	return ReturnValue;
}

RValue& InputCheckPressedBefore(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	if (!isRunningTAS && !isRunningOverride)
	{
		return ReturnValue;
	}
	
	std::unordered_map<int, TASCommand>* commandMap = nullptr;

	if (isRunningTAS)
	{
		commandMap = &tasCommandMap;
	}
	else if (isRunningOverride)
	{
		commandMap = &overrideCommandMap;
	}

	bool hasTASCommand = false;
	if (Args[0]->ToString().compare("actionOne") == 0)
	{
		ReturnValue = (*commandMap)[curTASFrame].TASCommandTypePressCharge;
		hasTASCommand = true;
	}
	else if (Args[0]->ToString().compare("left") == 0)
	{
		ReturnValue = (*commandMap)[curTASFrame].TASCommandTypePressLeft;
		hasTASCommand = true;
	}
	else if (Args[0]->ToString().compare("right") == 0)
	{
		ReturnValue = (*commandMap)[curTASFrame].TASCommandTypePressRight;
		hasTASCommand = true;
	}
	if (hasTASCommand)
	{
		callbackManagerInterfacePtr->CancelOriginalFunction();
	}
	return ReturnValue;
}

RValue& InputCheckReleasedBefore(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	if (!isRunningTAS && !isRunningOverride)
	{
		return ReturnValue;
	}

	std::unordered_map<int, TASCommand>* commandMap = nullptr;

	if (isRunningTAS)
	{
		commandMap = &tasCommandMap;
	}
	else if (isRunningOverride)
	{
		commandMap = &overrideCommandMap;
	}

	bool hasTASCommand = false;
	if (Args[0]->ToString().compare("actionOne") == 0)
	{
		ReturnValue = (*commandMap)[curTASFrame].TASCommandTypeReleaseCharge;
		hasTASCommand = true;
	}
	else if (Args[0]->ToString().compare("left") == 0)
	{
		ReturnValue = (*commandMap)[curTASFrame].TASCommandTypeReleaseLeft;
		hasTASCommand = true;
	}
	else if (Args[0]->ToString().compare("right") == 0)
	{
		ReturnValue = (*commandMap)[curTASFrame].TASCommandTypeReleaseRight;
		hasTASCommand = true;
	}
	if (hasTASCommand)
	{
		callbackManagerInterfacePtr->CancelOriginalFunction();
	}
	return ReturnValue;
}

RValue& DoAchievementBefore(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	callbackManagerInterfacePtr->CancelOriginalFunction();
	return ReturnValue;
}