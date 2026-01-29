class SCR_SquadRoleComboBoxElement : ScriptedWidgetComponent
{
	[Attribute("RankImage")]
	protected string m_sRankImageWidgetName;

	[Attribute("HalfEmptyGroupImage")]
	protected string m_sHalfEmptyGroupImageWidgetName;

	protected ImageWidget m_wRankImage;
	protected Widget m_wHalfEmptyGroupImage;

	//------------------------------------------------------------------------------------------------
	void SetRankImage(string rankIconName)
	{
		m_wRankImage.LoadImageFromSet(0, SCR_XPInfoDisplay.GetRankIconImageSet(), rankIconName);
	}

	//------------------------------------------------------------------------------------------------
	void SetVisibleInsufficientRank(bool show)
	{
		m_wRankImage.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	void SetVisibleNotEnoughFullGroup(bool show)
	{
		m_wHalfEmptyGroupImage.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRankImage = ImageWidget.Cast(w.FindAnyWidget(m_sRankImageWidgetName));
		m_wRankImage.SetColor(UIColors.WARNING);

		m_wHalfEmptyGroupImage = w.FindAnyWidget(m_sHalfEmptyGroupImageWidgetName);
		m_wHalfEmptyGroupImage.SetColor(UIColors.WARNING);
	}
}
