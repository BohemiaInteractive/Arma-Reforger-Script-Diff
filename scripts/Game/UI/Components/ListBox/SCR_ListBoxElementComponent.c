//! Element of a listbox.
class SCR_ListBoxElementComponent : SCR_ModularButtonComponent
{
	[Attribute("Image")]
	protected string m_sWidgetImageName;

	[Attribute("Text")]
	protected string m_sWidgetTextName;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
	
		m_bToggledOnlyThroughApi = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] imageOrImageset
	//! \param[in] iconName
	void SetImage(ResourceName imageOrImageset, string iconName)
	{
		ImageWidget w = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sWidgetImageName));
		
		if (imageOrImageset.IsEmpty())
			return;
		
		if (w)
		{
			if (imageOrImageset.EndsWith("imageset"))
			{
				if (!iconName.IsEmpty())
					w.LoadImageFromSet(0, imageOrImageset, iconName);
			}
			else
			{
				w.LoadImageTexture(0, imageOrImageset);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] text
	void SetText(string text)
	{
		TextWidget w = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sWidgetTextName));
		if (w)
			w.SetText(text);
	}
}
