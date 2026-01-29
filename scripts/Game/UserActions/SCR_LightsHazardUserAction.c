class SCR_LightsHazardUserAction : LightUserAction
{
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!pOwnerEntity || !pUserEntity)
			return;

		BaseLightManagerComponent lightManager = GetLightManager();
		if (!lightManager)
			return;
		
		bool lightsState;	
		
		lightsState = lightManager.GetLightsState(ELightType.Hazard);

		if (RplSession.Mode() != RplMode.Client)
			lightsState = !lightsState;

		lightManager.SetLightsState(ELightType.Hazard, lightsState);
				
		// Sound		
		PlaySound(pOwnerEntity, lightsState);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		BaseLightManagerComponent lightManager = GetLightManager();

		auto prefix = "";
		UIInfo actionInfo = GetUIInfo();
		if(actionInfo)
			prefix = actionInfo.GetName() + " ";

		if (lightManager && lightManager.GetLightsState(ELightType.Hazard))
			outName = prefix + "#AR-UserAction_State_Off";
		else
			outName = prefix + "#AR-UserAction_State_On";

		return true;
	}
};
