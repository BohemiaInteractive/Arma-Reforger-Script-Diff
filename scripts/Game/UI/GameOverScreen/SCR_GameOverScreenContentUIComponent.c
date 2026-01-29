class SCR_GameOverScreenContentUIComponent: ScriptedWidgetComponent
{
	[Attribute("GameOver_Image")]
	protected string m_sImageName;
	
	[Attribute("Image_SizeSetter")]
	protected string m_sImageHolderName;
	
	[Attribute("GameOver_State")]
	protected string m_sTileName;
	
	[Attribute("GameOver_Condition")]
	protected string m_sSubtitleName;
	
	[Attribute("GameOver_Description")]
	protected string m_sDebriefingName;
	
	protected Widget m_wRoot;
	
	/*!
	Fills the widgets of the gameover screen content
	\param endScreenUIContent contains the layout and any neccessary information for the endscreen content widget
	*/
	void InitContent(SCR_GameOverScreenUIContentData endScreenUIContent)
	{
		TextWidget titleWidget = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sTileName));
		TextWidget subtitleWidget = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sSubtitleName));
		TextWidget debriefingWidget = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sDebriefingName));
		ImageWidget image = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sImageName));
		
		if (titleWidget)
			titleWidget.SetTextFormat(endScreenUIContent.m_sTitle, endScreenUIContent.m_sTitleParam);

		if (subtitleWidget)
			subtitleWidget.SetTextFormat(endScreenUIContent.m_sSubtitle, endScreenUIContent.m_sSubtitleParam);
		
		if (debriefingWidget)
			debriefingWidget.SetTextFormat(endScreenUIContent.m_sDebriefing, endScreenUIContent.m_sDebriefingParam);
		
		
		if (image && !endScreenUIContent.m_sImageTexture.IsEmpty())
		{
			image.LoadImageTexture(0, endScreenUIContent.m_sImageTexture);
		}
		//~ Hide image if non set
		else 
		{
			Widget imageHolder = m_wRoot.FindAnyWidget(m_sImageHolderName);
			if (imageHolder)
				imageHolder.SetVisible(false);
		}
	}
	
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
	}
	override void HandlerDeattached(Widget w)
	{
	}
};
