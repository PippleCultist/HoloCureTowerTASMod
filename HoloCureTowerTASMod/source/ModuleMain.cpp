#include <Aurie/shared.hpp>
#include <YYToolkit/YYTK_Shared.hpp>
#include <CallbackManager/CallbackManagerInterface.h>
#include "ModuleMain.h"
#include "CodeEvents.h"
#include "ScriptFunctions.h"
#include "BuiltinFunctions.h"

using namespace Aurie;
using namespace YYTK;

RValue GMLVarIndexMapGMLHash[1001];

TRoutine origStructGetFromHashFunc;
TRoutine origStructSetFromHashFunc;

CallbackManagerInterface* callbackManagerInterfacePtr = nullptr;
YYTKInterface* g_ModuleInterface = nullptr;
YYRunnerInterface g_RunnerInterface;

PFUNC_YYGMLScript origConfirmedHoloHouseManagerCreateScript = nullptr;
PFUNC_YYGMLScript origSetCheckpointHoloHouseManagerCreateScript = nullptr;

CInstance* globalInstance = nullptr;

int objInputManagerIndex = -1;
int objPlayerPlatformerIndex = -1;
int objHoloHouseManagerIndex = -1;

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	AurieStatus status = AURIE_SUCCESS;
	status = ObGetInterface("callbackManager", (AurieInterfaceBase*&)callbackManagerInterfacePtr);
	if (!AurieSuccess(status))
	{
		printf("Failed to get callback manager interface. Make sure that CallbackManagerMod is located in the mods/Aurie directory.\n");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	// Gets a handle to the interface exposed by YYTK
	// You can keep this pointer for future use, as it will not change unless YYTK is unloaded.
	status = ObGetInterface(
		"YYTK_Main",
		(AurieInterfaceBase*&)(g_ModuleInterface)
	);

	// If we can't get the interface, we fail loading.
	if (!AurieSuccess(status))
	{
		printf("Failed to get YYTK Interface\n");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterBuiltinFunctionCallback(MODNAME, "struct_get_from_hash", nullptr, nullptr, &origStructGetFromHashFunc)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "struct_get_from_hash");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterBuiltinFunctionCallback(MODNAME, "struct_set_from_hash", nullptr, nullptr, &origStructSetFromHashFunc)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "struct_set_from_hash");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterBuiltinFunctionCallback(MODNAME, "draw_text", DrawTextBefore, nullptr, nullptr)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "draw_text");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_PlayerPlatformer_Create_0", nullptr, PlayerPlatformerCreateAfter)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "gml_Object_obj_PlayerPlatformer_Create_0");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_InputManager_Step_0", InputManagerStepBefore, InputManagerStepAfter)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "gml_Object_obj_InputManager_Step_0");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_HoloHouseManager_Step_0", HoloHouseManagerStepBefore, HoloHouseManagerStepAfter)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "gml_Object_obj_HoloHouseManager_Step_0");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterScriptFunctionCallback(MODNAME, "gml_Script_SetCheckPoint@gml_Object_obj_HoloHouseManager_Create_0", nullptr, nullptr, &origSetCheckpointHoloHouseManagerCreateScript)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "gml_Script_SetCheckPoint@gml_Object_obj_HoloHouseManager_Create_0");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterScriptFunctionCallback(MODNAME, "gml_Script_Confirmed@gml_Object_obj_HoloHouseManager_Create_0", ConfirmedHoloHouseManagerCreateBefore, nullptr, &origConfirmedHoloHouseManagerCreateScript)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "gml_Script_Confirmed@gml_Object_obj_HoloHouseManager_Create_0");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterScriptFunctionCallback(MODNAME, "gml_Script_input_check", InputCheckBefore, nullptr, nullptr)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "gml_Script_input_check");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterScriptFunctionCallback(MODNAME, "gml_Script_input_check_pressed", InputCheckPressedBefore, nullptr, nullptr)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "gml_Script_input_check_pressed");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterScriptFunctionCallback(MODNAME, "gml_Script_input_check_released", InputCheckReleasedBefore, nullptr, nullptr)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "gml_Script_input_check_released");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterScriptFunctionCallback(MODNAME, "gml_Script_DoAchievement", DoAchievementBefore, nullptr, nullptr)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "gml_Script_DoAchievement");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	g_RunnerInterface = g_ModuleInterface->GetRunnerInterface();
	g_ModuleInterface->GetGlobalInstance(&globalInstance);

	objInputManagerIndex = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "obj_InputManager" }).ToInt32());
	objPlayerPlatformerIndex = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "obj_PlayerPlatformer" }).ToInt32());
	objHoloHouseManagerIndex = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "obj_HoloHouseManager" }).ToInt32());

	for (int i = 0; i < std::extent<decltype(VariableNamesStringsArr)>::value; i++)
	{
		if (!AurieSuccess(status))
		{
			g_ModuleInterface->Print(CM_RED, "Failed to get hash for %s", VariableNamesStringsArr[i]);
		}
		GMLVarIndexMapGMLHash[i] = std::move(g_ModuleInterface->CallBuiltin("variable_get_hash", { VariableNamesStringsArr[i] }));
	}
	
	callbackManagerInterfacePtr->LogToFile(MODNAME, "Finished initializing");

	return AURIE_SUCCESS;
}