[BaseContainerProps(configRoot: true)]
class SCR_CampaignRadioMsgWhitelistConfig
{
	[Attribute("1", desc:"If checked, ignores the whitelist and allows all radio messages.")]
	protected bool m_bEnableAll;

	[Attribute(uiwidget: UIWidgets.ComboBox, desc:"Whitelist of allowed radio messages", enumType: SCR_ERadioMsg)]
	protected ref array<SCR_ERadioMsg> m_aWhitelistedRadioMessages;

	//------------------------------------------------------------------------------------------------
	//! Can send radio message
	//! \param[in] radioMsg
	//! \return true if the radio message is allowed to be send
	bool CanSendRadioMessage(SCR_ERadioMsg radioMsg)
	{
		if (m_bEnableAll)
			return true;

		return m_aWhitelistedRadioMessages.Contains(radioMsg);
	}
}
