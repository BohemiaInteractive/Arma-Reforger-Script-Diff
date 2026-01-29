/*!
Callsign Info. Used for Company, Platoon and Squad. It currently holds the callsign name
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sCallsign")]
class SCR_CallsignInfo
{
	[Attribute()]
	protected LocalizedString m_sCallsign;
	
	/*!
	Get name
	\return name
	*/
	string GetCallsign()
	{
		return m_sCallsign;
	}
};
