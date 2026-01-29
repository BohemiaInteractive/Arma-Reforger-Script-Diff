[BaseContainerProps(configRoot: true)]
class SCR_DamageStateConfig
{
	[Attribute(desc: "An array of all damage states to obtain the Icon and Color")]
	protected ref array<ref SCR_DamageStateInfo> m_aDamageStateUiInfo;
	
	[Attribute(desc: "UI info for Unconscious state")]
	protected ref SCR_DamageStateUIInfo m_UnconciousStateUiInfo;
	
	[Attribute(desc: "UI info for Death state")]
	protected ref SCR_DamageStateUIInfo m_DeathStateUiInfo;
	
	[Attribute(desc: "UI info for Destroyed state")]
	protected ref SCR_DamageStateUIInfo m_DestroyedStateUiInfo;
	
	[Attribute(desc: "UI info for Fractured state")]
	protected ref SCR_DamageStateUIInfo m_FracturedStateUiInfo;
 	
	/*!
	Get UI info using damage type
	\param damageType Damage type of the visual that needs to be obtained
	\return Ui info
	*/
	SCR_DamageStateUIInfo GetUiInfo(EDamageType damageType)
	{
		foreach(SCR_DamageStateInfo damageStateInfo: m_aDamageStateUiInfo)
		{
			if (damageStateInfo.m_eDamageType == damageType)
				return damageStateInfo.m_UiInfo;
		}
		
		Print(string.Format("'SCR_DamageStateConfig': %1 is not included in 'm_aDamageStateUiInfo' thus cannot get UIinfo", typename.EnumToString(EDamageType, damageType)), LogLevel.WARNING);
		return null;
	}
	
	/*!
	Get array of all damagestate info including the identifier enum
	\param[out] damageStateInfoArray array of info
	\return array size
	*/
	int GetDamageStateInfoArray(out notnull array<ref SCR_DamageStateInfo> damageStateInfoArray)
	{
		damageStateInfoArray.Clear();
		
		foreach(SCR_DamageStateInfo damageStateInfo: m_aDamageStateUiInfo)
			damageStateInfoArray.Insert(damageStateInfo);
		
		return damageStateInfoArray.Count();
	}
	
	/*!
	Get UI info for Unconcious State
	\return Ui info
	*/
	SCR_DamageStateUIInfo GetUnconciousStateUiInfo()
	{
		return m_UnconciousStateUiInfo;
	}
	
	/*!
	Get UI info for Death State
	\return Ui info
	*/
	SCR_DamageStateUIInfo GetDeathStateUiInfo()
	{
		return m_DeathStateUiInfo;
	}
	
	/*!
	Get UI info for Destroyed State
	\return Ui info
	*/
	SCR_DamageStateUIInfo GetDestroyedStateUiInfo()
	{
		return m_DestroyedStateUiInfo;
	}	
	
	/*!
	Get UI info for Destroyed State
	\return Ui info
	*/
	SCR_DamageStateUIInfo GetFracturedStateUiInfo()
	{
		return m_FracturedStateUiInfo;
	}
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EDamageType, "m_eDamageType")]
class SCR_DamageStateInfo
{
	[Attribute(desc: "Damage type associated with UI info", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EDamageType))]
	EDamageType m_eDamageType;
	
	[Attribute(desc: "UI info of damage type, including colors and background images")]
	ref SCR_DamageStateUIInfo m_UiInfo;
};