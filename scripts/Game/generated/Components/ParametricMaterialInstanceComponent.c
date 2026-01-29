/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class ParametricMaterialInstanceComponent: GameComponent
{
	/*!
	Use it to set albedo color.
	\param argbColor The new albedo color.
	*/
	proto external void SetColor(int argbColor);
	//! Get the current albedo color.
	proto external int GetColor();
	/*
	Use it to set emissive color.
	\param argbColor The new emissive color.
	*/
	proto external void SetEmissiveColor(int argbColor);
	//! Get the current emissive color.
	proto external int GetEmissiveC();
	/*
	Use it to set emissive multiplier.
	\param multiplier The new multiplier in <0, 1000>
	*/
	proto external void SetEmissiveMultiplier(int multiplier);
	//! Get the current emissive multiplier.
	proto external int GetEmissiveM();
	/*
	Use it to set user parameter modifier specified in Material (Dirt, Mud etc..).
	\param param The new parameter value in <0, 255>
	*/
	proto external void SetUserParam1(int param);
	proto external int GetUserParam1();
	/*
	Use it to set user parameter modifier specified in Material (Dirt, Mud etc..).
	\param param The new parameter value in <0, 255>
	*/
	proto external void SetUserParam2(int param);
	proto external int GetUserParam2();
	/*
	Use it to set user parameter modifier specified in Material (Dirt, Mud etc..).
	\param param The new parameter value in <0, 255>
	*/
	proto external void SetUserParam3(int param);
	proto external int GetUserParam3();
	/*
	Use it to set user parameter modifier specified in Material (Dirt, Mud etc..).
	\param param The new parameter value in <0, 255>
	*/
	proto external void SetUserParam4(int param);
	proto external int GetUserParam4();
	/*
	Use it to set user alpha test parameter.
	\param alphaTest The new alpha test value in <0, 255>
	*/
	proto external void SetUserAlphaTestParam(int alphaTest);
	//! Get the current alpha test value.
	proto external int GetUserAlphaTestParam();
	/*
	Sets if custom wetness is enabled. Wetness value itself can be then set using SetCustomWetnessParam1 and SetCustomWetnessParam2
	\param val Use true to enable custom wetness, false otherwise.
	*/
	proto external void SetCustomWetnessEnabled(bool val);
	//! Returns true if custom wetness is enabled.
	proto external bool GetCustomWetnessEnabled();
	/*
	Sets primary custom wetness value
	\param customWetness primary custom wetness value in <0, 255>
	*/
	proto external void SetCustomWetnessParam1(int customWetness);
	//! Get the current primary custom wetness value.
	proto external int GetCustomWetnessParam1();
	/*
	Sets secondary custom wetness value
	\param customWetness secondary custom wetness value in <0, 255>
	*/
	proto external void SetCustomWetnessParam2(int customWetness);
	//! Get the current primary custom wetness value.
	proto external int GetCustomWetnessParam2();
	/*
	Sets if rain sliding drops are enabled.
	\param val Use true to enable rain sliding drops, false otherwise.
	*/
	proto external void SetSlidingDropsEnabled(bool val);
	//! Returns true if sliding drops is enabled.
	proto external bool GetSlidingDropsEnabled();
	proto external bool SaveState(ScriptBitWriter writer);
	proto external bool LoadState(ScriptBitReader sreader);
}

/*!
\}
*/
