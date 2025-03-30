#pragma once
#include "ModuleMain.h"
#include <Aurie/shared.hpp>
#include <YYToolkit/YYTK_Shared.hpp>

RValue& ConfirmedHoloHouseManagerCreateBefore(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args);
RValue& InputCheckBefore(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args);
RValue& InputCheckPressedBefore(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args);
RValue& InputCheckReleasedBefore(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args);
RValue& DoAchievementBefore(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args);