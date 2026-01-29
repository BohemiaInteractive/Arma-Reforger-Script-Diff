/** @ingroup Editor_UI Editor_UI_Components Editor_UI_Attributes
*/
class SCR_SpinboxEditorAttributeUIComponent: SCR_BaseEditorAttributeUIComponent
{		
	protected SCR_SpinBoxComponent m_SpinboxWidgetComponent;
	protected bool m_bInitCalled;
	
	override void Init(Widget w, SCR_BaseEditorAttribute attribute)
	{
		Widget spingBoxWidget = w.FindAnyWidget(m_sUiComponentName);
		
		if (!spingBoxWidget) return;
		m_SpinboxWidgetComponent = SCR_SpinBoxComponent.Cast(spingBoxWidget.FindHandler(SCR_SpinBoxComponent));
		
		if (!m_SpinboxWidgetComponent) return;
		m_SpinboxWidgetComponent.m_OnChanged.Insert(OnChangeSpinbox);	
		
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (var)
		{
			SCR_BaseEditorAttributeFloatStringValues spinboxData;
			array<ref SCR_BaseEditorAttributeEntry> entries = new array<ref SCR_BaseEditorAttributeEntry>;
			attribute.GetEntries(entries);
			
			foreach (SCR_BaseEditorAttributeEntry entry: entries)
			{
				spinboxData = SCR_BaseEditorAttributeFloatStringValues.Cast(entry);
				if (spinboxData)
				{
					int count = spinboxData.GetValueCount();
					
					for (int i = 0; i < count; i++)
					{
						m_SpinboxWidgetComponent.AddItem(spinboxData.GetValuesEntry(i).GetName(), i == count - 1);
					}
					
					continue;
				}
			}
			
			//Spin box uses text entries
			if (!spinboxData)
			{
				int entriesCount = entries.Count();
				for (int i = 0; i < entriesCount; i++)
				{
					SCR_BaseEditorAttributeEntryText entry = SCR_BaseEditorAttributeEntryText.Cast(entries[i]);
					if (entry) 
						m_SpinboxWidgetComponent.AddItem(entry.GetText(), i == entriesCount - 1);
				}
			}
		}
		
		super.Init(w, attribute);
	}
	
	//Sets a default state for the UI and var value if conflicting attribute
	override void SetVariableToDefaultValue(SCR_BaseEditorAttributeVar var)
	{
		m_SpinboxWidgetComponent.SetCurrentItem(0);
		
		if (var)
			var.SetInt(0);
	}
	
	override void SetFromVar(SCR_BaseEditorAttributeVar var)
	{
		super.SetFromVar(var);
		
		if (!var)
			return;
		
		m_SpinboxWidgetComponent.SetCurrentItem(var.GetInt(), false, false);
		m_bInitCalled = true;	
	}
	
	override bool OnChangeInternal(Widget w, int x, int y, bool finished)
	{	
		SCR_BaseEditorAttribute attribute = GetAttribute();
		if (!attribute) return false;
		SCR_BaseEditorAttributeVar var = attribute.GetVariable(true);
		
		if (var.GetInt() == x)
			return false;
		
		var.SetInt(x);
		super.OnChangeInternal(w, x, y, finished);
		return false;
	}
	
	protected void OnChangeSpinbox(SCR_SpinBoxComponent spinbox, int value)
	{
		if (m_bInitCalled)
			OnChangeInternal(spinbox.GetRootWidget(), value, 0, false);
	}
	
	
	override void HandlerDeattached(Widget w)
	{
		if (m_SpinboxWidgetComponent)
			m_SpinboxWidgetComponent.m_OnChanged.Remove(OnChangeSpinbox);		
	}
};