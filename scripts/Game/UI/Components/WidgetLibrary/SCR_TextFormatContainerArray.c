class SCR_TextFormatContainerArray<Class T> : SCR_TextFormatContainerBase
{
	protected array<T> m_aTextParamData;

	//------------------------------------------------------------------------------------------------
	override void ParamSetTextFormat(TextWidget textWidget, string text)
	{
		if (!m_textParamData)
		{
			textWidget.SetText(text);
			return;
		}

		textWidget.SetTextFormat(text,
			GetArgOrNull(0),
			GetArgOrNull(1),
			GetArgOrNull(2),
			GetArgOrNull(3),
			GetArgOrNull(4),
			GetArgOrNull(5),
			GetArgOrNull(6),
			GetArgOrNull(7),
			GetArgOrNull(8));
	}

	//------------------------------------------------------------------------------------------------
	protected T GetArgOrNull(int idx)
	{
		if (!m_aTextParamData.IsIndexValid(idx))
			return null;

		return m_aTextParamData[idx];
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] textParams Given array of parameters to be used inside of text
	void SCR_TextFormatContainerArray(array<T> textParams)
	{
		m_aTextParamData = textParams;
	}
}