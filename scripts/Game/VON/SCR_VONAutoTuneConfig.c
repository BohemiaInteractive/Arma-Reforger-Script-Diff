//! Player role for auto tune setting
enum SCR_EVONAutoTuneRole
{
	ANY,
	COMMANDER,
	SQUAD_LEADER,
	SQUAD_MEMBER,
}

//! Frequency that can be tuned
enum SCR_EVONAutoTuneFrequency
{
	NONE,
	SQUAD,
	PLATOON,
	OBJECTIVE_PLATOON,
	COMMANDER,
}

//! VON auto tune config root
[BaseContainerProps(configRoot: true)]
class SCR_VONAutoTuneConfig
{
	[Attribute()]
	ref array<ref SCR_VONAutoTuneIdentityConfig> m_aAutoTuneList;
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ERadioType, "m_eRadioType", "Radio type: %1")]
class SCR_VONAutoTuneIdentityConfig
{
	[Attribute(uiwidget: UIWidgets.ComboBox, desc:"Radio type", enumType: ERadioType)]
	ERadioType m_eRadioType;

	[Attribute()]
	string m_sFactionKey;

	[Attribute()]
	ref array<ref SCR_VONAutoTuneRoleConfig> m_aRoles;
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_EVONAutoTuneRole, "m_eRole", "Player role: %1")]
class SCR_VONAutoTuneRoleConfig
{
	[Attribute(uiwidget: UIWidgets.ComboBox, desc:"Player role for which the frequency is tuned", enumType: SCR_EVONAutoTuneRole)]
	SCR_EVONAutoTuneRole m_eRole;

	[Attribute(desc:"List of channels that can be tuned")]
	ref array<ref SCR_VONAutoTuneChannelConfig> m_aChannels;
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_EVONAutoTuneFrequency, "m_eAutoTuneFrequency", "Radio frequency: %1")]
class SCR_VONAutoTuneChannelConfig
{
	[Attribute(desc:"Channel", params:"0 100")]
	int m_iChannel;

	[Attribute(uiwidget: UIWidgets.ComboBox, enumType: SCR_EVONAutoTuneFrequency)]
	SCR_EVONAutoTuneFrequency m_eAutoTuneFrequency;
}
