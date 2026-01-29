//! Group-level setting of movement speed of its characters. Used by Move waypoint.
//! It is meant for AIGroupMovementComponent.SetGroupCharactersWantedMovementType
[BaseContainerProps(visible: false)]
class SCR_AIGroupCharactersMovementSpeedSettingBase : SCR_AIGroupSetting
{
	//---------------------------------------------------------------------------------------------------
	override typename GetCategorizationType()
	{
		return SCR_AIGroupCharactersMovementSpeedSettingBase;
	}
	
	//---------------------------------------------------------------------------------------------------
	//! desiredSpeed - speed wanted by AI
	EMovementType GetSpeed(EMovementType desiredSpeed);
}

[BaseContainerProps()]
class SCR_AIGroupCharactersMovementSpeedSetting : SCR_AIGroupCharactersMovementSpeedSettingBase
{
	[Attribute(typename.EnumToString(EMovementType, EMovementType.IDLE), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EMovementType))]
	protected EMovementType m_eSpeed;
	
	//---------------------------------------------------------------------------------------------------
	static SCR_AIGroupCharactersMovementSpeedSetting Create(SCR_EAISettingOrigin origin, EMovementType speed)
	{
		auto s = new SCR_AIGroupCharactersMovementSpeedSetting();
		s.Init(origin);
		s.m_eSpeed = speed;
		return s;
	}
	
	//---------------------------------------------------------------------------------------------------
	override EMovementType GetSpeed(EMovementType desiredSpeed)
	{
		return m_eSpeed;
	}
	
	//---------------------------------------------------------------------------------------------------
	override string GetDebugText()
	{
		return typename.EnumToString(EMovementType, m_eSpeed);
	}
}