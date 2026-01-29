class SCR_AnalyticsDataCollectionHelper
{
	//------------------------------------------------------------------------------------------------
	//! To get the name of prefab of entity without the whole path in ResourceName
	//! \param[in] entity
	//! \return string entity prefab name
	static string GetEntityPrefabName(notnull IEntity entity)
	{
		string entityName;

		EntityPrefabData entityPrefabData = entity.GetPrefabData();
		if (!entityPrefabData)
			return entityName;

		entityName = entityPrefabData.GetPrefab().GetName();
		
		return GetShortEntityPrefabName(entityName);
	}
	
	//------------------------------------------------------------------------------------------------
	//! To calculate the supply cost of requested entity
	//! Evaluates only the cost from CAMPAIGN budget
	//! \param[in] entity
	//! \return int supply cost
	static int GetEntitySupplyCost(IEntity entity)
	{
		SCR_EditableEntityComponent editableEntityComponent = SCR_EditableEntityComponent.GetEditableEntity(entity);
		if (!editableEntityComponent)
			return 0;

		array<ref SCR_EntityBudgetValue> budgets = {};
		editableEntityComponent.GetEntityBudgetCost(budgets, entity);

		foreach (SCR_EntityBudgetValue budget : budgets)
		{
			// We only care about Campaign budget
			if (budget.GetBudgetType() == EEditableEntityBudget.CAMPAIGN)
				return budget.GetBudgetValue();
		}

		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! To get shortened prefab name without the whole path
	//! \param[in] entityPrefabName
	//! \return string short entity prefab name
	static string GetShortEntityPrefabName(string entityPrefabName)
	{
		array<string> pieces = {};
		entityPrefabName.Split("/", pieces, true);

		string shortName;
		if (pieces.Count() > 0)
			shortName = pieces[pieces.Count() - 1];

		return shortName;
	}

	//------------------------------------------------------------------------------------------------
	//! To get shortened string made from data array
	//! \param[in] dataArray
	//! \return string short dataArray
	static string GetShortDataArray(string dataArray)
	{
		int firstWhiteSpaceIndex = dataArray.IndexOf(" ");
		if (firstWhiteSpaceIndex < 1)
			return string.Empty;

		return dataArray.Substring(firstWhiteSpaceIndex +1, dataArray.Length() - 1 - firstWhiteSpaceIndex);
	}

	//------------------------------------------------------------------------------------------------
	//! To get shortened name of Analytics Data Collection Module
	//! \param[in] moduleName
	//! \return string short module name
	static string GetShortModuleName(string moduleName)
	{
		array<string> pieces = {};
		moduleName.Split("<", pieces, true);

		string shortName;

		if (pieces.Count() > 0)
			shortName = pieces[0];

		shortName.Split("_", pieces, true);

		if (pieces.Count() > 0)
			shortName = pieces[pieces.Count() - 1];

		return shortName;
	}
}