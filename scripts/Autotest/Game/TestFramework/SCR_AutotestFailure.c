/*!
Game test failure.
*/
class SCR_AutotestFailure : TestFailureBase
{
	protected string m_sFailureReason;

	//------------------------------------------------------------------------------------------------
	//! Create new failure.
	//! \param[in] reason Reason for the failure, supports string interpolation.
	//! \param[in] param1 String param
	//! \param[in] param2 String param
	//! \param[in] param3 String param
	//! \return Failure
	static SCR_AutotestFailure Create(string reason, string param1 = "", string param2 = "", string param3 = "")
	{
		SCR_AutotestFailure failure = new SCR_AutotestFailure(string.Format(reason, param1, param2, param3));
		return failure;
	}

	//------------------------------------------------------------------------------------------------
	//! Plain failure text.
	string GetFailureReason()
	{
		return m_sFailureReason;
	}

	//------------------------------------------------------------------------------------------------
	//! Text used for xml report output.
	override string FailureText()
	{
		// do not use string.Format,
		// limit of 8191 characers can resut in malformed XML if failure reason is too long
		return "<failure type=\"Result\">" + TestHarness.EscapeForXml(m_sFailureReason) + "</failure>";
	}

	//------------------------------------------------------------------------------------------------
	// Private constructor enforces usage of static factory methods for instantiation.
	private void SCR_AutotestFailure(string reason = "")
	{
		m_sFailureReason = reason;
	}
}
