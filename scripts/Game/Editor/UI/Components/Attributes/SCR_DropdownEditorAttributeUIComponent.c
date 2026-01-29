/** @ingroup Editor_UI Editor_UI_Components Editor_UI_Attributes
*/
class SCR_DropdownEditorAttributeUIComponent: SCR_BaseEditorAttributeUIComponent
{		
	protected SCR_ComboBoxComponent m_ComboBoxComponent;
	protected ref SCR_BaseEditorAttributeFloatStringValues m_comboboxData;
	protected bool m_bInitCalled;
	
	protected ref array<string> m_aDropdownArray = {};
	
	override void Init(Widget w, SCR_BaseEditorAttribute attribute)
	{
		Widget comboBoxWidget = w.FindAnyWidget(m_sUiComponentName);
		
		if (!comboBoxWidget) 
			return;
		
		m_ComboBoxComponent = SCR_ComboBoxComponent.Cast(comboBoxWidget.FindHandler(SCR_ComboBoxComponent));
		
		if (!m_ComboBoxComponent) 
			return;
		
		m_ComboBoxComponent.m_OnChanged.Insert(OnChangeComboBox);	
		
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (var)
		{
			array<ref SCR_BaseEditorAttributeEntry> entries = new array<ref SCR_BaseEditorAttributeEntry>;
			attribute.GetEntries(entries);
			
			foreach (SCR_BaseEditorAttributeEntry entry: entries)
			{
				m_comboboxData = SCR_BaseEditorAttributeFloatStringValues.Cast(entry);
				if (m_comboboxData)
				{
					int count = m_comboboxData.GetValueCount();
					
					for (int i = 0; i < count; i++)
					{
						m_ComboBoxComponent.AddItem(m_comboboxData.GetValuesEntry(i).GetName());
					}
					
					continue;
				}
			}
			
			//Drop box uses text entries
			if (!m_comboboxData)
			{
				int entriesCount = entries.Count();
				for (int i = 0; i < entriesCount; i++)
				{
					SCR_BaseEditorAttributeEntryText entry = SCR_BaseEditorAttributeEntryText.Cast(entries[i]);
					if (entry) 
						m_aDropdownArray.Insert(entry.GetText());
				}
			}
		}
		
		//~ Create the entries
		CreateDropdownEntries();
		
		super.Init(w, attribute);
	}
	
	//~ Create the actual dropdown array
	protected void CreateDropdownEntries()
	{
		if (!m_ComboBoxComponent)
			return;
		
		foreach(string entry : m_aDropdownArray)
		{
			m_ComboBoxComponent.AddItem(GetFullDropdownEntryText(entry));
		}
	}
	
	//~ Override for any additional functionality for the text entries
	protected string GetFullDropdownEntryText(string text)
	{
		return text;
	}
	
	/*!
	Get ComboBoxData from the drop down UI Attribute
	\return Combobox data
	*/
	SCR_BaseEditorAttributeFloatStringValues GetComboBoxData()
	{
		return m_comboboxData;
	}
	
	//Sets a default state for the UI and var value if conflicting attribute
	override void SetVariableToDefaultValue(SCR_BaseEditorAttributeVar var)
	{
		m_ComboBoxComponent.SetCurrentItem(0);
		
		if (var)
			var.SetInt(0);
	}
	
	override void SetFromVar(SCR_BaseEditorAttributeVar var)
	{
		super.SetFromVar(var);
		
		if (!var)
			return;
		
		m_ComboBoxComponent.SetCurrentItem(var.GetInt(), false, false);
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
	
	protected void OnChangeComboBox(SCR_ComboBoxComponent comboBox, int value)
	{
		if (m_bInitCalled)
			OnChangeInternal(comboBox.GetRootWidget(), value, 0, false);
	}
	
	
	override void HandlerDeattached(Widget w)
	{
		if (m_ComboBoxComponent)
			m_ComboBoxComponent.m_OnChanged.Remove(OnChangeComboBox);		
	}
};