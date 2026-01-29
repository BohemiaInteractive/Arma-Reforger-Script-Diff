/*
===========================================
Do not modify, this script is generated
===========================================
*/

class WeatherState: ScriptAndConfig
{
	/*!
	Gets state ID (index inside states array)
	*/
	proto external int GetStateID();
	/*!
	Gets state name ID (must be unique string within the states list)
	*/
	proto external string GetStateName();
	/*!
	Get start variant name ID
	*/
	proto external string GetStartVariantName();
	proto external int GetStartVariantIndex();
	/*!
	Get duration min range (in-game hours)
	*/
	proto external float GetDurationMin();
	/*!
	Get duration max range (in-game hours)
	*/
	proto external float GetDurationMax();
	/*!
	Gets variant list for this state.
	*/
	proto external void GetVariantsList(out notnull array<ref WeatherVariant> variants);
	/*!
	Gets transition list for this tate.
	*/
	proto external void GetTransitionsList(out notnull array<ref WeatherTransition> transitions);
	/*!
	Gets localized name string.
	*/
	proto external string GetLocalizedName();
	/*!
	Gets localized description string
	*/
	proto external string GetLocalizedDescription();
	/*!
	Gets UI icon resourcename path.
	*/
	proto external ResourceName GetIconPath();
	proto external int GetNumTransitions();
}
