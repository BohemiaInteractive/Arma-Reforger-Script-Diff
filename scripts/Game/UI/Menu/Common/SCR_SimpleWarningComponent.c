//------------------------------------------------------------------------------------------------
class SCR_SimpleWarningComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("#AR-Account_LoginTimeout")]
	protected string m_sWarning;
	
	[Attribute(UIConstants.ICON_WARNING)]
	protected string m_sIconName;
	
	[Attribute(UIColors.GetColorAttribute(UIColors.WARNING))]
	protected ref Color m_Color;
	
	[Attribute(UIColors.GetColorAttribute(UIColors.WARNING))]
	protected ref Color m_TextColor;
	
	protected TextWidget m_wWarning;
	protected Widget m_wWarningWrapper;
	protected ImageWidget m_wWarningImage;
	protected SizeLayoutWidget m_wWarningImageSize;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wWarning = TextWidget.Cast(w.FindAnyWidget("WarningText"));
		m_wWarningImage = ImageWidget.Cast(w.FindAnyWidget("WarningImage"));
		m_wWarningImageSize = SizeLayoutWidget.Cast(w.FindAnyWidget("WarningImageSize"));
		m_wWarningWrapper = w.FindAnyWidget("WarningWrapper");
		ResetWarning();
		ResetWarningColor();
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetWarning()
	{
		SetWarning(m_sWarning, m_sIconName);
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetWarningColor()
	{
		SetWarningColor(m_Color, m_TextColor);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWarning(string text, string iconName, ResourceName imageset = string.Empty)
	{
		if (imageset.IsEmpty())
			imageset = UIConstants.ICONS_IMAGE_SET;
		
		if (m_wWarning)
			m_wWarning.SetText(text);
		
		if (m_wWarningImage && !iconName.IsEmpty())
			m_wWarningImage.LoadImageFromSet(0, imageset, iconName);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWarningColor(Color color, Color textColor)
	{
		if (m_wWarning)
			m_wWarning.SetColor(textColor);
		
		if (m_wWarningImage)
			m_wWarningImage.SetColor(color);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWarningVisible(bool visible, bool preserveSpace = true)
	{
		m_wRoot.SetVisible(visible || !visible && preserveSpace);
		
		if (m_wWarningWrapper)
			m_wWarningWrapper.SetVisible(visible);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIconVisible(bool visible, bool preserveSpace = false)
	{
		if (m_wWarningImage)
			m_wWarningImage.SetVisible(visible);
		
		if (m_wWarningImageSize)
			m_wWarningImageSize.SetVisible(visible || !visible && preserveSpace);
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_SimpleWarningComponent FindComponentInHierarchy(notnull Widget root)
	{
		ScriptedWidgetEventHandler handler = SCR_WidgetTools.FindHandlerInChildren(root, SCR_SimpleWarningComponent);
		if (handler)
			return SCR_SimpleWarningComponent.Cast(handler);

		return null;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_SimpleWarningComponent FindComponent(notnull Widget w)
	{
		return SCR_SimpleWarningComponent.Cast(w.FindHandler(SCR_SimpleWarningComponent));
	}
}