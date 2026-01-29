class SCR_AISetWeaponRaised : SCR_AICharacterStats
{
	[Attribute("true", UIWidgets.CheckBox, "Raise weapon",)]
	private bool m_bWeaponRaised;

	static const string RAISED_PORT = "Raised";
	
	private SCR_CharacterControllerComponent m_CharacterController;

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_AIInfo || !m_CharacterController)
			return ENodeResult.FAIL;

		int raiseWeapon;
		if(!GetVariableIn(RAISED_PORT,raiseWeapon))
			raiseWeapon = m_bWeaponRaised;
		
		m_CharacterController.SetWeaponRaised(raiseWeapon);

		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		RAISED_PORT
	};
	override array<string> GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_bWeaponRaised: %1\n", m_bWeaponRaised);
		return s;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		super.OnInit(owner);
		
		if(owner.GetControlledEntity())
		{
			m_CharacterController = SCR_CharacterControllerComponent.Cast(owner.GetControlledEntity().FindComponent(SCR_CharacterControllerComponent));
			if (!m_CharacterController)
				NodeError(this, owner, "Can't find SCR_CharacterControllerComponent.");
		}
	}
};