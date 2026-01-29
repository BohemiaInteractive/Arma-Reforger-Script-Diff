//! Dynamic description for arsenal save types. What they mean when they are set
[BaseContainerProps(), BaseContainerCustomStringTitleField("Arsenal Save type description (CUSTOM)")]
class SCR_ArsenalSaveTypeAttributeDynamicDescription : SCR_BaseAttributeDynamicDescription
{
	protected ref array<SCR_ArsenalSaveTypeInfo> m_aArsenalSaveTypeInfo = {};
	
	//------------------------------------------------------------------------------------------------
	override void InitDynamicDescription(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi)
	{
		super.InitDynamicDescription(attribute);
		
		if (!attribute.IsInherited(SCR_ArsenalSaveTypeAttribute))
		{
			Print("'SCR_ArsenalSaveTypeAttributeDynamicDescription' is not attached to the 'SCR_ArsenalSaveTypeAttribute'!", LogLevel.ERROR);
			return;
		}
			
		SCR_ArsenalManagerComponent arsenalManager;
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return;
		
		SCR_ArsenalSaveTypeInfoHolder arsenalSaveTypeHolder;
		arsenalSaveTypeHolder = arsenalManager.GetArsenalSaveTypeInfoHolder();
		if (!arsenalSaveTypeHolder)
			return;
		
		arsenalSaveTypeHolder.GetArsenalSaveTypeInfoList(m_aArsenalSaveTypeInfo);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsValid(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi)
	{		
		if (!super.IsValid(attribute, attributeUi) || !attribute.IsInherited(SCR_ArsenalSaveTypeAttribute))
			return false;
		
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (!var)
			return false;
		
		if (m_aArsenalSaveTypeInfo.IsEmpty() || !m_aArsenalSaveTypeInfo.IsIndexValid(var.GetInt()) || !m_aArsenalSaveTypeInfo[var.GetInt()].GetUIInfo())
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void GetDescriptionData(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi, out SCR_EditorAttributeUIInfo uiInfo, out string param1 = string.Empty, out string param2 = string.Empty, out string param3 = string.Empty)
	{
		SCR_ArsenalSaveTypeUIInfo saveTypeUIInfo = m_aArsenalSaveTypeInfo[attribute.GetVariableOrCopy().GetInt()].GetUIInfo();
		
		param1 = saveTypeUIInfo.GetDescription();
		uiInfo = m_DescriptionDisplayInfo;
	}
}
