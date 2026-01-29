/*
Component for configuring dialog data from editor.
*/

//------------------------------------------------------------------------------------------------
class SCR_DialogDataComponent: ScriptedWidgetComponent
{
	[Attribute("", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(EDialogType))]
	EDialogType m_iDialogType;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "")]
	ResourceName m_sIconSet;
};

//------------------------------------------------------------------------------------------------
class SCR_ErrorDialogDataComponent: SCR_DialogDataComponent
{
	[Attribute()]
	ref SCR_ErrorDialogMessages m_ErrorDialogMessages;
};