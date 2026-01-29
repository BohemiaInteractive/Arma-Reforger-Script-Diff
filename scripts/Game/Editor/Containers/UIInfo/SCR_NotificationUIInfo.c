//! UIInfo used by the Notifications system
[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_UINotificationInfo : SCR_UIInfo
{	
	[Attribute("0", UIWidgets.ComboBox, "Notification Color", "", ParamEnumArray.FromEnum(ENotificationColor) )]
	protected ENotificationColor m_cNotificationColor;
	
	[Attribute("0", UIWidgets.ComboBox, "Editor Position Data, Set the teleport position to the notification location either never (no teleport) once (where it happened) or Update (Follows whatever was the target of the notification)", "", ParamEnumArray.FromEnum(ENotificationSetPositionData) )]
	protected ENotificationSetPositionData m_EditorSetPositionData;
	
	//------------------------------------------------------------------------------------------------
	//! Get the notification color enum to set the widgets to in the notification message
	//! \return Notification widget color in enum that the Notification log converts to a color
	ENotificationColor GetNotificationColor()
	{
		return m_cNotificationColor;
	}

	//------------------------------------------------------------------------------------------------
	//! Get type of: if and how the teleport to notification works
	//! \return Teleport enum that the Notification message to: Go to location, go to updated location or never go to location
	ENotificationSetPositionData GetEditorSetPositionData()
	{
		return m_EditorSetPositionData;
	}	
}
