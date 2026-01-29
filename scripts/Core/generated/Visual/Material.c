/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Visual
\{
*/

sealed class Material: Managed
{
	private void Material();

	/*!
	Get material, must exist in internal cache.
	\param matName	material resource
	*/
	static proto ref Material GetMaterial(ResourceName matName);
	/*!
	get material or load it of it doesn't exist in internal cache
	\param matName	material resource
	\param flags		material creation flags, use 0
	*/
	static proto ref Material GetOrLoadMaterial(ResourceName matName, int flags);
	static proto ref Material Create(string name, string matClassName, map<string, string> params = null);
	/*!
	set param of material by string name
	\param paramName	name of parameter
	\param value		value
	*/
	proto bool SetParam(string paramName, void value);
	/*!
	reset param of material to default value
	\param paramName	name of parameter
	*/
	proto external void ResetParam(string paramName);
	/*!
	set param index for faster access to material properties
	\param paramName	name of parameter
	\return parameter index
	*/
	proto external int GetParamIndex(string paramName);
	//! Set parameter of material by index.
	proto void SetParamByIndex(int paramIndex, void value);
	/*!
	material name
	*/
	proto void GetName(out string name);
}

/*!
\}
*/
