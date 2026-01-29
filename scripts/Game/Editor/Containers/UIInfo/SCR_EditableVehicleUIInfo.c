[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_EditableVehicleUIInfo: SCR_EditableEntityUIInfo
{	
	[Attribute("1", desc: "If true when placing the entity in editor this will make sure that both crew and passengers are spawned as one group. Else passengers and crew are split in two groups")]
	protected bool m_bEditorPlaceAsOneGroup;
	
	[Attribute(desc: "Auto filled by plugin. Compartments Types that can be filled with characters within when using the editor to place the vehicle.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECompartmentType))]
	protected ref array<ECompartmentType> m_aOccupantFillCompartmentTypes;
	
	[Attribute("", category: "Editable Entity", desc: "Auto filled by plugin. Contains budget costs of default prefabs for pilot/crew slots")]
	private ref array<ref SCR_EntityBudgetValue> m_aCrewEntityBudgetCost;
	
	[Attribute("", category: "Editable Entity", desc: "Auto filled by plugin. Contains budget costs of default prefabs for passenger slots")]
	private ref array<ref SCR_EntityBudgetValue> m_aPassengerEntityBudgetCost;

	//------------------------------------------------------------------------------------------------
	//! Get bool if spawned characters should be placed as one if placing the vehicle with crew and passenger flags
	//! \return If editor should place as one group or not
	bool GetEditorPlaceAsOneGroup()
	{
		return m_bEditorPlaceAsOneGroup;
	}

	//------------------------------------------------------------------------------------------------
	//! Get an array of compart ment types this vehicle can be filled with
	//! \param[out] array of compartment types
	//! \return count of array
	int GetOccupantFillCompartmentTypes(out notnull array<ECompartmentType> compartmentTypes)
	{
		compartmentTypes.Clear();
		compartmentTypes.Copy(m_aOccupantFillCompartmentTypes);
		
		return compartmentTypes.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if editor is allowed to fill vehicle with given Crew
	//! \return Returns true if can fill with Crew
	bool CanFillWithCrew()
	{
		return CanFillWithGivenTypes(SCR_BaseCompartmentManagerComponent.CREW_COMPARTMENT_TYPES);
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if editor is allowed to fill vehicle with given Passengers
	//! \return Returns true if can fill with Passengers
	bool CanFillWithPassengers()
	{
		return CanFillWithGivenTypes(SCR_BaseCompartmentManagerComponent.PASSENGER_COMPARTMENT_TYPES);
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if editor is allowed to fill vehicle with given occupant types
	//! \param typesToCheck given Occupant types
	//! \return Returns true if at least one of the given occupent types is true
	bool CanFillWithGivenTypes(array<ECompartmentType> typesToCheck)
	{
		if (m_aOccupantFillCompartmentTypes.IsEmpty() || !typesToCheck || typesToCheck.IsEmpty())
			return false;
		
		foreach (ECompartmentType checkType: typesToCheck)
		{
			foreach (ECompartmentType type: m_aOccupantFillCompartmentTypes)
			{
				if (type == checkType)
					return true;
			}
		}
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns an array of of budgets for crew
	//! \param[out] budgetCosts Crew budget costs
	void GetFillBudgetCostsOfCrew(out notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{
		budgetCosts.Clear();
		
		if (!CanFillWithCrew())
			return;
		
		foreach(SCR_EntityBudgetValue budget: m_aCrewEntityBudgetCost)
			budgetCosts.Insert(budget);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns an array of of budgets for crew
	//! \param[out] budgetCosts Crew budget costs
	void GetFillBudgetCostsOfPassengers(out notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{
		budgetCosts.Clear();
		
		if (!CanFillWithPassengers())
			return;
		
		foreach(SCR_EntityBudgetValue budget: m_aPassengerEntityBudgetCost)
			budgetCosts.Insert(budget);
	}
}
