//------------------------------------------------------------------------------------------------
//! Returns true if character is in a vehicle
[BaseContainerProps()]
class SCR_CharacterIsInVehicleCondition : SCR_AvailableActionCondition
{
	[Attribute("false", UIWidgets.CheckBox, "Allow if player is in pilot or driver station")]
	protected bool m_bIsDriver;

	[Attribute("false", UIWidgets.CheckBox, "Allow if player is in turret")]
	protected bool m_bIsTurret;

	[Attribute("false", UIWidgets.CheckBox, "Allow if player is in cargo")]
	protected bool m_bIsCargo;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity is in vehicle
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		bool result = false;

		// In vehicle
		if (data.GetIsCharacterInVehicle())
		{
			// No compartment type filters - allow all
			result = !(m_bIsDriver || m_bIsTurret || m_bIsCargo);

			// Check is player fits one of the required roles
			ECompartmentType type = data.GetCompartmentType();
			if (m_bIsDriver)
				result = result || type == ECompartmentType.PILOT;

			if (m_bIsTurret)
				result = result || type == ECompartmentType.TURRET;

			if (m_bIsCargo)
				result = result || type == ECompartmentType.CARGO;
		}

		return GetReturnResult(result);
	}
};
