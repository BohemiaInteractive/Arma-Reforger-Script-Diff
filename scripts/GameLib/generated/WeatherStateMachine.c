/*
===========================================
Do not modify, this script is generated
===========================================
*/

class WeatherStateMachine: ScriptAndConfig
{
	proto external ref WeatherState GetState(int index);

	proto external void GetStatesList(out notnull array<ref WeatherState> states);
	proto external int GetNumStates();
	proto external string GetStartStateName();
	proto external int GetStartStateIndex();
}
