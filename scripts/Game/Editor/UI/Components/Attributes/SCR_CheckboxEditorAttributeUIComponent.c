/** @ingroup Editor_UI Editor_UI_Components Editor_UI_Attributes
*/
class SCR_CheckboxEditorAttributeUIComponent: SCR_BaseEditorAttributeUIComponent
{
	protected SCR_SelectionWidgetComponent m_SelectionBoxComponent;
	
	override void Init(Widget w, SCR_BaseEditorAttribute attribute)
	{
		Widget checkboxWidget = w.FindAnyWidget(m_sUiComponentName);
		if (!checkboxWidget) 
			return;
		
		m_SelectionBoxComponent = SCR_SelectionWidgetComponent.Cast(checkboxWidget.FindHandler(SCR_SelectionWidgetComponent));
		if (!m_SelectionBoxComponent) 
			return;

		m_SelectionBoxComponent.m_OnChanged.Insert(OnChangeCheckbox);

		super.Init(w, attribute);
	}
	
	//Sets a default state for the UI and var value if conflicting attribute
	override void SetVariableToDefaultValue(SCR_BaseEditorAttributeVar var)
	{
		m_SelectionBoxComponent.SetCurrentItem(0);
		
		if (var)
			var.SetBool(false);
	}
	
	override void SetFromVar(SCR_BaseEditorAttributeVar var)
	{	
		super.SetFromVar(var);
		
		if (!var)
			return;
		
		m_SelectionBoxComponent.SetCurrentItem((int)var.GetBool(), false, false);
	}
	
	protected void OnChangeCheckbox(SCR_SelectionWidgetComponent selectionBox, bool value)
	{
		OnChangeInternal(selectionBox.GetRootWidget(), value, 0, false);
	}
	
	override bool OnChangeInternal(Widget w, int x, int y, bool finished)
	{	
		if (!m_SelectionBoxComponent) 
			return false;
		
		SCR_BaseEditorAttribute attribute = GetAttribute();
		if (!attribute) return false;

		SCR_BaseEditorAttributeVar var = attribute.GetVariable(true);
		
		if (var.GetBool() == x)
			return false;
		
		var.SetBool(x);
		super.OnChangeInternal(w, x, y, finished);
		return false;
	}	
	
	override void HandlerDeattached(Widget w)
	{	
		if (m_SelectionBoxComponent)
			m_SelectionBoxComponent.m_OnChanged.Remove(OnChangeCheckbox);		
	}
};