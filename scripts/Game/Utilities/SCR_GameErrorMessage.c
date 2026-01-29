//------------------------------------------------------------------------------------------------
//! Wrapper for error messages
class SCR_GameErrorMessage
{
	protected string m_sMessage;
	protected string m_sTitle;
	
	//------------------------------------------------------------------------------------------------
	string GetMessage()
	{
		return m_sMessage;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetTitle()
	{
		return m_sTitle;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_GameErrorMessage(string message, string title = "Error")
	{
		m_sMessage = message;
		m_sTitle = title;
	}
};