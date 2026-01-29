//------------------------------------------------------------------------------------------------	
class SCR_ScriptProfanityFilterRequestCallback : ScriptProfanityFilterRequestCallback
{
	ref ScriptInvoker m_OnResult = new ScriptInvoker; // (array<string> filteredTexts)
	ref array<string> m_FilteredTexts = new array<string>();
	
	//------------------------------------------------------------------------------------------------
	override void OnFilteredResult()
	{
		GetTexts(m_FilteredTexts);
		m_OnResult.Invoke(m_FilteredTexts);
	}	
};
