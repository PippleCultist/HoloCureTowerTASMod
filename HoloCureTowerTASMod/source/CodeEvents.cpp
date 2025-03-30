#include "CodeEvents.h"
#include "CommonFunctions.h"
#include <fstream>

extern int objInputManagerIndex;
extern int objPlayerPlatformerIndex;
extern int objHoloHouseManagerIndex;
extern CallbackManagerInterface* callbackManagerInterfacePtr;
extern YYTKInterface* g_ModuleInterface;
extern PFUNC_YYGMLScript origConfirmedHoloHouseManagerCreateScript;
extern PFUNC_YYGMLScript origSetCheckpointHoloHouseManagerCreateScript;

bool hasEnteredTower = false;
bool isRunningTAS = false;
bool isInTower = false;
bool isOnGround = true;
bool isRunningOverride = false;
int hasEnteredTowerCD = -1;

std::unordered_map<int, TASCommand> tasCommandMap;
int curTASFrame = 0;
int maxTASFrame = 0;
int maxOverrideFrame = 0;

void PlayerPlatformerCreateAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	if (!hasEnteredTower)
	{
		hasEnteredTower = true;
		hasEnteredTowerCD = 5;
	}
	else
	{
		hasEnteredTower = false;
	}
}

void InputManagerStepBefore(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	if (isInTower)
	{
		RValue playerPlatformer = g_ModuleInterface->CallBuiltin("instance_find", { objPlayerPlatformerIndex, 0 });
		RValue onGround = getInstanceVariable(playerPlatformer, GML_onGround);
		if (isOnGround && !onGround.ToBoolean())
		{
			isOnGround = false;
		}
		else if (!isOnGround && onGround.ToBoolean())
		{
			callbackManagerInterfacePtr->LogToFile(MODNAME, "Landed on ground on frame %d", curTASFrame);
			g_ModuleInterface->Print(CM_BLUE, "Landed on ground on frame %d", curTASFrame);
			isOnGround = true;
		}
	}
}

void InputManagerStepAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	if (isRunningTAS)
	{
		curTASFrame++;
		if (curTASFrame > maxTASFrame)
		{
			isRunningTAS = false;
		}
	}
	else if (isRunningOverride)
	{
		curTASFrame++;
		if (curTASFrame > maxOverrideFrame)
		{
			isRunningOverride = false;
		}
	}
}

void HoloHouseManagerStepBefore(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	if (isRunningTAS)
	{
		CInstance* Self = std::get<0>(Args);
		if (tasCommandMap[curTASFrame].TASCommandTypeSetCheckpoint)
		{
			RValue result;
			origSetCheckpointHoloHouseManagerCreateScript(Self, nullptr, result, 0, nullptr);
		}
		if (tasCommandMap[curTASFrame].TASCommandTypeLoadCheckpoint)
		{
			RValue playerPlatformer = g_ModuleInterface->CallBuiltin("instance_find", { objPlayerPlatformerIndex, 0 });
			RValue towerCheckPoint = g_ModuleInterface->CallBuiltin("variable_global_get", { "towerCheckPoint" });
			setInstanceVariable(playerPlatformer, GML_x, towerCheckPoint[0]);
			setInstanceVariable(playerPlatformer, GML_y, towerCheckPoint[1]);
		}
	}
}

void parseTASCommands(std::unordered_map<int, TASCommand>& commandMap, int& maxFrame, std::string fileName)
{
	CreateDirectory(L"TASRun", NULL);
	std::ifstream inFile;
	inFile.open(fileName);
	std::string line;
	commandMap.clear();
	while (std::getline(inFile, line))
	{
		if (line.empty())
		{
			continue;
		}
		std::string frameNum;
		std::string action;
		std::istringstream stream(line);
		std::getline(stream, frameNum, ' ');
		std::getline(stream, action, ' ');
		int num = atoi(frameNum.c_str());
		if (action.compare("pressCharge") == 0)
		{
			commandMap[num].TASCommandTypePressCharge = true;
		}
		else if (action.compare("releaseCharge") == 0)
		{
			commandMap[num].TASCommandTypeReleaseCharge = true;
		}
		else if (action.compare("pressLeft") == 0)
		{
			commandMap[num].TASCommandTypePressLeft = true;
		}
		else if (action.compare("releaseLeft") == 0)
		{
			commandMap[num].TASCommandTypeReleaseLeft = true;
		}
		else if (action.compare("pressRight") == 0)
		{
			commandMap[num].TASCommandTypePressRight = true;
		}
		else if (action.compare("releaseRight") == 0)
		{
			commandMap[num].TASCommandTypeReleaseRight = true;
		}
		else if (action.compare("jumpRight") == 0)
		{
			std::string jumpDuration;
			std::getline(stream, jumpDuration, ' ');
			int duration = atoi(jumpDuration.c_str());
			commandMap[num].TASCommandTypePressRight = true;
			commandMap[num + duration + 2].TASCommandTypeReleaseRight = true;
			commandMap[num + 1].TASCommandTypePressCharge = true;
			commandMap[num + duration + 1].TASCommandTypeReleaseCharge = true;
			maxFrame = max(maxFrame, num + duration + 2);
		}
		else if (action.compare("jumpLeft") == 0)
		{
			std::string jumpDuration;
			std::getline(stream, jumpDuration, ' ');
			int duration = atoi(jumpDuration.c_str());
			commandMap[num].TASCommandTypePressLeft = true;
			commandMap[num + duration + 2].TASCommandTypeReleaseLeft = true;
			commandMap[num + 1].TASCommandTypePressCharge = true;
			commandMap[num + duration + 1].TASCommandTypeReleaseCharge = true;
			maxFrame = max(maxFrame, num + duration + 2);
		}
		else if (action.compare("setCheckpoint") == 0)
		{
			commandMap[num].TASCommandTypeSetCheckpoint = true;
		}
		else if (action.compare("loadCheckpoint") == 0)
		{
			commandMap[num].TASCommandTypeLoadCheckpoint = true;
		}
		maxFrame = max(maxFrame, num);
	}
}

void HoloHouseManagerStepAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	if (hasEnteredTower)
	{
		if (hasEnteredTowerCD == 0)
		{
			CInstance* Self = std::get<0>(Args);
			setInstanceVariable(Self, GML_canControl, true);
			setInstanceVariable(Self, GML_paused, true);
			setInstanceVariable(Self, GML_towerMode, true);
			setInstanceVariable(Self, GML_towerWin, false);
			setInstanceVariable(Self, GML_pauseOption, 1.0);
			RValue result;
			origConfirmedHoloHouseManagerCreateScript(Self, nullptr, result, 0, nullptr);
			hasEnteredTowerCD = -1;
			curTASFrame = 0;
			maxTASFrame = 0;
			isInTower = true;
			isRunningTAS = true;
			isOnGround = true;
			isRunningOverride = false;
			maxOverrideFrame = 0;
			parseTASCommands(tasCommandMap, maxTASFrame, "TASRun/runData.txt");
		}
		else
		{
			hasEnteredTowerCD--;
		}
	}
}