[BaseContainerProps(visible: false, insertable: false)]
class SCR_AIGroupFormationSettingBase : SCR_AIGroupSetting
{
	override typename GetCategorizationType()
	{
		return SCR_AIGroupFormationSettingBase;
	}
	
	//! formation - the formation wanted by AI at this moment.
	//! For instance they might want one fireteam to do something in a specific formation.
	//! moveHandlerId - see AIGroupMovementComponent.
	SCR_EAIGroupFormation GetFormation(int moveHandlerId, SCR_EAIGroupFormation desiredFormation);
}

[BaseContainerProps()]
class SCR_AIGroupFormationSetting : SCR_AIGroupFormationSettingBase
{
	[Attribute(SCR_EAIGroupFormation.Wedge.ToString(), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EAIGroupFormation))]
	protected SCR_EAIGroupFormation m_eFormation;
	
	static SCR_AIGroupFormationSetting Create(SCR_EAISettingOrigin origin, SCR_EAIGroupFormation formation)
	{
		SCR_AIGroupFormationSetting s = new SCR_AIGroupFormationSetting();
		s.Init(origin);
		s.m_eFormation = formation;
		return s;
	}
	
	override SCR_EAIGroupFormation GetFormation(int moveHandlerId, SCR_EAIGroupFormation desiredFormation)
	{
		return m_eFormation;
	}
	
	override string GetDebugText()
	{
		return typename.EnumToString(SCR_EAIGroupFormation, m_eFormation);
	}
}