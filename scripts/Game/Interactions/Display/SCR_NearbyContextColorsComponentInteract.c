enum SCR_ENearbyInteractionContextColors
{
	GENERIC,
	DEFAULT,
	HEALING,
	STABLE,
	MEDIUM,
	SEVERE
}

[BaseContainerProps()]
class SCR_NearbyContextColorsComponentInteract
{
	[Attribute(SCR_ENearbyInteractionContextColors.GENERIC.ToString(), UIWidgets.ComboBox, string.Empty, string.Empty, enumType: SCR_ENearbyInteractionContextColors)]
	SCR_ENearbyInteractionContextColors m_eId;

	[Attribute(UIColors.GetColorAttribute(UIColors.NEUTRAL_INFORMATION), UIWidgets.ColorPicker)]
	ref Color m_IconColor;

	[Attribute(UIColors.GetColorAttribute(UIColors.DARK_GREY), UIWidgets.ColorPicker)]
	ref Color m_IconGlowColor;

	[Attribute(UIColors.GetColorAttribute(Color.Black), UIWidgets.ColorPicker)]
	ref Color m_BackgroundColor;

	[Attribute(UIColors.GetColorAttribute(UIColors.INTERACT_GENERIC_SECONDARY), UIWidgets.ColorPicker)]
	ref Color m_OuterBackgroundColor;
}
