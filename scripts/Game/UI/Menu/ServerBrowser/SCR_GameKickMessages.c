[BaseContainerProps(configRoot : true)]
class SCR_ErrorDialogMessages
{
	[Attribute("", UIWidgets.Object, "Groups of game kick caused under which cause codes are aggretated")]
	ref array<ref SCR_ErrorDialogMessageGroup> m_aKickCauseGroups;
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sName")]
class SCR_ErrorDialogMessageGroup : SCR_ErrorDialogMessageEntry
{	
	[Attribute("", UIWidgets.EditBox, "General error that will be shown if no spefic entry if verified")]
	string m_sFallbackEntryName;
	
	[Attribute("", UIWidgets.Object, "Entries in group")]
	ref array<ref SCR_ErrorDialogMessageEntry> m_aKickCauseEntries;
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sName")]
class SCR_ErrorDialogMessageEntry
{
	[Attribute("", UIWidgets.EditBox, "Name of group")]
	string m_sName;
	
	[Attribute("", UIWidgets.EditBox, "String that will be display in dialog on this kick reason")]
	string m_sNameLocalized;
};