/**
Base zone data. INHERENT FROM THIS DO NOT USE ON IT'S OWN!
*/
[BaseContainerProps(configRoot: true), BaseContainerCustomStringTitleField("DO NOT USE BASE CLASS!")]
class SCR_BaseEffectsModulePositionData
{	
	//------------------------------------------------------------------------------------------------
	/*!
	Get random position to spawn (or if PROJECTILE for target).
	Override in inherented version
	\return Random position
	*/
	vector GetNewPosition(SCR_EffectsModule effectModule);
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called by effect module. Called when a barrage is done
	
	*/
	void ResetPositions(SCR_EffectsModule effectModule);
};