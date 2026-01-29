/*
===========================================
Do not modify, this script is generated
===========================================
*/

class WeatherVariant: ScriptAndConfig
{
	proto external ref WeatherWindPattern GetWindPattern();
	proto external ref WeatherRainPattern GetRainPattern();
	proto external ref WeatherFogPattern GetFogPattern();
	proto external ref WeatherLightningPattern GetLightningPattern();

	proto external string GetVariantName();
	proto external float GetDurationMin();
	proto external float GetDurationMax();
	proto external int GetNumTransitions();
	proto external void GetTransitionsList(out notnull array<ref WeatherTransition> transitions);
}
