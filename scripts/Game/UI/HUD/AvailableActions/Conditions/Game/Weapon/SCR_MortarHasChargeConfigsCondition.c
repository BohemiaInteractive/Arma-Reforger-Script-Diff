[BaseContainerProps()]
class SCR_MortarHasChargeConfigsCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Return true if character is in ADS of current controlled vehicle turret
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		// Current weapon
		BaseControllerComponent controller = data.GetCurrentVehicleController();
		if (!controller)
			return GetReturnResult(false);

		// Turret
		IEntity turretEntity = controller.GetOwner();
		if (!turretEntity)
			return GetReturnResult(false);

		SCR_BaseHUDComponent hudComp = SCR_BaseHUDComponent.Cast(turretEntity.FindComponent(SCR_BaseHUDComponent));
		if (!hudComp)
			return GetReturnResult(false);

		array<BaseInfoDisplay> infoDisplays = {};

		if (hudComp.GetInfoDisplays(infoDisplays) < 1)
			return GetReturnResult(false);

		SCR_MortarInfo mortarDisplay;
		foreach (BaseInfoDisplay display : infoDisplays)
		{
			mortarDisplay = SCR_MortarInfo.Cast(display);
			if (mortarDisplay)
				break;
		}

		if (!mortarDisplay)
			return GetReturnResult(false);

		infoDisplays.Clear();
		if (mortarDisplay.GetInfoDisplays(infoDisplays) < 1)
			return GetReturnResult(false);

		SCR_BallisticTableDisplay ballisticsDisplay;
		foreach (BaseInfoDisplay display : infoDisplays)
		{
			ballisticsDisplay = SCR_BallisticTableDisplay.Cast(display);
			if (ballisticsDisplay)
				break;
		}

		if (!ballisticsDisplay)
			return GetReturnResult(false);

		return GetReturnResult(ballisticsDisplay.GetHasDifferentChargeConfigs());
	}
}