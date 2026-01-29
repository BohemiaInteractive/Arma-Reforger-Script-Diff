/*!
Base struct for use in SCR_DSSessionCallback.
*/
[BaseContainerProps()]
class SCR_JsonApiStruct: JsonApiStruct
{
	/*!
	Write world data into the struct.
	*/
	bool Serialize()
	{
		return false;
	}
	/*!
	Read data from the struct and apply them in the world.
	*/
	bool Deserialize()
	{
		return false;
	}
	/*!
	Clear struct's data
	*/
	void ClearCache()
	{
	}
	/*!
	Log struct's data
	*/
	void Log()
	{
	}
};