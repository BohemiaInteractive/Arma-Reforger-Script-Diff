//! Attribute base for Name, icon and float value for other attributes to inherent from
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_BaseFloatValueHolderEditorAttribute: SCR_BaseEditorAttribute
{
	[Attribute(desc: "Values")]
	protected ref array<ref SCR_EditorAttributeFloatStringValueHolder> m_aValues;
	
	//------------------------------------------------------------------------------------------------
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		outEntries.Insert(new SCR_BaseEditorAttributeFloatStringValues(m_aValues));
		return outEntries.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	protected int ConvertValueToIndex(float value)
	{
		foreach (int index, SCR_EditorAttributeFloatStringValueHolder valueHolder : m_aValues)
		{
			if (valueHolder.GetFloatValue() == value)
				return index;
		}
		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool ConvertIndexToValue(int index, out float outValue)
	{
		if (!m_aValues.IsIndexValid(index))
			return false;
		
		outValue = m_aValues[index].GetFloatValue();
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Validate if the attribute has the enums set up correctly
	protected bool ValidateEnumValues(typename enumType, bool checkEnumCount = true)
	{
		 array<int> enumValues = {};
		if (m_aValues.Count() != SCR_Enum.GetEnumValues(enumType, enumValues) && checkEnumCount)
			Print("SCR_BaseFloatValueHolderEditorAttribute enum value count is not equal to the attribute list count. Some enums of type '" + enumType + "' might not be able to be set via the attribute config.", LogLevel.WARNING);
		
		foreach (SCR_EditorAttributeFloatStringValueHolder value : m_aValues)
		{
			//~ Check if the values in the config exists in the enum value
			if (!enumValues.Contains(value.GetFloatValue()))
			{
				Print("SCR_BaseFloatValueHolderEditorAttribute has an invalid value it is trying to set ('" + value.GetFloatValue() + "')  which does not exist in the '" + enumType + "' enum", LogLevel.ERROR);		
				return false;
			}
		}
		
		return true;
	}
};

[BaseContainerProps(), BaseContainerCustomTitleField("m_sEntryName")]
class SCR_EditorAttributeFloatStringValueHolder
{	
	[Attribute()]
	protected ResourceName m_Icon;
	
	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sEntryName;
	
	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sEntryDescription;
	
	[Attribute()]
	protected float m_fEntryFloatValue;
	
	// Hack to get this to work with imagesets, as it was made without them in mind
	protected string m_sIconSetPath;
	protected string m_sIconName;
	protected bool m_bIsImageset;
	
	//------------------------------------------------------------------------------------------------
	void SetIcon(ResourceName newIcon, bool isImageset = false)
	{
		m_Icon = newIcon;
		m_bIsImageset = isImageset;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetIcon()
	{
		return m_Icon;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetIconName()
	{
		return m_sIconName;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsImageset()
	{
		return m_bIsImageset;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetName(string newName)
	{
		m_sEntryName = newName;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetName()
	{
		return m_sEntryName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDescription(string newDescription)
	{
		m_sEntryDescription = newDescription;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetDescription()
	{
		return m_sEntryDescription;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFloatValue(float newValue)
	{
		m_fEntryFloatValue = newValue;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetFloatValue()
	{
		return m_fEntryFloatValue;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWithUIInfo(SCR_UIInfo info, float value)
	{
		SetName(info.GetName());
		SetIcon(info.GetIconPath());
		SetFloatValue(value);
		
		if (m_Icon.IsEmpty())
		{
			SetIcon(info.GetImageSetPath(), true);
			m_sIconName = info.GetIconSetName();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWithUIInfo(UIInfo info, float value)
	{
		SetName(info.GetName());
		SetIcon(info.GetIconPath());
		SetFloatValue(value);
	}

}
