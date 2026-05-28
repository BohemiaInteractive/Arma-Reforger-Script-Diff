//! Collection of test suites.
[BaseContainerProps(configRoot: true, category: "Autotest")]
class SCR_AutotestGroup
{
	[Attribute()]
	protected ref array<ref SCR_AutotestSuiteBase> m_aSuites;
	
	[Attribute()]
	protected ref array<ref SCR_AutotestGroup> m_aGroups;

	//------------------------------------------------------------------------------------------------
	//! Get all test suites in this group.
	array<ref SCR_AutotestSuiteBase> GetSuites(bool ungroupedSuitesOnly = false)
	{
		if (ungroupedSuitesOnly)
			return m_aSuites;

		array<ref SCR_AutotestSuiteBase> allSuites = {};
		foreach (SCR_AutotestSuiteBase suite : m_aSuites)
		{
			allSuites.Insert(suite);
		}

		foreach (SCR_AutotestGroup group : m_aGroups)
		{
			array<ref SCR_AutotestSuiteBase> groupSuites = group.GetSuites();
			foreach (SCR_AutotestSuiteBase suite : groupSuites)
			{
				allSuites.Insert(suite);
			}
		}

		return allSuites;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Get all test groups in this group.
	array<ref SCR_AutotestGroup> GetGroups()
	{
		return m_aGroups;
	}
}
