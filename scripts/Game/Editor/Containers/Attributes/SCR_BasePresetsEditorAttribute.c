/**
Attribute base for Name, icon and float value for other attributes to inherent from
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_BasePresetsEditorAttribute: SCR_BaseFloatValueHolderEditorAttribute
{	
	[Attribute("3", "How many buttons should there be displayed on a row")]
	protected int m_iButtonsOnRow;
	
	[Attribute("0", "Adds a randomzie button to select a random entity (Only works with single select)")]
	protected bool m_bHasRandomizeButton;
	
	[Attribute("", "Icon of randomize Button")]
	protected ResourceName m_sRandomizeButtonIcon;
	
	[Attribute("1", "Set false if the button should never try to use icons instead of text")]
	protected bool m_bUseIcon;	
	
	[Attribute("1", "If true it will show the given description (it will use Name if no description is given) when hovering over the button")]
	protected bool m_bHasButtonDescription;
	
	[Attribute("%1", "If m_bHasButtonDescription is true then it will use this string as description (and entry description (or if empty enity name as Param %1)) this means using %1 will simply display the entry description/name")]
	protected LocalizedString m_sButtonDescription;
	
	[Attribute("-1", "Set -1 to have UI set size")]
	protected float m_fButtonHeight;
	
	protected void CreatePresets()
	{
	}
	
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		CreatePresets();
		outEntries.Insert(new SCR_EditorAttributePresetEntry(m_iButtonsOnRow, m_bHasRandomizeButton, m_sRandomizeButtonIcon, m_bUseIcon, m_bHasButtonDescription, m_sButtonDescription, m_fButtonHeight));
		outEntries.Insert(new SCR_BaseEditorAttributeFloatStringValues(m_aValues));
		return outEntries.Count();
	}
};