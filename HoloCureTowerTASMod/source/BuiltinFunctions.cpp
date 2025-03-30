#include "BuiltinFunctions.h"
#include "ModuleMain.h"

void DrawTextBefore(RValue& Result, CInstance* Self, CInstance* Other, int numArgs, RValue* Args)
{
	if (numArgs >= 3)
	{
		if (Args[2].ToString().compare("TIME") == 0)
		{
			Args[2] = std::string_view("TAS " + Args[2].ToString());
		}
	}
}