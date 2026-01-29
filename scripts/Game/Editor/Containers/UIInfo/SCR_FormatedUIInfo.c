[BaseContainerProps(configRoot: true)]
class SCR_FormatedUIInfo : SCR_UIInfo
{
	[Attribute(desc: "Param1 which replaces %1 if name contains it")]
	protected string m_sParam1;
	
	[Attribute(desc: "Param1 which replaces %2 if name contains it")]
	protected string m_sParam2;
	
	[Attribute(desc: "Param1 which replaces %3 if name contains it")]
	protected string m_sParam3;
	
	[Attribute(desc: "Param1 which replaces %4 if name contains it")]
	protected string m_sParam4;

	//------------------------------------------------------------------------------------------------
	//! Set the params used when getting and setting the Name using the UI Format
	//! \param[in] param1 First param
	//! \param[in] param2 Second param
	//! \param[in] param3 Third param
	//! \param[in] param4 Forth param
	void SetParams(string param1 = string.Empty, string param2 = string.Empty, string param3 = string.Empty, string param4 = string.Empty)
	{
		m_sParam1 = param1;
		m_sParam2 = param2;
		m_sParam3 = param3;
		m_sParam4 = param4;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the params used when getting and setting the Name using the UI Format
	//! \param[out] param1 First param
	//! \param[out] param2 Second param
	//! \param[out] param3 Third param
	//! \param[out] param4 Forth param
	void GetParams(out string param1 = string.Empty, out string param2 = string.Empty, out string param3 = string.Empty, out string param4 = string.Empty)
	{
		param1 = m_sParam1;
		param2 = m_sParam2;
		param3 = m_sParam3;
		param4 = m_sParam4;
	}
	
	//------------------------------------------------------------------------------------------------
	override LocalizedString GetName()
	{
		return WidgetManager.Translate(super.GetName(), m_sParam1, m_sParam2, m_sParam3, m_sParam4);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool SetNameTo(TextWidget textWidget)
	{
		if (!textWidget)
			return false;
		
		textWidget.SetTextFormat(super.GetName(), m_sParam1, m_sParam2, m_sParam3, m_sParam4);
		return true;
	}
}
