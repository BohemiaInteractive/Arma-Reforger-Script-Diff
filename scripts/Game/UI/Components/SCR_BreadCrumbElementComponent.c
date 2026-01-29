/*!
Breacrumb element component for quick manipulation of breadcrunb Layout
Has icon and label 
*/

//------------------------------------------------------------------------------------------------
class SCR_BreadCrumbsElementComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("Icon")]
	protected string m_sIcon;
	
	[Attribute("Label")]
	protected string m_sLabel;
	
	protected ImageWidget m_wIcon;
	protected TextWidget m_wLabel;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wIcon = ImageWidget.Cast(w.FindAnyWidget(m_sIcon));
		m_wLabel = TextWidget.Cast(w.FindAnyWidget(m_sLabel));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set used icon and label 
	void SetBreadCrumb(SCR_UIInfo info)
	{
		if (m_wLabel)
			m_wLabel.SetText("");
		
		if (m_wIcon)
			info.SetIconTo(m_wIcon);
	}
}