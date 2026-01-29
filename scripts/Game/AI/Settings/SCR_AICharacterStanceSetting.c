//! Base class for all settings which specify stance for character
//! !!! Use this class when calling GetCurrentSetting !!!
[BaseContainerProps(visible: false)]
class SCR_AICharacterStanceSettingBase : SCR_AICharacterSettingWithCause
{
	//---------------------------------------------------------------------------------------------------
	override typename GetCategorizationType()
	{
		return SCR_AICharacterStanceSettingBase;
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Must be implemented in derived class
	//! Returns the limited stance value
	//! desiredStance - stance wanted by AI
	ECharacterStance GetStance(ECharacterStance desiredStance);
}

//! Limits stance within min-max range
[BaseContainerProps()]
class SCR_AICharacterStanceSetting_Range : SCR_AICharacterStanceSettingBase
{
	[Attribute(typename.EnumToString(ECharacterStance, ECharacterStance.PRONE), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECharacterStance), desc: "Minimum allowed stance")]
	protected ECharacterStance m_eMinStance;

	[Attribute(typename.EnumToString(ECharacterStance, ECharacterStance.STAND), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECharacterStance), desc: "Maximum allowed stance")]
	protected ECharacterStance m_eMaxStance;
	
	//---------------------------------------------------------------------------------------------------
	static SCR_AICharacterStanceSetting_Range Create(SCR_EAISettingOrigin origin, SCR_EAIBehaviorCause cause, ECharacterStance minStance, ECharacterStance maxStance)
	{
		auto s = new SCR_AICharacterStanceSetting_Range();
		s.Init(origin, cause);
		s.m_eMinStance = minStance;
		s.m_eMaxStance = maxStance;
		s.VerifyStanceValues();
		return s;
	}
	
	//---------------------------------------------------------------------------------------------------
	override ECharacterStance GetStance(ECharacterStance desiredStance)
	{
		return Math.ClampInt(desiredStance, m_eMinStance, m_eMaxStance);
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void VerifyStanceValues()
	{
		// Values in ECharacterStance are not arithmetically ordered from lowest stance to highest,
		// so we should potentially swap them, to produce a min-max range
		if (m_eMinStance > m_eMaxStance)
		{
			int temp = m_eMaxStance;
			m_eMaxStance = m_eMinStance;
			m_eMinStance = temp;
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	override string GetDebugText()
	{
		return string.Format("Min/Max: %1/%2",
			typename.EnumToString(ECharacterStance, Math.Max(m_eMinStance, m_eMaxStance)),
			typename.EnumToString(ECharacterStance, Math.Min(m_eMinStance, m_eMaxStance)));
	}
}

//! Limits stance to one value
[BaseContainerProps()]
class SCR_AICharacterStanceSetting : SCR_AICharacterStanceSettingBase
{
	[Attribute(typename.EnumToString(ECharacterStance, ECharacterStance.PRONE), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECharacterStance))]
	protected ECharacterStance m_eStance;
	
	//---------------------------------------------------------------------------------------------------
	static SCR_AICharacterStanceSetting Create(SCR_EAISettingOrigin origin, SCR_EAIBehaviorCause cause, ECharacterStance stance)
	{
		auto s = new SCR_AICharacterStanceSetting();
		s.Init(origin, cause);
		s.m_eStance = stance;
		return s;
	}
	
	//---------------------------------------------------------------------------------------------------
	override ECharacterStance GetStance(ECharacterStance desiredStance)
	{
		return m_eStance;
	}
	
	//---------------------------------------------------------------------------------------------------
	override string GetDebugText()
	{
		return string.Format("%1", typename.EnumToString(ECharacterStance, m_eStance));
	}
}