//------------------------------------------------------------------------------------------------
/*!
	The most basic payload that can be derived and sent via SCR_SpawnRequestComponent to be
	handled by SCR_SpawnHandlerComponent on the authority.

	Each data needs a specific handler, the relation of SCR_SpawnRequestComponent : SCR_SpawnHandlerComponent is always 1:1.
	See existing implementation for details and examples.
*/
class SCR_SpawnData
{
	//------------------------------------------------------------------------------------------------
	/*!
		Returns the resource name of prefab to spawn.
	*/
	ResourceName GetPrefab();

	//------------------------------------------------------------------------------------------------
	/*!
		Returns the position at which to spawn.
	*/
	vector GetPosition();

	//------------------------------------------------------------------------------------------------
	/*!
		Returns the orientation at which to spawn.
	*/
	vector GetAngles();

	//------------------------------------------------------------------------------------------------
	/*!
		Are data valid in the most basic context?
	*/
	bool IsValid()
	{
		return true;
	}
	
	bool GetSkipPreload()
	{
		return false;
	}
	
	void SetSkipPreload(bool skip);
};
