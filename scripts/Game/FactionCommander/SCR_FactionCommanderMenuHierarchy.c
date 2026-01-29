[BaseContainerProps(configRoot: true)]
class SCR_FactionCommanderMenuHierarchy
{
	[Attribute()]
	protected ref array<ref SCR_FactionCommanderMenuEntry> m_aEntriesHierarchy;

	//------------------------------------------------------------------------------------------------
	void GetEntries(out notnull array<ref SCR_FactionCommanderMenuEntry> entries)
	{
		if (m_aEntriesHierarchy)
			entries = m_aEntriesHierarchy;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sName")]
class SCR_FactionCommanderMenuEntry
{
	[Attribute("1")]
	protected bool m_bEnabled;

	[Attribute("", desc: "System Kname to identify this entry by.")]
	protected string m_sName;

	[Attribute("", desc: "Text used in radial menu UI.")]
	protected string m_sDisplayName;

	[Attribute("{3262679C50EF4F01}UI/Textures/Icons/icons_wrapperUI.imageset", UIWidgets.ResourceNamePicker, "Imageset with the desired icon.", "imageset")]
	ResourceName m_sIconImageset;

	[Attribute("", desc: "Imageset quad name.")]
	protected string m_sIconName;

	[Attribute("0", desc: "When enabled, this entry will open the list of groups Commander can issue the associated order to.")]
	protected bool m_bGroupOrder;

	[Attribute("0", desc: "Accessible by squad leaders. Group and faction assignment will be available only to the Commander.")]
	protected bool m_bSupportRequest;

	[Attribute("0", desc: "This entry will be a menu category regardless of it being a group order or having any children (commonly to be expanded later in script).")]
	protected bool m_bForceCategoryClass;

	[Attribute(desc: "Class for evaluating conditions for visibility and availability of the given entry.")]
	protected ref SCR_FactionCommanderBaseMenuHandler m_MenuHandler;

	[Attribute()]
	protected ref array<ref SCR_FactionCommanderMenuEntry> m_aChildEntries;
	
	[Attribute("0", desc: "Show a hint for this objective when true")]
	protected bool m_bShowHint;
	
	[Attribute(uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EHint))]
	protected EHint m_eHintId;
	
	//------------------------------------------------------------------------------------------------
	bool GetShowHint()
	{
		return m_bShowHint;
	}
	
	//------------------------------------------------------------------------------------------------
	EHint GetHintId()
	{
		return m_eHintId;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled()
	{
		return m_bEnabled;
	}

	//------------------------------------------------------------------------------------------------
	string GetName()
	{
		return m_sName;
	}

	//------------------------------------------------------------------------------------------------
	string GetDisplayName()
	{
		return m_sDisplayName;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetIconImageset()
	{
		return m_sIconImageset;
	}

	//------------------------------------------------------------------------------------------------
	string GetIconName()
	{
		return m_sIconName;
	}

	//------------------------------------------------------------------------------------------------
	bool IsGroupOrder()
	{
		return m_bGroupOrder;
	}

	//------------------------------------------------------------------------------------------------
	bool IsSupportRequest()
	{
		return m_bSupportRequest;
	}

	//------------------------------------------------------------------------------------------------
	bool IsCategory()
	{
		return m_bForceCategoryClass;
	}

	//------------------------------------------------------------------------------------------------
	SCR_FactionCommanderBaseMenuHandler GetMenuHandler()
	{
		return m_MenuHandler;
	}

	//------------------------------------------------------------------------------------------------
	void GetEntries(out notnull array<ref SCR_FactionCommanderMenuEntry> entries)
	{
		if (m_aChildEntries)
			entries = m_aChildEntries;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sName")]
class SCR_FactionCommanderTaskRootMenuEntry : SCR_FactionCommanderMenuEntry
{
	[Attribute("", UIWidgets.ResourceNamePicker, "", "et")]
	protected ResourceName m_sTaskPrefab;

	protected typename m_TaskTypename;

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetTaskPrefab()
	{
		return m_sTaskPrefab;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	typename GetTaskPrefabClassTypename()
	{
		if (m_TaskTypename)
			return m_TaskTypename;

		if (m_sTaskPrefab.IsEmpty())
			return typename.Empty;

		Resource resource = BaseContainerTools.LoadContainer(m_sTaskPrefab);
		if (!resource || !resource.IsValid())
		{
			Print("Task prefab resource name is not valid", LogLevel.WARNING);
			return typename.Empty;
		}

		BaseResourceObject resourceObject = resource.GetResource();
		if (!resourceObject)
			return typename.Empty;

		BaseContainer container = resourceObject.ToBaseContainer();
		m_TaskTypename = container.GetClassName().ToType();
		return m_TaskTypename;
	}
}
