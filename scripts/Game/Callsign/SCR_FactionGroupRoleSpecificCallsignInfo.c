
//! Stores info about possible callsigns for a specific group role.
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_EGroupRole, "m_GroupRole")]
class SCR_FactionGroupRoleSpecificCallsignInfo
{
	[Attribute(SCR_EGroupRole.ASSAULT.ToString(), UIWidgets.ComboBox, enumType: SCR_EGroupRole)]
	protected SCR_EGroupRole m_GroupRole;

	[Attribute()]
	protected ref array<ref SCR_CallsignInfo> m_aCallsigns;

	//------------------------------------------------------------------------------------------------
	SCR_EGroupRole GetGroupRole()
	{
		return m_GroupRole;
	}

	//------------------------------------------------------------------------------------------------
	//! \return list of callsigns as strings
	array< string> GetCallsignNames()
	{
		array<string> callsignNames = {};
		foreach (SCR_CallsignInfo callsign : m_aCallsigns)
		{
			callsignNames.Insert(callsign.GetCallsign());
		}

		return callsignNames;
	}
}