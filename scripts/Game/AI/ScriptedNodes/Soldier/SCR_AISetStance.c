//! Sets stance of character.
//! Also before doing so, it checks restriction from group info component, and from settings component.
class SCR_AISetStance : AITaskScripted
{
	// Inputs
	protected static const string STANCE_PORT = "Stance";
	
	protected CharacterControllerComponent m_CharacterController;
	protected SCR_AICharacterSettingsComponent m_SettingsComp;
	
	[Attribute("0", UIWidgets.ComboBox, "Desired character stance", "", ParamEnumArray.FromEnum(ECharacterStance) )]
	protected ECharacterStance m_eStance;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		super.OnInit(owner);
		
		m_SettingsComp = SCR_AICharacterSettingsComponent.Cast(owner.FindComponent(SCR_AICharacterSettingsComponent));
		
		IEntity myEntity = owner.GetControlledEntity();
		if(myEntity)
		{
			m_CharacterController = CharacterControllerComponent.Cast(owner.GetControlledEntity().FindComponent(CharacterControllerComponent));
			if (!m_CharacterController)
				NodeError(this, owner, "Can't find CharacterControllerComponent.");
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_CharacterController)
			return ENodeResult.FAIL;
		
		ECharacterStance stance;
		
		if(!GetVariableIn(STANCE_PORT, stance))
			stance = m_eStance;
		
		// Stance is first limited by this character's group
		AIGroup group = owner.GetParentGroup();
		if (group)
		{
			SCR_AIGroupInfoComponent groupInfoComp = SCR_AIGroupInfoComponent.Cast(group.FindComponent(SCR_AIGroupInfoComponent));
			if (groupInfoComp)
				stance = groupInfoComp.GetAllowedStance(stance);
		}
		
		// Check if stance is restricted by settings
		SCR_AICharacterStanceSettingBase stanceSetting = null;
		if (m_SettingsComp)
			stanceSetting = SCR_AICharacterStanceSettingBase.Cast(m_SettingsComp.GetCurrentSetting(SCR_AICharacterStanceSettingBase));
		
		if (stanceSetting)
			stance = stanceSetting.GetStance(stance);
		
		SCR_AIStanceHandling.SetStance(m_CharacterController, stance);

		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true;}

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		STANCE_PORT
	};
	override array<string> GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_eStance: %1\n", typename.EnumToString(ECharacterStance, m_eStance));
		return s;
	}
	
	//------------------------------------------------------------------------------------------------
	override static string GetOnHoverDescription()
	{
		return "Sets stance of character. Also checks restriction from group info component, and from settings component.";
	}
};
