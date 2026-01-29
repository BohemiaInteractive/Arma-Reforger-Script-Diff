//! Base class for all classes which specify movement speed of character
//! !!! Use this class when calling GetCurrentSetting !!!
[BaseContainerProps(visible: false)]
class SCR_AICharacterMovementSpeedSettingBase : SCR_AICharacterSettingWithCause
{
	//---------------------------------------------------------------------------------------------------
	override typename GetCategorizationType()
	{
		return SCR_AICharacterMovementSpeedSettingBase;
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Must be implemented in derived class
	//! Returns the limited speed value
	//! desiredSpeed - stance wanted by AI
	EMovementType GetSpeed(EMovementType desiredSpeed);
}

//! Limits movement speed within range
[BaseContainerProps()]
class SCR_AICharacterMovementSpeedSetting_Range : SCR_AICharacterMovementSpeedSettingBase
{
	[Attribute(typename.EnumToString(EMovementType, EMovementType.IDLE), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EMovementType), desc: "Minimum allowed speed")]
	protected EMovementType m_eMinSpeed;

	[Attribute(typename.EnumToString(EMovementType, EMovementType.SPRINT), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EMovementType), desc: "Maximum allowed speed")]
	protected EMovementType m_eMaxSpeed;
	
	//---------------------------------------------------------------------------------------------------
	static SCR_AICharacterMovementSpeedSetting_Range Create(SCR_EAISettingOrigin origin, SCR_EAIBehaviorCause cause, EMovementType minSpeed, EMovementType maxSpeed)
	{
		SCR_AICharacterMovementSpeedSetting_Range s = new SCR_AICharacterMovementSpeedSetting_Range();
		s.Init(origin, cause);
		s.m_eMinSpeed = minSpeed;
		s.m_eMaxSpeed = maxSpeed;
		s.VerifySpeedValues();
		return s;
	}
	
	//---------------------------------------------------------------------------------------------------
	override EMovementType GetSpeed(EMovementType desiredSpeed)
	{
		return Math.ClampInt(desiredSpeed, m_eMinSpeed, m_eMaxSpeed);
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void VerifySpeedValues()
	{
		if (m_eMinSpeed > m_eMaxSpeed)
		{
			int temp = m_eMinSpeed;
			m_eMinSpeed = m_eMaxSpeed;
			m_eMaxSpeed = temp;
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	override string GetDebugText()
	{
		return string.Format("Min/Max: %1/%2",
			typename.EnumToString(EMovementType, m_eMinSpeed),
			typename.EnumToString(EMovementType, m_eMaxSpeed));
	}
}

//! Limits movement speed to one value
[BaseContainerProps()]
class SCR_AICharacterMovementSpeedSetting : SCR_AICharacterMovementSpeedSettingBase
{
	[Attribute(typename.EnumToString(EMovementType, EMovementType.IDLE), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EMovementType))]
	protected EMovementType m_eSpeed;
	
	//---------------------------------------------------------------------------------------------------
	static SCR_AICharacterMovementSpeedSetting Create(SCR_EAISettingOrigin origin, SCR_EAIBehaviorCause cause, EMovementType speed)
	{
		auto s = new SCR_AICharacterMovementSpeedSetting();
		s.Init(origin, cause);
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
		return string.Format("%1",	typename.EnumToString(EMovementType, m_eSpeed));
	}
}