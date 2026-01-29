//
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_LoadoutBaseEditorAttribute : SCR_BaseMultiSelectPresetsEditorAttribute
{
	[Attribute()]
	protected ref SCR_ArsenalItemTypeUIConfig m_ArsenalItemTypeConfig;
	
	[Attribute("0", UIWidgets.Flags, "", enums: ParamEnumArray.FromEnum(SCR_EArsenalItemType))]
	protected SCR_EArsenalItemType m_eInventoryItemTypes;
	
	
	protected override void CreatePresets()
	{
		array<ref SCR_ArsenalItemTypeUI> ArsenalUIInfo = m_ArsenalItemTypeConfig.GetAvailableItems();
		
		if (ArsenalUIInfo.IsEmpty())
			return;
		
		//~ Safty
		array<int> intValues = new array<int>;
		if (SCR_Enum.GetEnumValues(SCR_EArsenalItemType, intValues) != ArsenalUIInfo.Count())
			Print("List of supported item types for editor arsenal and loadout is a diffrent length from Arsenal Item Type UI config. Make sure that any new 'SCR_EArsenalItemType' enum also has an entry in 'SCR_ArsenalItemTypeUIConfig'", LogLevel.WARNING);
		
		m_aValues.Clear();
		SCR_EditorAttributeFloatStringValueHolder value;
		foreach (SCR_ArsenalItemTypeUI item: ArsenalUIInfo)
		{
			if (m_eInventoryItemTypes & item.GetItemType())
			{
				value = new SCR_EditorAttributeFloatStringValueHolder();
				value.SetWithUIInfo(item.GetInfo(), item.GetItemType());
				m_aValues.Insert(value);
			}
		
		}
	}
};