//------------------------------------------------------------------------------------------------
//! Returns true if autohover is on
[BaseContainerProps()]
class SCR_HelicopterAutohoverCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		HelicopterControllerComponent controller = HelicopterControllerComponent.Cast(data.GetCurrentVehicleController());
		if (!controller)
			return false;
	
		return GetReturnResult(controller.GetAutohoverEnabled());		
	}
};
