class SCR_CharacterPerceivableComponentClass : CharacterPerceivableComponentClass
{
}

class SCR_CharacterPerceivableComponent : CharacterPerceivableComponent
{
	protected SCR_CharacterControllerComponent m_CharacterController;
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(owner);
		if (!character)
			return;
		
		m_CharacterController = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		
		if (m_CharacterController)
			m_CharacterController.m_OnLifeStateChanged.Insert(OnLifeStateChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_CharacterPerceivableComponent()
	{
		if (m_CharacterController)
			m_CharacterController.m_OnLifeStateChanged.Remove(OnLifeStateChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		bool disarmed = newLifeState != ECharacterLifeState.ALIVE;
		SetDisarmed(disarmed);
	}
}
