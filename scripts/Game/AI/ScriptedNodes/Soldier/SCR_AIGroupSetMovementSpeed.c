//! Sets movement speed of character.
//! Also checks restrictions from settings.
class SCR_AIGroupSetMovementSpeed : AITaskScripted
{
	// Inputs
	protected static const string SPEED_PORT = "Speed";
	
	protected AIGroupMovementComponent m_MovementComponent;
	protected SCR_AIGroupSettingsComponent m_SettingsComp;
	
	[Attribute(EMovementType.IDLE.ToString(), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EMovementType))]
	protected EMovementType m_eSpeed;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		super.OnInit(owner);
		
		m_SettingsComp = SCR_AIGroupSettingsComponent.Cast(owner.FindComponent(SCR_AIGroupSettingsComponent));
		m_MovementComponent = AIGroupMovementComponent.Cast(owner.FindComponent(AIGroupMovementComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_MovementComponent)
			return ENodeResult.FAIL;
		
		EMovementType speed;
		
		if(!GetVariableIn(SPEED_PORT, speed))
			speed = m_eSpeed;
		
		// Check if speed is restricted by settings
		SCR_AIGroupCharactersMovementSpeedSettingBase setting = null;
		if (m_SettingsComp)
			setting = SCR_AIGroupCharactersMovementSpeedSettingBase.Cast(m_SettingsComp.GetCurrentSetting(SCR_AIGroupCharactersMovementSpeedSettingBase));
		
		if (setting)
			speed = setting.GetSpeed(speed);
		
		m_MovementComponent.SetGroupCharactersWantedMovementType(speed);

		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true;}

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		SPEED_PORT
	};
	override array<string> GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("Speed: %1\n", typename.EnumToString(EMovementType, m_eSpeed));
		return s;
	}

	//------------------------------------------------------------------------------------------------
	override static string GetOnHoverDescription()
	{
		return "Sets movement speed of character. Also checks restrictions from settings.";
	}
};
