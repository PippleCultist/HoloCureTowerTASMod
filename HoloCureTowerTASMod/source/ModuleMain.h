#pragma once
#include <Aurie/shared.hpp>
#include <YYToolkit/YYTK_Shared.hpp>
#include <CallbackManager/CallbackManagerInterface.h>

#define VERSION_NUM "v1.0.3"
#define MODNAME "Holocure Tower TAS Mod " VERSION_NUM

#define SOME_ENUM(DO) \
	DO(moveLeftPressed) \
	DO(moveRightPressed) \
	DO(actionOnePressed) \
	DO(actionTwoPressed) \
	DO(actionThreePressed) \
	DO(moveLeftReleased) \
	DO(moveRightReleased) \
	DO(actionOneReleased) \
	DO(canControl) \
	DO(paused) \
	DO(pauseOption) \
	DO(towerMode) \
	DO(towerWin) \
	DO(onGround) \
	DO(x) \
	DO(y) \

#define MAKE_ENUM(VAR) GML_ ## VAR,
enum VariableNames
{
	SOME_ENUM(MAKE_ENUM)
};

#define MAKE_STRINGS(VAR) #VAR,
const char* const VariableNamesStringsArr[] =
{
	SOME_ENUM(MAKE_STRINGS)
};