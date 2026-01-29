[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_sDeptName", true)]
class SCR_CreditsDepartmentHeader
{
	[Attribute("", UIWidgets.EditBox, "Enter the department name for Credits screen here.")]
	private string m_sDeptName;

	[Attribute()]
	private ref array<ref SCR_CreditsPersonName> m_aCreditsNames;

	//------------------------------------------------------------------------------------------------
	void GetCreditsNamesList(out notnull array<ref SCR_CreditsPersonName> nameList)
	{
		nameList = m_aCreditsNames;
	}

	//------------------------------------------------------------------------------------------------
	string GetDeptName()
	{
		return m_sDeptName;
	}
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_sPersonName", true)]
class SCR_CreditsPersonName
{
	[Attribute("", UIWidgets.EditBox, "Name.")]
	private string m_sPersonName;

	//------------------------------------------------------------------------------------------------
	string GetPersonName()
	{
		return m_sPersonName;
	}
};

[BaseContainerProps(configRoot: true)]
class SCR_CreditsHeader
{
	[Attribute(desc: "List of Departments.")]
	private ref array<ref SCR_CreditsDepartmentHeader> m_aDepartmentList;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Credit backgrounds list.", params: "edds")]
	private ref array<ResourceName> m_aBackgroundList;

	//------------------------------------------------------------------------------------------------
	void GetCreditsDepartmentList(out notnull array<ref SCR_CreditsDepartmentHeader> deptList)
	{
		deptList = m_aDepartmentList;
	}

	//------------------------------------------------------------------------------------------------
	void GetBackgrounds(out notnull array<ResourceName> backgroudList)
	{
		backgroudList = m_aBackgroundList;
	}
};
