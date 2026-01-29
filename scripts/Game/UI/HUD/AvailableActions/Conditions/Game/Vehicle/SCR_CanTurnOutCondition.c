[BaseContainerProps()]
class SCR_CanTurnOutCondition : SCR_AvailableActionCondition
{
	[Attribute(desc: "Requires the hatch to be open")]
	protected bool m_bRequireOpenHatch;

	[Attribute(desc: "Should it be required that character is turned out")]
	protected bool m_bCharacterTurnedOut;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		ChimeraCharacter character = data.GetCharacter();
		if (!character)
			return GetReturnResult(false);

		CompartmentAccessComponent compartmentAccessComp = character.GetCompartmentAccessComponent();
		if (!compartmentAccessComp)
			return GetReturnResult(false);

		if (compartmentAccessComp.IsGettingIn() || compartmentAccessComp.IsGettingOut() || compartmentAccessComp.IsSwitchingSeatsAnim())
			return GetReturnResult(false);

		BaseCompartmentSlot compartment = compartmentAccessComp.GetCompartment();
		if (!compartment || !compartment.CanTurnOut())
			return GetReturnResult(false);

		if (compartment.IsOccupantTurnedOut() != m_bCharacterTurnedOut)
			return GetReturnResult(false);

		BaseCompartmentManagerComponent compartmentManager = compartment.GetManager();
		return GetReturnResult(compartmentManager && compartmentManager.IsDoorOpen(compartment.GetTurnOutDoorIndex()) == m_bRequireOpenHatch);
	}
}