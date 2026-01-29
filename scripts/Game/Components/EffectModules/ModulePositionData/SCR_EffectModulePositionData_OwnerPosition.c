/**
Zone data that will spawn the entity at the location of the owner (Snapping to ground might apply)
*/
[BaseContainerProps(configRoot: true), BaseContainerCustomStringTitleField("Owner")]
class SCR_EffectsModulePositionData_OwnerPosition : SCR_BaseEffectsModulePositionData
{	
	//------------------------------------------------------------------------------------------------
	override vector GetNewPosition(SCR_EffectsModule effectModule)
	{
		return effectModule.GetLocalTargetOffset();
	}
};