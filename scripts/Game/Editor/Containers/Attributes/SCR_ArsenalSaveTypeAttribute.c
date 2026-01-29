[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_ArsenalSaveTypeAttribute : SCR_BaseFloatValueHolderEditorAttribute
{
	protected SCR_ArsenalSaveTypeInfoHolder m_ArsenalSaveTypeHolder;
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return null;
		
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.FindArsenalComponent(editableEntity.GetOwner());
		if (!arsenalComponent)
			return null;

		if (!arsenalComponent || !arsenalComponent.HasSaveArsenalAction())
			return null;
		
		SCR_ArsenalSaveTypeInfoHolder arsenalSaveTypeHolder = GetArsenalSaveTypeInfoHolder();
		if (!arsenalSaveTypeHolder)
			return null;
		
		array<SCR_ArsenalSaveTypeInfo> arsenalSaveTypeUIInfoList = {};
		int count = arsenalSaveTypeHolder.GetArsenalSaveTypeInfoList(arsenalSaveTypeUIInfoList);
		
		//~ Set var to type. If arsenal type is not valid than don't show attribute
		for (int i = 0; i < count; i++)
		{
			if (arsenalSaveTypeUIInfoList[i].GetSaveType() == arsenalComponent.GetArsenalSaveType())
				return SCR_BaseEditorAttributeVar.CreateInt(i);
		}

		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ArsenalSaveTypeInfoHolder GetArsenalSaveTypeInfoHolder()
	{
		if (m_ArsenalSaveTypeHolder)
			return m_ArsenalSaveTypeHolder;
		
		SCR_ArsenalManagerComponent arsenalManager;
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return null;
		
		m_ArsenalSaveTypeHolder = arsenalManager.GetArsenalSaveTypeInfoHolder();
		return m_ArsenalSaveTypeHolder;
	}
	
	//------------------------------------------------------------------------------------------------
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		FillValues();
		outEntries.Insert(new SCR_BaseEditorAttributeFloatStringValues(m_aValues));
		return outEntries.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FillValues()
	{
		SCR_ArsenalSaveTypeInfoHolder arsenalSaveTypeHolder = GetArsenalSaveTypeInfoHolder();
		if (!arsenalSaveTypeHolder)
			return;

		m_aValues.Clear();
		array<SCR_ArsenalSaveTypeInfo> arsenalSaveTypeUIInfoList = {};
		int count = arsenalSaveTypeHolder.GetArsenalSaveTypeInfoList(arsenalSaveTypeUIInfoList);
		SCR_EditorAttributeFloatStringValueHolder value;
		SCR_ArsenalSaveTypeUIInfo uiInfo;
		
		for(int i = 0; i < count; i++)
		{
			uiInfo = arsenalSaveTypeUIInfoList[i].GetUIInfo();
			if (!uiInfo)
				continue;
			
			value = new SCR_EditorAttributeFloatStringValueHolder();
			value.SetName(uiInfo.GetName());
			value.SetFloatValue(arsenalSaveTypeUIInfoList[i].GetSaveType());
			
			m_aValues.Insert(value);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return;
		
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.FindArsenalComponent(editableEntity.GetOwner());
		if (!arsenalComponent)
			return;

		if (!arsenalComponent || !arsenalComponent.HasSaveArsenalAction())
			return;
		
		SCR_ArsenalSaveTypeInfoHolder arsenalSaveTypeHolder = GetArsenalSaveTypeInfoHolder();
		if (!arsenalSaveTypeHolder)
			return;
		
		array<SCR_ArsenalSaveTypeInfo> arsenalSaveTypeUIInfoList = {};
		arsenalSaveTypeHolder.GetArsenalSaveTypeInfoList(arsenalSaveTypeUIInfoList);
		
		int value = var.GetInt();
		
		if (!arsenalSaveTypeUIInfoList.IsIndexValid(value))
			return;
		
		arsenalComponent.SetArsenalSaveType(arsenalSaveTypeUIInfoList[value].GetSaveType());
	}
}
