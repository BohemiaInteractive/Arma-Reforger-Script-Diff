class SCR_PrefabStructureFillingPlugin_Car : SCR_PrefabStructureFillingPlugin_TypeBase
{
	[Attribute(defvalue: "0", desc: "Number Of Lights Wanted", params: "0 inf")]
	int m_iDesiredTotalNumberOfLights;

	//------------------------------------------------------------------------------------------------
	override bool Process(notnull IEntitySource prefab)
	{
		if (!super.Process(prefab))
			return false;

		if (!CreateLights(prefab, m_iDesiredTotalNumberOfLights))
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] prefab
	//! \param[in] desiredTotalNumberOfLights
	//! \return
	protected bool CreateLights(notnull IEntitySource prefab, int desiredTotalNumberOfLights)
	{
		if (desiredTotalNumberOfLights < 1)
		{
			Print("No lights requested", LogLevel.NORMAL);
			return true;
		}

		int componentIndex = SCR_BaseContainerTools.FindComponentIndex(prefab, BaseLightManagerComponent);
		if (componentIndex < 0)
		{
			Print("No " + BaseLightManagerComponent + " on Prefab", LogLevel.WARNING);
			return false;
		}

		IEntityComponentSource componentSource = prefab.GetComponent(componentIndex);
		if (!componentSource)
		{
			Print("[SCR_PrefabStructureFillingPlugin_Car.CreateLights] wat (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.FATAL);
			return false;
		}

		BaseContainerList lightSlots = componentSource.GetObjectArray("LightSlots");
		if (!lightSlots)
		{
			Print("[SCR_PrefabStructureFillingPlugin_Car.CreateLights] wat (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.FATAL);
			return false;
		}

		int lightSlotsCount = lightSlots.Count();
		if (desiredTotalNumberOfLights <= lightSlotsCount)
		{
			PrintFormat("Requested %1 lights, %2 existing", desiredTotalNumberOfLights, lightSlotsCount, level: LogLevel.NORMAL);
			return true;
		}

		array<ref ContainerIdPathEntry> path = { new ContainerIdPathEntry("components", componentIndex) };
		bool isEditing = m_WorldEditorAPI.IsDoingEditAction();
		if (!isEditing)
			m_WorldEditorAPI.BeginEntityAction();

		for (int i = lightSlotsCount; i < desiredTotalNumberOfLights; ++i)
		{
			if (!m_WorldEditorAPI.CreateObjectArrayVariableMember(prefab, path, "LightSlots", "SCR_LightSlot", i))
			{
				Print("Error creating SCR_LightSlot", LogLevel.ERROR);
				return false;
			}
		}

		if (!isEditing)
			m_WorldEditorAPI.EndEntityAction();

		Print("Successfully created " + (desiredTotalNumberOfLights - lightSlotsCount) + " lights", LogLevel.NORMAL);
		return true;
	}
}
