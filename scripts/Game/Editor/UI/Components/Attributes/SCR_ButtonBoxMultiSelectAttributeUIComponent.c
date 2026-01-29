/** @ingroup Editor_UI Editor_UI_Components Editor_UI_Attributes
*/
class SCR_ButtonBoxMultiSelectAttributeUIComponent: SCR_ButtonBoxAttributeUIComponent
{	
	protected bool m_bUseCustomFlags = false;
	
	override void Init(Widget w, SCR_BaseEditorAttribute attribute)
	{
		//Get if uses custom flags
		array<ref SCR_BaseEditorAttributeEntry> entries = new array<ref SCR_BaseEditorAttributeEntry>;
		attribute.GetEntries(entries);
		
		foreach (SCR_BaseEditorAttributeEntry entry: entries)
		{	
			SCR_EditorAttributePresetMultiSelectEntry multiSelectPresetEntry = SCR_EditorAttributePresetMultiSelectEntry.Cast(entry);
			if (multiSelectPresetEntry)
			{
				m_bUseCustomFlags = multiSelectPresetEntry.GetUsesCustomFlags();
				break;	
			}
		}
		
		super.Init(w, attribute);
	}
	
	override void SetFromVar(SCR_BaseEditorAttributeVar var)
	{
		if (!var)
			return;
		
		int count = m_ToolBoxComponent.GetNumItems();
		vector flags = var.GetVector();
		int value;
		
		if (!m_bUseCustomFlags)
		{
			int index = 0;
			int flagPerIndex = 0;
			int flag = 1;

			//Set State
			for (int i = 0; i < count; i++)
			{	
				value = flags[index];
				m_ToolBoxComponent.SetItemSelected(i, (value & flag), false);	
				flag *= 2;
				flagPerIndex++;
				
				if (flagPerIndex >= 31)
				{
					flagPerIndex = 0;
					flag = 1;
					index++;
					
					if (index > 2)
						break;
				}
			}
		}
		else 
		{
			value = flags[0];
			
			//Check if has custom flag
			for (int i = 0; i < count; i++)
			{
				m_ToolBoxComponent.SetItemSelected(i, (value & (int)m_ButtonBoxData.GetEntryFloatValue(i)), false);	
			}	
		
		}
		
		m_bButtonValueInitCalled = true;
		
		SetFromVarOrDefault();
	}
	
	//x is index, y is selectState
	override bool OnChangeInternal(Widget w, int x, int y, bool finished)
	{			
		if (!m_bButtonValueInitCalled)
			return false;
		
		SCR_BaseEditorAttribute attribute = GetAttribute();
		if (!attribute) 
			return false;
		SCR_BaseEditorAttributeVar var = attribute.GetVariable(true);
		
		vector flags;
		int count = m_ToolBoxComponent.GetNumItems();
		if (!m_bUseCustomFlags)
		{
			flags = Vector(0,0,0);	
			int index = 0;
			int flagPerIndex = 0;
			int flag = 1;
			int value;
			
			for (int i = 0; i < count; i++)
			{
				//If selected add
				if (m_ToolBoxComponent.IsItemSelected(i))
				{
					value = flags[index];
					value |= flag;
					flags[index] = value;
				}
				
				flag *= 2;
				flagPerIndex++;
				
				if (flagPerIndex >= 31)
				{
					flagPerIndex = 0;
					flag = 1;
					index++;
					
					if (index > 2)
						break;
				}
			}
		}
		else 
		{
			int flagValues = flags[0];
			
			for (int i = 0; i < count; i++)
			{
				if (m_ToolBoxComponent.IsItemSelected(i))
					flagValues |= (int)m_ButtonBoxData.GetEntryFloatValue(i);
				else 
					flagValues &= ~(int)m_ButtonBoxData.GetEntryFloatValue(i);
			}
			
			flags[0] = flagValues;
		}
		var.SetVector(flags);
		AttributeValueChanged();
		return false;
	}
	
	protected override void OnMultiSelectButton(SCR_ToolboxComponent toolbox, int index, bool state)
	{		
		super.OnMultiSelectButton(toolbox, index, state);
		OnChangeInternal(toolbox.GetRootWidget(), index, state, false);
	}
};