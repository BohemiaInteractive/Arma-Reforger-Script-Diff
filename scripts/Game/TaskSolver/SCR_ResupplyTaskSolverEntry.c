[BaseContainerProps()]
class SCR_ResupplyTaskSolverEntry : SCR_TaskSolverEntry
{
	[Attribute("0.3", UIWidgets.Slider, params: "0 1 0.05", desc: "Minimum resource fraction (0–1) needed in the truck to return to base.")]
	protected float m_fResourceValueThreshold;

	[Attribute("", UIWidgets.ResourceNamePicker, params: "et", category: "Waypoint Prefabs")]
	protected ResourceName m_sGetInWaypointPrefab;

	[Attribute("", UIWidgets.ResourceNamePicker, params: "et", category: "Waypoint Prefabs")]
	protected ResourceName m_sGetOutWaypointPrefab;

	[Attribute("", UIWidgets.ResourceNamePicker, params: "et", category: "Waypoint Prefabs")]
	protected ResourceName m_sMoveWaypointPrefab;

	[Attribute("", UIWidgets.ResourceNamePicker, params: "et", category: "Waypoint Prefabs")]
	protected ResourceName m_sLoadSuppliesWaypointPrefab;

	[Attribute("", UIWidgets.ResourceNamePicker, params: "et", category: "Waypoint Prefabs")]
	protected ResourceName m_sUnloadSuppliesWaypointPrefab;

	[Attribute(EAIGroupCombatMode.HOLD_FIRE.ToString(), UIWidgets.ComboBox, enumType: EAIGroupCombatMode)]
	protected EAIGroupCombatMode m_eGroupCombatMode;

	[Attribute()]
	protected ref array<ref SCR_ConditionCheckUIEntry> m_aConditionCheckUIEntries;

	//------------------------------------------------------------------------------------------------
	float GetResourceValueThreshold()
	{
		return m_fResourceValueThreshold;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetGetInWaypointPrefabResourceName()
	{
		return m_sGetInWaypointPrefab;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetGetOutWaypointPrefabResourceName()
	{
		return m_sGetOutWaypointPrefab;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetMoveWaypointPrefabResourceName()
	{
		return m_sMoveWaypointPrefab;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetLoadSuppliesWaypointPrefabResourceName()
	{
		return m_sLoadSuppliesWaypointPrefab;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetUnloadSuppliesWaypointPrefabResourceName()
	{
		return m_sUnloadSuppliesWaypointPrefab;
	}

	//------------------------------------------------------------------------------------------------
	EAIGroupCombatMode GetGroupCombatMode()
	{
		return m_eGroupCombatMode;
	}

	//------------------------------------------------------------------------------------------------
	array<ref SCR_ConditionCheckUIEntry> GetConditionCheckUIEntries()
	{
		return m_aConditionCheckUIEntries;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ConditionCheckUIEntry GetTaskSolverEntry(typename conditionCheckTypename)
	{
		foreach (SCR_ConditionCheckUIEntry conditionCheckUIEntry : m_aConditionCheckUIEntries)
		{
			if (conditionCheckUIEntry.Type() == conditionCheckTypename)
				return conditionCheckUIEntry;
		}

		return null;
	}
}
