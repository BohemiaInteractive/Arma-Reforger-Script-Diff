class SCR_AIDecoWeaponUnobstructed : DecoratorScripted
{
	static override bool VisibleInPalette() { return true; }

	protected CharacterControllerComponent m_CharacterController;
	protected CompartmentAccessComponent m_CompartmentAccess;
	protected bool m_bInitialized;
	
	override bool TestFunction(AIAgent owner)
	{
		if (!m_bInitialized)
			return false;
		
		// Don't care if in compartment
		if (m_CompartmentAccess.GetCompartment())
			return true;
		
		return m_CharacterController.GetWeaponObstructedState() == EWeaponObstructedState.UNOBSTRUCTED;
	}
	
	override void OnInit(AIAgent owner)
	{
		IEntity controlledEntity = owner.GetControlledEntity();
		m_CharacterController = CharacterControllerComponent.Cast(controlledEntity.FindComponent(CharacterControllerComponent));
		m_CompartmentAccess = CompartmentAccessComponent.Cast(controlledEntity.FindComponent(CompartmentAccessComponent));
		m_bInitialized = m_CharacterController && m_CompartmentAccess;
	}
	
	static override string GetOnHoverDescription()
	{
		return "Returns TRUE if current weapon is not obstructed";
	}
}