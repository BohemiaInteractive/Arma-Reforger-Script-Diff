//! Collection of test suites.
[BaseContainerProps(configRoot: true, category: "Autotest")]
class SCR_AutotestGroup
{
	[Attribute()]
	protected ref array<ref SCR_AutotestSuiteBase> m_aSuites;

	//------------------------------------------------------------------------------------------------
	//! Get all test suites in this group.
	array<ref SCR_AutotestSuiteBase> GetSuites()
	{
		return m_aSuites;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get enabled test suites in this group.
	array<ref SCR_AutotestSuiteBase> GetEnabledSuites()
	{
		array<ref SCR_AutotestSuiteBase> suites = {};

		foreach (SCR_AutotestSuiteBase suite : GetSuites())
		{
			if (suite.ShouldSkipInGroup())
				suites.Insert(suite);
		}

		return suites;
	}
}
