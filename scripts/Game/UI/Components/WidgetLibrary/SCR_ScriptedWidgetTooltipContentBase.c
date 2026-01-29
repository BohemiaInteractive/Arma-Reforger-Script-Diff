[BaseContainerProps(configRoot : true)]
class SCR_ScriptedWidgetTooltipContentBase
{
	[Attribute("{197FC671D07413E9}UI/layouts/Menus/Tooltips/Tooltip_SimpleMessage.layout", UIWidgets.ResourceNamePicker, ".layout for the content of the Tooltip", params: "layout")]
	protected ResourceName m_sContentLayout;
	
	[Attribute("", desc: "Message to display")]
	protected string m_sMessageText;
	
	[Attribute(UIColors.GetColorAttribute(UIColors.NEUTRAL_INFORMATION))]
	protected ref Color m_MessageColor;
	
	protected Widget m_wContentRoot;
	protected RichTextWidget m_wMessage;
	
	protected string m_sDefaultMessage;
	
	// Const
	protected const string WIDGET_MESSAGE = "Message";
	
	//------------------------------------------------------------------------------------------------
	bool Init(WorkspaceWidget workspace, Widget wrapper)
	{
		Clear();
		
		m_wContentRoot = workspace.CreateWidgets(m_sContentLayout, wrapper);
		if (!m_wContentRoot)
			return false;
		
		m_sDefaultMessage = m_sMessageText;
		
		m_wMessage = RichTextWidget.Cast(m_wContentRoot.FindAnyWidget(WIDGET_MESSAGE));
		ResetMessage();
		ResetMessageColor();
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void Clear()
	{
		if (m_wContentRoot)
			m_wContentRoot.RemoveFromHierarchy();
	}
	
	//------------------------------------------------------------------------------------------------
	bool SetMessage(string message)
	{
		if (!m_wMessage)
			return false;

		m_wMessage.SetText(message);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool ResetMessage()
	{
		return SetMessage(GetDefaultMessage());
	}

	//------------------------------------------------------------------------------------------------
	bool SetMessageColor(Color color)
	{
		if (!m_wMessage || !color)
			return false;

		m_wMessage.SetColor(color);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool ResetMessageColor()
	{
		return SetMessageColor(m_MessageColor);
	}
	
	//------------------------------------------------------------------------------------------------
	Widget GetContentRoot()
	{
		return m_wContentRoot;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetMessage()
	{
		if (!m_wMessage)
			return string.Empty;
		
		return m_wMessage.GetText();
	}
	
	//------------------------------------------------------------------------------------------------
	string GetDefaultMessage()
	{
		return m_sDefaultMessage;
	}
}