/** @ingroup Editor_UI Editor_UI_Components Editor_UI_Attributes
*/
class SCR_AttributeDescriptionUIComponent: ScriptedWidgetComponent
{
	[Attribute("Content")]
	protected string m_sContentTextName;
	
	[Attribute("Icon")]
	protected string m_sIconName;	
	
	[Attribute("Line")]
	protected string m_sLineName;
	
	[Attribute("Background")]
	protected string m_sBackgroundName;
	
	protected RichTextWidget m_ContentText;
	protected ImageWidget m_Icon;
	protected ImageWidget m_Line;
	protected Widget m_Background;
	
	protected void SetAttributeDescription(SCR_EditorAttributeUIInfo uiInfo, string customContent = string.Empty, string param1 = string.Empty, string param2 = string.Empty,  string param3 = string.Empty)
	{		
		//Hide description
		if (!uiInfo)
		{
			m_Background.SetVisible(false);
			m_Icon.SetVisible(false);
			m_ContentText.SetVisible(false);
			m_Line.SetVisible(false);
			
			return;
		}
		else 
		{
			m_Background.SetVisible(true);
			m_ContentText.SetVisible(true);
			m_Line.SetVisible(true);
		}
			
		//Default Description
		if (customContent.IsEmpty())
		{	
			m_ContentText.SetTextFormat(uiInfo.GetDescription(), param1, param2, param3);

			if (uiInfo.GetIconPath().IsEmpty())
			{
				m_Icon.SetVisible(false);
			}
			else
			{
				uiInfo.SetIconTo(m_Icon);
				
				if (uiInfo.GetDescriptionIconColor())
					m_Icon.SetColor(uiInfo.GetDescriptionIconColor());
				
				m_Icon.SetVisible(true);
			}
		}
		//Custom description
		else 
		{
			m_ContentText.SetTextFormat(customContent, param1, param2, param3);
			
			if (uiInfo.GetIconPath().IsEmpty())
			{
				m_Icon.SetVisible(false);
			}
			else
			{
				uiInfo.SetIconTo(m_Icon);
				
				if (uiInfo.GetDescriptionIconColor())
					m_Icon.SetColor(uiInfo.GetDescriptionIconColor());
				
				m_Icon.SetVisible(true);
			}
		}
	}
	
	protected void OnCategoryChanged(ResourceName currentCategory)
	{		
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
		{
			GetGame().GetCallqueue().CallLater(SetAttributeDescription, 1, false, null, string.Empty, string.Empty, string.Empty, string.Empty);	
		}
	}
	
	override void HandlerAttached(Widget w)
	{
		SCR_AttributesManagerEditorComponent attributesManager = SCR_AttributesManagerEditorComponent.Cast(SCR_AttributesManagerEditorComponent.GetInstance(SCR_AttributesManagerEditorComponent));
		if (!attributesManager) 
			return;
		
		m_ContentText = RichTextWidget.Cast(w.FindAnyWidget(m_sContentTextName));
		m_Icon = ImageWidget.Cast(w.FindAnyWidget(m_sIconName));
		m_Line = ImageWidget.Cast(w.FindAnyWidget(m_sLineName));
		
		if (!m_ContentText || !m_Icon)
			return;
		
		m_Background = w.FindAnyWidget(m_sBackgroundName);
		
		//Hide desciption at start
		SetAttributeDescription(null);
		
		attributesManager.GetOnAttributeDescriptionChanged().Insert(SetAttributeDescription);
	}
	
	override void HandlerDeattached(Widget w)
	{
		SCR_AttributesManagerEditorComponent attributesManager = SCR_AttributesManagerEditorComponent.Cast(SCR_AttributesManagerEditorComponent.GetInstance(SCR_AttributesManagerEditorComponent));
		if (!attributesManager || !m_ContentText || !m_Icon) 
			return;
		
		attributesManager.GetOnAttributeDescriptionChanged().Remove(SetAttributeDescription);
	}

};