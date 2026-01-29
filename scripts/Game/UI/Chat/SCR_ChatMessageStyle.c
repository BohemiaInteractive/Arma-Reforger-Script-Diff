/*
Config which specifies the appearence of the chat messasge.
*/

[BaseContainerProps(configRoot: true)]
class SCR_ChatMessageStyle : Managed
{
	[Attribute("", UIWidgets.ColorPicker, "Color")]
	ref Color m_Color;
	
	[Attribute("", UIWidgets.CheckBox, "Badge is colored")]
	bool m_bColoredBadge;

	[Attribute(defvalue: "1", desc: "If game should color the badge of the message (white rectangle on the left side) based on the sender's relation to the local player.\nThis property is ignored if \"Colored Badge\" is true")]
	bool m_bColorBadgeWithRelationColor;

	[Attribute(defvalue: "-358144", uiwidget : UIWidgets.ColorPicker, desc: "Badge color for the commander")]
	int m_iBadgeColorForCommander;
	
	[Attribute("", UIWidgets.CheckBox, "Icon is colored")]
	bool m_bColoredIcon;
	
	[Attribute("", UIWidgets.CheckBox, "Background is colored")]
	bool m_bColoredBackground;

	[Attribute(defvalue: "1", desc: "If game should color the background of the message for the commander.\nThis property is skipped when \"Colored Background\" is used")]
	bool m_bColorCommanderBackground;

	[Attribute(defvalue: "-15198208", uiwidget : UIWidgets.ColorPicker, desc: "Background color for the commander")]
	int m_iCommanderBackgroundColor;
	
	[Attribute("", UIWidgets.CheckBox, "Player name is colored")]
	bool m_bColoredPlayerName;
	
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Imageset for the icon", params: "imageset")]
	ResourceName m_IconImageset;
	
	[Attribute("", UIWidgets.EditBox, "Image name in the imageset")]
	string m_sIconName;
	
	[Attribute("<Name>", UIWidgets.EditBox, "Channel name")]
	string m_sName;
	
	[Attribute("<name>", UIWidgets.EditBox, "Channel name, all lowercase")]
	string m_sNameLower;
	
	[Attribute("/0", UIWidgets.EditBox, "Prefix to quickly send a message to that channel")]
	string m_sPrefix;
};