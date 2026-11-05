class SCR_TextFormatContainer<Class T> : SCR_TextFormatContainerBase
{
	protected T m_TextParamData;

	//------------------------------------------------------------------------------------------------
	override void ParamSetTextFormat(TextWidget textWidget, string text)
	{
		if (!m_TextParamData)
		{
			textWidget.SetText(text);
			return;
		}

		textWidget.SetTextFormat(text, m_TextParamData);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] textParams Given parameter to be used inside of text
	void SCR_TextFormatContainer(T textParam)
	{
		m_TextParamData = textParam;
	}
}