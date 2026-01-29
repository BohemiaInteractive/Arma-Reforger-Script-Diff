/*!
Game test result.
*/
class SCR_AutotestResult : TestResultBase
{
	protected bool m_bIsSuccess;
	protected string m_sFailureReason;

	//------------------------------------------------------------------------------------------------
	//! Create new result indicating that the test succeeded.
	//! \return Success result
	static SCR_AutotestResult AsSuccess()
	{
		SCR_AutotestResult result = new SCR_AutotestResult(true);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Create new result indicating that the test failed.
	//! \param[in] reason Reason for the failure, supports string interpolation.
	//! \param[in] param1 String param
	//! \param[in] param2 String param
	//! \param[in] param3 String param
	//! \return Failure result
	static SCR_AutotestResult AsFailure(string reason, string param1 = "", string param2 = "", string param3 = "")
	{
		SCR_AutotestResult result = new SCR_AutotestResult(false, string.Format(reason, param1, param2, param3));
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Plain failure text.
	string GetFailureReason()
	{
		return m_sFailureReason;
	}

	//------------------------------------------------------------------------------------------------
	//! Is failure
	//! \return true means failure
	override bool Failure()
	{
		return !m_bIsSuccess;
	}

	//------------------------------------------------------------------------------------------------
	//! Text used for xml report output.
	override string FailureText()
	{
		return string.Format("<failure type=\"Result\">%1</failure>", m_sFailureReason);
	}

	//------------------------------------------------------------------------------------------------
	// Private constructor enforces usage of static factory methods for instantiation.
	/* private */ void SCR_AutotestResult(bool isSuccess, string reason = "")
	{
		m_bIsSuccess = isSuccess;
		m_sFailureReason = reason;
	}
}
