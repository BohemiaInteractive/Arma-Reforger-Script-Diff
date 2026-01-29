#ifdef WORKBENCH
[WorkbenchToolAttribute("Destructible Prefab Finder Tool", "Scans selected folder for prefabs containing MPD components or destructible entities", "", awesomeFontCode: 0xF468)]
class SCR_DestructiblePrefabFinderTool : WorldEditorTool
{
	[Attribute("1", UIWidgets.CheckBox, "Find prefabs containing SCR_DestructionMultiPhaseComponent in given folder", category: "Folder Scan Filters")]
	protected bool m_bFindMultiPhaseDestructionComponents;

	[Attribute("1", UIWidgets.CheckBox, "Find prefabs containing SCR_DestructibleEntity in given folder", category: "Folder Scan Filters")]
	protected bool m_bFindDestructibleEntities;

	[Attribute("1", UIWidgets.CheckBox, "Find prefabs containing SCR_DestructibleEntity as well as SCR_DestructionMultiPhaseComponent", category: "Folder Scan Filters")]
	protected bool m_bFindDestructibleEntitiesWithMPD;

	[Attribute("0", UIWidgets.CheckBox, "Include disabled MPD Components", category: "Folder Scan Filters")]
	protected bool m_bIncludeDisabledMPDComponents;

	[Attribute("1", UIWidgets.CheckBox, "Check if MPD Components have at least one hit zone attached to them", category: "Folder Scan Filters")]
	protected bool m_bCheckMPDForHitzone;

	[Attribute("1", UIWidgets.CheckBox, "Check if MPD Prefabs have RplComponent attached to them", category: "Folder Scan Filters")]
	protected bool m_bCheckMPDForRplComponent;

	[Attribute("1", UIWidgets.CheckBox, "Check if MPD Components have default values", category: "Folder Scan Filters")]
	protected bool m_bCheckMPDForDefaultValues;

	protected ref array<ResourceName> m_aSelection = {};
	protected ref array<ResourceName> m_aPrefabs = {};

	protected ref array<ResourceName> m_aMPDComponents = {};
	protected ref array<ResourceName> m_aMPDComponentsNoHitZone = {};
	protected ref array<ResourceName> m_aMPDComponentsMissingRpl = {};
	protected ref array<ResourceName> m_aMPDComponentsDefaultValues = {};

	protected ref array<ResourceName> m_aDestructibleEntities = {};
	protected ref array<ResourceName> m_aDestructibleEntitiesWithMPD = {};

	//------------------------------------------------------------------------------------------------
	protected void CheckForMPDComponent(ResourceName resourceName, Resource resource)
	{
		IEntityComponentSource componentSource = SCR_BaseContainerTools.FindComponentSource(resource, SCR_DestructionMultiPhaseComponent);

		if (!componentSource)
			return;

		bool enabled;
		if (m_bIncludeDisabledMPDComponents || (componentSource.Get("Enabled", enabled) && enabled))
		{
			m_aMPDComponents.Insert(resourceName);

			if (m_bCheckMPDForRplComponent)
			{
				IEntityComponentSource rplSource = SCR_BaseContainerTools.FindComponentSource(resource, RplComponent);
				if (!rplSource)
					m_aMPDComponentsMissingRpl.Insert(resourceName);
			}

			if (m_bCheckMPDForHitzone)
			{
				BaseContainerList hitZoneArray = componentSource.GetObjectArray("Additional hit zones");
				if (!hitZoneArray || hitZoneArray.Count() < 1)
					m_aMPDComponentsNoHitZone.Insert(resourceName);
			}

			if (m_bCheckMPDForDefaultValues)
			{
				array<string> modifiedVarNames = SCR_BaseContainerTools.GetPrefabSetValueNames(componentSource);
				if (modifiedVarNames.IsEmpty())
					m_aMPDComponentsDefaultValues.Insert(resourceName);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckForDestructibleEntity(ResourceName resourceName, Resource resource)
	{
		typename className = SCR_BaseContainerTools.GetContainerClassName(resource).ToType();
		if (className != SCR_DestructibleEntity)
			return;

		m_aDestructibleEntities.Insert(resourceName);

		if (m_bFindDestructibleEntitiesWithMPD)
		{
			IEntityComponentSource componentSource = SCR_BaseContainerTools.FindComponentSource(resource, SCR_DestructionMultiPhaseComponent);

			if (componentSource)
			{
				bool enabled;
				if (m_bIncludeDisabledMPDComponents || (componentSource.Get("Enabled", enabled) && enabled))
					m_aDestructibleEntitiesWithMPD.Insert(resourceName);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Scan Folders")]
	protected void FolderScan()
	{
		m_aSelection.Clear();
		m_aPrefabs.Clear();
		m_aMPDComponents.Clear();
		m_aMPDComponentsMissingRpl.Clear();
		m_aMPDComponentsNoHitZone.Clear();
		m_aMPDComponentsDefaultValues.Clear();
		m_aDestructibleEntities.Clear();
		m_aDestructibleEntitiesWithMPD.Clear();

		Debug.BeginTimeMeasure();

		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		worldEditor.GetResourceBrowserSelection(m_aSelection.Insert, true);

		foreach (string resourceName : m_aSelection)
		{
			if (resourceName.EndsWith(".et"))
				m_aPrefabs.Insert(resourceName);
		}

		Resource resource;
		foreach (string resourceName : m_aPrefabs)
		{
			resource = Resource.Load(resourceName);

			if (m_bFindMultiPhaseDestructionComponents)
				CheckForMPDComponent(resourceName, resource);

			if (m_bFindDestructibleEntities)
				CheckForDestructibleEntity(resourceName, resource);
		}

		Print("------------------------------------------------------------------------", LogLevel.NORMAL);

		if (m_bFindMultiPhaseDestructionComponents)
		{
			if (m_bIncludeDisabledMPDComponents)
				Print("Number of *.et files containing SCR_DestructionMultiPhaseComponent: " + m_aMPDComponents.Count(), LogLevel.NORMAL);
			else
				Print("Number of *.et files containing enabled SCR_DestructionMultiPhaseComponent: " + m_aMPDComponents.Count(), LogLevel.NORMAL);
		}

		if (m_bFindDestructibleEntities)
			Print("Number of *.et files containing SCR_DestructibleEntity: " + m_aDestructibleEntities.Count(), LogLevel.NORMAL);

		if (m_bFindDestructibleEntitiesWithMPD)
		{
			if (m_bIncludeDisabledMPDComponents)
				Print("Number of *.et files containing SCR_DestructibleEntity and SCR_DestructionMultiPhaseComponent: " + m_aDestructibleEntitiesWithMPD.Count(), LogLevel.NORMAL);
			else
				Print("Number of *.et files containing SCR_DestructibleEntity and enabled SCR_DestructionMultiPhaseComponent: " + m_aDestructibleEntitiesWithMPD.Count(), LogLevel.NORMAL);
		}

		Debug.EndTimeMeasure("Folder scan done");
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Print MPDs")]
	protected void PrintMPDComponents()
	{
		Debug.BeginTimeMeasure();

		Print("------------------------------------------------------------------------", LogLevel.NORMAL);

		foreach (ResourceName resourceName : m_aMPDComponents)
		{
			Print(resourceName.GetPath(), LogLevel.NORMAL);
		}

		Print("------------------------------------------------------------------------", LogLevel.NORMAL);

		if (m_bIncludeDisabledMPDComponents)
			Debug.EndTimeMeasure("Found " + m_aMPDComponents.Count() + " MPD Components");
		else
			Debug.EndTimeMeasure("Found " + m_aMPDComponents.Count() + " enabled MPD Components");
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Print Destr. Entities")]
	protected void PrintDestructibleEntities()
	{
		Debug.BeginTimeMeasure();

		Print("------------------------------------------------------------------------", LogLevel.NORMAL);

		foreach (ResourceName resourceName : m_aDestructibleEntities)
		{
			Print(resourceName.GetPath(), LogLevel.NORMAL);
		}

		Print("------------------------------------------------------------------------", LogLevel.NORMAL);

		Debug.EndTimeMeasure("Found " + m_aDestructibleEntities.Count() + " Destructible Entities");
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Print Destr. Entities w/ MPD")]
	protected void PrintDestructibleEntitiesWithMPD()
	{
		Debug.BeginTimeMeasure();

		Print("------------------------------------------------------------------------", LogLevel.NORMAL);

		foreach (ResourceName resourceName : m_aDestructibleEntitiesWithMPD)
		{
			Print(resourceName.GetPath(), LogLevel.NORMAL);
		}

		Print("------------------------------------------------------------------------", LogLevel.NORMAL);

		if (m_bIncludeDisabledMPDComponents)
			Debug.EndTimeMeasure("Found " + m_aDestructibleEntitiesWithMPD.Count() + " Destructible Entities with MPD Components");
		else
			Debug.EndTimeMeasure("Found " + m_aDestructibleEntitiesWithMPD.Count() + " Destructible Entities with enabled MPD Components");
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Print MPDs missing Rpl")]
	protected void PrintMPDComponentsMissingRpl()
	{
		Debug.BeginTimeMeasure();

		Print("------------------------------------------------------------------------", LogLevel.NORMAL);

		foreach (ResourceName resourceName : m_aMPDComponentsMissingRpl)
		{
			Print(resourceName.GetPath(), LogLevel.NORMAL);
		}

		Print("------------------------------------------------------------------------", LogLevel.NORMAL);

		if (m_bIncludeDisabledMPDComponents)
			Debug.EndTimeMeasure("Found " + m_aMPDComponentsMissingRpl.Count() + " MPD Components without RplComponent");
		else
			Debug.EndTimeMeasure("Found " + m_aMPDComponentsMissingRpl.Count() + " enabled MPD Components without RplComponent");
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Print MPDs missing HitZone")]
	protected void PrintMPDComponentsMissingHitZone()
	{
		Debug.BeginTimeMeasure();

		Print("------------------------------------------------------------------------", LogLevel.NORMAL);

		foreach (ResourceName resourceName : m_aMPDComponentsNoHitZone)
		{
			Print(resourceName.GetPath(), LogLevel.NORMAL);
		}

		Print("------------------------------------------------------------------------", LogLevel.NORMAL);

		if (m_bIncludeDisabledMPDComponents)
			Debug.EndTimeMeasure("Found " + m_aMPDComponentsNoHitZone.Count() + " MPD Components without any HitZone");
		else
			Debug.EndTimeMeasure("Found " + m_aMPDComponentsNoHitZone.Count() + " enabled MPD Components without any HitZone");
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Print MPDs with Defaults")]
	protected void PrintMPDComponentsWithDefaults()
	{
		Debug.BeginTimeMeasure();

		Print("------------------------------------------------------------------------", LogLevel.NORMAL);

		foreach (ResourceName resourceName : m_aMPDComponentsDefaultValues)
		{
			Print(resourceName.GetPath(), LogLevel.NORMAL);
		}

		Print("------------------------------------------------------------------------", LogLevel.NORMAL);

		if (m_bIncludeDisabledMPDComponents)
			Debug.EndTimeMeasure("Found " + m_aMPDComponentsDefaultValues.Count() + " MPD Components with default values");
		else
			Debug.EndTimeMeasure("Found " + m_aMPDComponentsDefaultValues.Count() + " enabled MPD Components with default values");
	}
}
#endif // WORKBENCH
