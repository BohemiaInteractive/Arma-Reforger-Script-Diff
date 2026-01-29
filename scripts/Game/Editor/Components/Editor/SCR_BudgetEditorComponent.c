[ComponentEditorProps(category: "GameScripted/Editor", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_BudgetEditorComponentClass : SCR_BaseEditorComponentClass
{
}

void ScriptInvoker_EntityBudgetMaxUpdated(EEditableEntityBudget budgetType, int currentBudgetValue, int newMaxBudgetValue);
typedef func ScriptInvoker_EntityBudgetMaxUpdated;
typedef ScriptInvokerBase<ScriptInvoker_EntityBudgetMaxUpdated> ScriptInvoker_EntityBudgetMaxUpdatedEvent;

void ScriptInvoker_EntityBudgetUpdated(EEditableEntityBudget budgetType, int originalBudgetValue, int updatedBudgetValue, int maxBudgetValue);
typedef func ScriptInvoker_EntityBudgetUpdated;
typedef ScriptInvokerBase<ScriptInvoker_EntityBudgetUpdated> ScriptInvoker_EntityBudgetUpdatedEvent;

void ScriptInvoker_EntityBudgetMaxReached(EEditableEntityBudget budgetType, bool maxReached);
typedef func ScriptInvoker_EntityBudgetMaxReached;
typedef ScriptInvokerBase<ScriptInvoker_EntityBudgetMaxReached> ScriptInvoker_EntityBudgetMaxReachedEvent;

// if enabled we will perform double checks to make sure nothing broke
#define BUDGET_OPTIMIZATION_CHECKS

class SCR_BudgetEditorComponent : SCR_BaseEditorComponent
{
	const int DEFAULT_MAX_BUDGET = 100;
	const int DEFAULT_MIN_COST = 1;
	
	[Attribute(desc: "Define maximum budget values for player")]
	protected ref array<ref SCR_EntityBudgetValue> m_MaxBudgets;
	
	//contains all the budgets, even those not defined as an attribute
	protected ref map<EEditableEntityBudget, ref SCR_EntityBudgetValue> m_maxBudgetsInternal = new map<EEditableEntityBudget, ref SCR_EntityBudgetValue>;
	
	protected SCR_EditableEntityCore m_EntityCore;
	protected SCR_BaseEditableEntityFilter m_DestroyedEntityFilter;
	
	protected ref map<EEditableEntityBudget, SCR_EditableEntityCoreBudgetSetting> m_BudgetSettingsMap = new map<EEditableEntityBudget, SCR_EditableEntityCoreBudgetSetting>;
	
	ref ScriptInvoker_EntityBudgetUpdatedEvent Event_OnBudgetUpdated = new ScriptInvoker_EntityBudgetUpdatedEvent();
	ref ScriptInvoker_EntityBudgetMaxReachedEvent Event_OnBudgetMaxReached = new ScriptInvoker_EntityBudgetMaxReachedEvent();
	ref ScriptInvoker_EntityBudgetMaxUpdatedEvent Event_OnBudgetMaxUpdated = new ScriptInvoker_EntityBudgetMaxUpdatedEvent();
	ref ScriptInvoker Event_OnBudgetPreviewUpdated = new ScriptInvoker();
	ref ScriptInvoker Event_OnBudgetPreviewReset = new ScriptInvoker();
	
	//Delayed set max values
	protected ref map<EEditableEntityBudget, int> m_DelayedSetMaxBudgetsMap = new map<EEditableEntityBudget, int>();
	protected bool m_bListenToMaxBudgetDelay = false;

	//------------------------------------------------------------------------------------------------
	//! Checks if budget is sufficient to place passed entity
	//! \param[in] info
	//! \param[out] blockingBudget
	//! \param[in] showNotification
	//! \return
	bool CanPlaceEntityInfo(SCR_EditableEntityUIInfo info, out EEditableEntityBudget blockingBudget, bool showNotification)
	{
		array<ref SCR_EntityBudgetValue> budgetCosts = {};
		return CanPlaceEntityInfo(info, budgetCosts, blockingBudget, showNotification);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks if budget is sufficient to place passed entity
	//! \param[in] info
	//! \param[out] budgetCosts
	//! \param[out] blockingBudget
	//! \param[in] showNotification
	//! \return
	bool CanPlaceEntityInfo(SCR_EditableEntityUIInfo info, out notnull array<ref SCR_EntityBudgetValue> budgetCosts, out EEditableEntityBudget blockingBudget, bool showNotification)
	{
		if (!IsBudgetCapEnabled())
			return true;

		bool canPlace = false;
		if (GetEntityPreviewBudgetCosts(info, budgetCosts))
			canPlace = CanPlace(budgetCosts, blockingBudget);
		else
			canPlace = CanPlaceEntityType(info.GetEntityType(), budgetCosts, blockingBudget);

		if (showNotification)
			CanPlaceResult(canPlace, showNotification);

		return canPlace;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks if budget is sufficient to place passed EditableEntityComponentClass source
	//! \param[in] editableEntitySource IEntityComponentSource SCR_EditableEntityComponentClass to check budget cost of prefab
	//! \param[out] blockingBudget
	//! \param[in] isPlacingPlayer
	//! \param[in] updatePreview
	//! \param[in] showNotification
	//! \return
	bool CanPlaceEntitySource(IEntityComponentSource editableEntitySource, out EEditableEntityBudget blockingBudget, bool isPlacingPlayer = false, bool updatePreview = true, bool showNotification = true)
	{
		bool canPlace = true;
		if (editableEntitySource && IsBudgetCapEnabled())
		{
			array<ref SCR_EntityBudgetValue> budgetCosts = {};
			if (!GetEntitySourcePreviewBudgetCosts(editableEntitySource, budgetCosts))
			{
				canPlace = false;
			}
			// Clear budget cost when placing as player
			if (isPlacingPlayer)
				budgetCosts.Clear();
			
			if (updatePreview)
				UpdatePreviewCost(budgetCosts);
			
			canPlace = canPlace && CanPlace(budgetCosts, blockingBudget);
		}
		return CanPlaceResult(canPlace, showNotification);
	}
	
	void GetBudgetCosts(IEntityComponentSource editableEntitySource, out notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{		
		GetEntitySourcePreviewBudgetCosts(editableEntitySource, budgetCosts);
	}
	
	void GetBudgetCostsDontDiscardCampaignBudget(IEntityComponentSource editableEntitySource, out notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{		
		if (!editableEntitySource)
			return;
		
 		SCR_EditableEntityUIInfo editableUIInfo = SCR_EditableEntityComponentClass.GetInfo(editableEntitySource);
		if (editableUIInfo)
			GetEntityPreviewBudgetCostsDontDiscardCampaignBudget(editableUIInfo, budgetCosts);

		return;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fallback function in case no budget costs are defined on the entity
	//! \param[in] entityType EEditableEntityType of the entity
	//! \param[out] budgetCosts
	//! \param[out] blockingBudget
	//! \return true if passed array is not empty and entity can be placed according to the passed values
	protected bool CanPlaceEntityType(EEditableEntityType entityType, out notnull array<ref SCR_EntityBudgetValue> budgetCosts, out EEditableEntityBudget blockingBudget)
	{
		if (!IsBudgetCapEnabled())
			return true;

		GetEntityTypeBudgetCost(entityType, budgetCosts);		
		return CanPlace(budgetCosts, blockingBudget);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks if budget is sufficient using passed budget costs
	//! \param[in] budgetCosts array with budget costs values either read from entity in world or directly from prefab
	//! \param[out] blockingBudget
	//! \return True if passed budgetCosts array is not empty and entity can be placed according to the passed values
	bool CanPlace(notnull array<ref SCR_EntityBudgetValue> budgetCosts, out EEditableEntityBudget blockingBudget)
	{
		if (!IsBudgetCapEnabled() || budgetCosts.IsEmpty())
			return true;
		
		foreach (SCR_EntityBudgetValue budgetCost : budgetCosts)
		{
			if (!CanPlace(budgetCost, blockingBudget))
				return false;
		}
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CanPlace(SCR_EntityBudgetValue budgetCost, out EEditableEntityBudget blockingBudget)
	{
		EEditableEntityBudget budgetType = budgetCost.GetBudgetType();
		int maxBudgetValue;
		if (!GetMaxBudgetValue(budgetType, maxBudgetValue))
			return false;
		
		int originalBudgetValue = GetCurrentBudgetValue(budgetType);
		int budgetChange = budgetCost.GetBudgetValue();
		if (!CanPlace(originalBudgetValue, budgetChange, maxBudgetValue))
		{
			blockingBudget = budgetType;
			return false;
		}
		else
		{
			return true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CanPlace(int currentBudget, int budgetChange, int maxBudget)
	{
		return currentBudget + budgetChange <= maxBudget;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] entityUIInfo
	//! \param[out] budgetCosts
	//! \return
	bool GetEntityPreviewBudgetCosts(SCR_EditableEntityUIInfo entityUIInfo, out notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{
		if (!entityUIInfo)
			return false;
		
		if (!entityUIInfo.GetEntityBudgetCost(budgetCosts))
			GetEntityTypeBudgetCost(entityUIInfo.GetEntityType(), budgetCosts);
		
		array<ref SCR_EntityBudgetValue> entityChildrenBudgetCosts = {};
		entityUIInfo.GetEntityChildrenBudgetCost(entityChildrenBudgetCosts);
		
		SCR_EntityBudgetValue.MergeBudgetCosts(budgetCosts, entityChildrenBudgetCosts);
		
		FilterAvailableBudgets(budgetCosts);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] entityUIInfo
	//! \param[out] budgetCosts
	//! \return
	bool GetEntityPreviewBudgetCostsDontDiscardCampaignBudget(SCR_EditableEntityUIInfo entityUIInfo, out notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{
		if (!entityUIInfo)
			return false;
		
		if (!entityUIInfo.GetEntityBudgetCost(budgetCosts))
			GetEntityTypeBudgetCost(entityUIInfo.GetEntityType(), budgetCosts);
		
		array<ref SCR_EntityBudgetValue> entityChildrenBudgetCosts = {};
		entityUIInfo.GetEntityChildrenBudgetCost(entityChildrenBudgetCosts);
		
		SCR_EntityBudgetValue.MergeBudgetCosts(budgetCosts, entityChildrenBudgetCosts);
		
		FilterAvailableBudgetsDontDiscardCampaignBudget(budgetCosts);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] editableEntitySource
	//! \param[out] budgetCosts
	//! \return
	bool GetEntitySourcePreviewBudgetCosts(IEntityComponentSource editableEntitySource, out notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{
		if (!editableEntitySource)
			return false;
		
 		SCR_EditableEntityUIInfo editableUIInfo = SCR_EditableEntityComponentClass.GetInfo(editableEntitySource);
		if (editableUIInfo)
			return GetEntityPreviewBudgetCosts(editableUIInfo, budgetCosts);

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return the cost of the occupants for a vehicle. It takes the cost directly from the UIinfo
	//! \param[in] editableEntitySource The Vehicle Entity source to get budget from
	//! \param[in] placingFlags To check which costs it needs to grab (Passengers and/or Vrew)
	//! \param[out] budgetCosts Array of total budget costs
	//! \param[in] includeVehicleCost If true will also return the vehicle budget cost
	//! \return False if unsuccesfull in obtaining costs
	bool GetVehicleOccupiedBudgetCosts(IEntityComponentSource editableEntitySource, EEditorPlacingFlags placingFlags, out notnull array<ref SCR_EntityBudgetValue> budgetCosts, bool includeVehicleCost = true)
	{
		if (includeVehicleCost && !GetEntitySourcePreviewBudgetCosts(editableEntitySource, budgetCosts))
			return false;
		
		SCR_EditableVehicleUIInfo editableUIInfo = SCR_EditableVehicleUIInfo.Cast(SCR_EditableEntityComponentClass.GetInfo(editableEntitySource));
		return GetVehicleOccupiedBudgetCosts(editableUIInfo, placingFlags, budgetCosts);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] editableUIInfo
	//! \param[in] placingFlags
	//! \param[out] budgetCosts
	//! \return
	bool GetVehicleOccupiedBudgetCosts(SCR_EditableVehicleUIInfo editableUIInfo, EEditorPlacingFlags placingFlags, out notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{
		if (!editableUIInfo)
			return false;
		
		array<ref SCR_EntityBudgetValue> vehicleOccupantsBudgetCosts = {};
		
		if (placingFlags & EEditorPlacingFlags.VEHICLE_CREWED)
		{
			editableUIInfo.GetFillBudgetCostsOfCrew(vehicleOccupantsBudgetCosts);
			SCR_EntityBudgetValue.MergeBudgetCosts(budgetCosts, vehicleOccupantsBudgetCosts);
		}

		if (placingFlags & EEditorPlacingFlags.VEHICLE_PASSENGER)
		{
			editableUIInfo.GetFillBudgetCostsOfPassengers(vehicleOccupantsBudgetCosts);
			SCR_EntityBudgetValue.MergeBudgetCosts(budgetCosts, vehicleOccupantsBudgetCosts);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] budgetCosts
	void UpdatePreviewCost(notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{
		ResetPreviewCost();
		foreach (SCR_EntityBudgetValue budgetCost : budgetCosts)
		{
			EEditableEntityBudget budgetType = budgetCost.GetBudgetType();
			int budgetValue = budgetCost.GetBudgetValue();
			int maxBudget;
			if (!GetMaxBudgetValue(budgetType, maxBudget))
				continue;

			float max = maxBudget;
			// Calculate current budget percentage
			int currentBudget = GetCurrentBudgetValue(budgetType);
			float newBudgetPercent;
			float budgetChange;
			if (max != 0)
			{
				// Calculate budget percentage after entity is placed
				float currentBudgetPercent = currentBudget / max * 100;
				int newBudget = currentBudget + budgetValue;
				
				newBudgetPercent = newBudget / max * 100;
				// How much the budget percentage is increased
				budgetChange = newBudgetPercent - currentBudgetPercent;
			}
			else
			{
				newBudgetPercent = 100;
				budgetChange = 0; // Show no change when maxbudget is 0
			}
			
			Event_OnBudgetPreviewUpdated.Invoke(budgetType, newBudgetPercent, budgetChange);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ResetPreviewCost()
	{
		Event_OnBudgetPreviewReset.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsBudgetAvailable(EEditableEntityBudget type)
	{
		SCR_EntityBudgetValue budget;
		return GetMaxBudget(type, budget);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetMaxBudget(EEditableEntityBudget type, out SCR_EntityBudgetValue budget)
	{
		#ifdef BUDGET_OPTIMIZATION_CHECKS
		const bool result = m_maxBudgetsInternal.Find(type, budget);
	
		if(result && !budget)
		{
			Print("GetMaxBudget: Budget type wasn't defined!", LogLevel.ERROR);
			return GetMaxBudget_Old(type, budget);
		}
		#endif
		
		return m_maxBudgetsInternal.Find(type, budget);
	}
	
	
	//old version of GetMaxBudget
	//------------------------------------------------------------------------------------------------
	protected bool GetMaxBudget_Old(EEditableEntityBudget type, out SCR_EntityBudgetValue budget)
	{	
		//iterate like we used to do
		foreach	(SCR_EntityBudgetValue maxBudget : m_MaxBudgets)
		{
			if(!maxBudget)
				continue;
			
			if (maxBudget.GetBudgetType() == type)
			{
				budget = maxBudget;
				return true;
			}
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get max budget for this player per budget type
	//! \param[in] type
	//! \param[out] maxBudget
	//! \return
	bool GetMaxBudgetValue(EEditableEntityBudget type, out int maxBudget)
	{
		SCR_EntityBudgetValue budgetValue;
		if (GetMaxBudget(type, budgetValue))
		{
			maxBudget = budgetValue.GetBudgetValue();
			return true;
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set max budget for this player for budget type
	//! \param[in] type
	//! \param[in] newValue
	void SetMaxBudgetValue(EEditableEntityBudget type, int newValue)
	{
		int oldValue = 0;
		SCR_EntityBudgetValue maxBudget;
		if (GetMaxBudget(type, maxBudget))
		{
			oldValue = maxBudget.GetBudgetValue();
			maxBudget.SetBudgetValue(newValue);
		}
		Rpc(UpdateMaxBudgetOwner, type, oldValue, newValue);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] type
	//! \param[in] newValue
	//! \param[in] playerChangingBudget
	void DelayedSetMaxBudgetSetup(EEditableEntityBudget type, int newValue, int playerChangingBudget)
	{
		m_DelayedSetMaxBudgetsMap.Insert(type, newValue);
		
		if (!m_bListenToMaxBudgetDelay)
		{
			m_bListenToMaxBudgetDelay = true;
			GetGame().GetCallqueue().CallLater(DelayedSetMaxBudget, 100, false, playerChangingBudget);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedSetMaxBudget(int playerChangingBudget)
	{
		m_bListenToMaxBudgetDelay = false;
		SetMultiMaxBudgetValues(m_DelayedSetMaxBudgetsMap, playerChangingBudget);
		m_DelayedSetMaxBudgetsMap = new map<EEditableEntityBudget, int>();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] budgets
	//! \param[in] playerChangingBudget unused
	void SetMultiMaxBudgetValues(notnull map<EEditableEntityBudget, int> budgets, int playerChangingBudget)
	{
		foreach (EEditableEntityBudget type, int budget: budgets)
   	 	{
       		SetMaxBudgetValue(type, budget);
   		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the minimun cost of the given entity type
	//! \param[in] entityType entity type
	//! \param[out] budgetCosts The minimum cost for the given budget type
	void GetEntityTypeBudgetCost(EEditableEntityType entityType, out array<ref SCR_EntityBudgetValue> budgetCosts)
	{
		EEditableEntityBudget entityBudgetType = m_EntityCore.GetBudgetForEntityType(entityType);
		int minBudgetCost = GetEntityTypeBudgetCost(entityBudgetType);
		
		budgetCosts = { new SCR_EntityBudgetValue(entityBudgetType, minBudgetCost) };
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetEntityTypeBudgetCost(EEditableEntityBudget budgetType)
	{
		SCR_EditableEntityCoreBudgetSetting budgetSettings = m_BudgetSettingsMap.Get(budgetType);
		if (budgetSettings)
			return budgetSettings.GetMinBudgetCost();
		
		return DEFAULT_MIN_COST;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get last received budget value for type
	//! \param[in] type
	//! \return
	int GetCurrentBudgetValue(EEditableEntityBudget type)
	{
		SCR_EditableEntityCoreBudgetSetting budgetSettings;
		if (!GetCurrentBudgetSettings(type, budgetSettings))
			return 0;

		return budgetSettings.GetCurrentBudget() + budgetSettings.GetReservedBudget();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set current budget value
	//! \param[in] budgetType
	//! \param[in] value
	void SetCurrentBudgetValue(EEditableEntityBudget budgetType, int value)
	{
		int originalBudgetValue, budgetChange, maxBudgetValue;
		SCR_EditableEntityCoreBudgetSetting budgetSettings;
		if (GetCurrentBudgetSettings(budgetType, budgetSettings) && GetMaxBudgetValue(budgetType, maxBudgetValue))
		{
			originalBudgetValue = budgetSettings.GetCurrentBudget();
			budgetChange = budgetSettings.SetCurrentBudget(value);
			
			if (budgetChange != value - originalBudgetValue)
				budgetChange = 0;
			
			bool budgetMaxReached;
			bool sendBudgetMaxEvent = CheckMaxBudgetReached(budgetType, budgetChange, originalBudgetValue, value, maxBudgetValue, budgetMaxReached);
			
			Rpc(OnEntityCoreBudgetUpdatedOwner, budgetType, value, budgetChange, sendBudgetMaxEvent, budgetMaxReached);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get current settings of budget with given type
	//! \param[in] budgetType
	//! \param[out] blockingBudgetInfo
	//! \return
	bool GetCurrentBudgetInfo(EEditableEntityBudget budgetType, out SCR_UIInfo blockingBudgetInfo)
	{
		SCR_EditableEntityCoreBudgetSetting budgetSettings;
		if (GetCurrentBudgetSettings(budgetType, budgetSettings))
		{
			blockingBudgetInfo = budgetSettings.GetInfo();
			return true;
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] budgets
	void GetBudgets(out notnull array<ref SCR_EditableEntityCoreBudgetSetting> budgets)
	{
		foreach (EEditableEntityBudget budgetType, SCR_EditableEntityCoreBudgetSetting setting : m_BudgetSettingsMap)
		{
			budgets.Insert(setting);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetCurrentBudgetSettings(EEditableEntityBudget budgetType, out SCR_EditableEntityCoreBudgetSetting budgetSettings)
	{
		return m_BudgetSettingsMap.Find(budgetType, budgetSettings);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get default budget definitions from core, budget values not synced on client, use GetCurrentBudget for updated budget
	protected void RefreshBudgetSettings()
	{
		m_BudgetSettingsMap = new map<EEditableEntityBudget, SCR_EditableEntityCoreBudgetSetting>;
		
		if (!m_EntityCore)
			return;

		array<ref SCR_EditableEntityCoreBudgetSetting> outBudgets = {};
		m_EntityCore.GetBudgets(outBudgets);
		
		SCR_EditableEntityCoreBudgetSetting budget;
		for (int i = outBudgets.Count() - 1; i >= 0; i--)
		{
			budget = outBudgets[i];
			EEditableEntityBudget budgetType = budget.GetBudgetType();
			int maxBudget;
			if (GetMaxBudgetValue(budgetType, maxBudget))
			{
				m_BudgetSettingsMap.Insert(budgetType, budget);
				int currentBudgetValue = budget.GetCurrentBudget();
				Event_OnBudgetUpdated.Invoke(budgetType, currentBudgetValue, currentBudgetValue, maxBudget);
				Event_OnBudgetMaxUpdated.Invoke(budgetType, currentBudgetValue, maxBudget);
			}
			else
			{
				outBudgets.RemoveOrdered(i);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void UpdateMaxBudgetOwner(int budgetType, int oldMaxBudget, int newMaxBudget)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_BUDGET_CHANGES))
			Print(string.Format("Updated maximum budget received for type %1: %2", budgetType, newMaxBudget), LogLevel.NORMAL);
		
		SCR_EntityBudgetValue maxBudget;
		if (GetMaxBudget(budgetType, maxBudget))
			maxBudget.SetBudgetValue(newMaxBudget);
		
		int currentBudget = GetCurrentBudgetValue(budgetType);
		// Notify max budget reached
		if (oldMaxBudget == 0 || (currentBudget > oldMaxBudget && currentBudget < newMaxBudget)) // current budget above old budget, below new max budget
			Event_OnBudgetMaxReached.Invoke(budgetType, false);
		else if (newMaxBudget == 0 || (currentBudget < oldMaxBudget && currentBudget > newMaxBudget)) // current budget below old budget, above new max budget
			Event_OnBudgetMaxReached.Invoke(budgetType, true);
		
		Event_OnBudgetMaxUpdated.Invoke(budgetType, currentBudget, newMaxBudget);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CanPlaceResult(bool canPlace, bool showNotification)
	{
		if (showNotification)
			Rpc(CanPlaceOwner, canPlace);

		return canPlace;
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void CanPlaceOwner(bool canPlace)
	{
		if (!canPlace)
		{
			GetManager().SendNotification(ENotification.EDITOR_PLACING_BUDGET_MAX);
			//Event_OnBudgetMaxReached.Invoke(budgetType, maxBudgetReached);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEntityCoreBudgetUpdated(EEditableEntityBudget entityBudget, int originalBudgetValue, int budgetChange, int updatedBudgetValue)
	{
		int maxBudgetValue;
		if (!GetMaxBudgetValue(entityBudget, maxBudgetValue))
			return;	
		bool budgetMaxReached;
		bool sendBudgetMaxEvent = CheckMaxBudgetReached(entityBudget, budgetChange, originalBudgetValue, updatedBudgetValue, maxBudgetValue, budgetMaxReached);
		/*if (!IsOwner())*/
			Rpc(OnEntityCoreBudgetUpdatedOwner, entityBudget, updatedBudgetValue, budgetChange, sendBudgetMaxEvent, budgetMaxReached);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void OnEntityCoreBudgetUpdatedOwner(EEditableEntityBudget entityBudget, int budgetValue, int budgetChange, bool sendBudgetMaxEvent, bool budgetMaxReached)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_BUDGET_CHANGES))
			Print(string.Format("Updated budget received for type %1: %2", entityBudget, budgetValue), LogLevel.NORMAL);
		
		int maxBudget;
		SCR_EditableEntityCoreBudgetSetting budgetSettings;
		if (GetCurrentBudgetSettings(entityBudget, budgetSettings) && GetMaxBudgetValue(entityBudget, maxBudget))
		{
			int currentBudget = budgetSettings.GetCurrentBudget();
			int ownerBudgetChange = budgetSettings.SetCurrentBudget(budgetValue);
			
			Event_OnBudgetUpdated.Invoke(entityBudget, currentBudget, budgetValue, maxBudget);
			
			if (sendBudgetMaxEvent)
				Event_OnBudgetMaxReached.Invoke(entityBudget, budgetMaxReached);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CheckMaxBudgetReached(EEditableEntityBudget entityBudget, int budgetChange, int originalBudgetValue, int updatedBudgetValue, int maxBudgetValue, out bool maxBudgetReached)
	{
		if (budgetChange >= 0 && originalBudgetValue + budgetChange >= maxBudgetValue)
		{ 
			// Send budget max reached true event
			maxBudgetReached = true;
			return true;
		}
		else if (budgetChange < 0 && updatedBudgetValue < maxBudgetValue)
		{
			// Send when budget is lowered between 100-90%, budget max reached false event
			if (originalBudgetValue >= maxBudgetValue * 0.9)
			{
				maxBudgetReached = false;
				return true;
			}
		}
		// Do not send max budget reached event
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FilterAvailableBudgets(inout notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{
		SCR_EntityBudgetValue budget;
		for (int i = budgetCosts.Count() - 1; i >= 0; i--)
		{
			budget = budgetCosts[i];
			if (!IsBudgetAvailable(budget.GetBudgetType()))
				budgetCosts.Remove(i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FilterAvailableBudgetsDontDiscardCampaignBudget(inout notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{
		SCR_EntityBudgetValue budget;
		for (int i = budgetCosts.Count() - 1; i >= 0; i--)
		{
			budget = budgetCosts[i];
			
			if(budget.GetBudgetType() == EEditableEntityBudget.CAMPAIGN)
				continue;
			
			if (!IsBudgetAvailable(budget.GetBudgetType()))
				budgetCosts.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] budgetType
	//! \return
	SCR_EditableEntityCoreBudgetSetting GetBudgetSetting(EEditableEntityBudget budgetType)
	{
		return m_BudgetSettingsMap.Get(budgetType);
	}

	//------------------------------------------------------------------------------------------------
	protected EEditableEntityBudget GetFirstAvailableBudget()
	{
		if (!m_MaxBudgets || m_MaxBudgets.IsEmpty())
			return null;
		else
			return m_MaxBudgets[0].GetBudgetType();
	}

	//------------------------------------------------------------------------------------------------
	//!
	void DemandBudgetUpdateFromServer()
	{
		if (!m_RplComponent)
			return;

		if (m_RplComponent.Role() != RplRole.Proxy)
			return;

		Rpc(RpcServer_UpdateBudget);
	}

	//------------------------------------------------------------------------------------------------
	//!
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcServer_UpdateBudget()
	{
		array<ref SCR_EditableEntityCoreBudgetSetting> outBudgets = {};
		m_EntityCore.GetBudgets(outBudgets);

		foreach (SCR_EditableEntityCoreBudgetSetting budgetSetting : outBudgets)
		{
			Rpc(RpcOwner_UpdateBudget, budgetSetting.GetBudgetType(), budgetSetting.GetCurrentBudget());
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] budgetType
	//! \param[in] currentBudget
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcOwner_UpdateBudget(EEditableEntityBudget budgetType, int currentBudget)
	{
		SCR_EditableEntityCoreBudgetSetting budget;
		if (!m_BudgetSettingsMap.Find(budgetType, budget))
			return;

		budget.SetCurrentBudget(currentBudget);

		int maxBudget;
		if (GetMaxBudgetValue(budgetType, maxBudget))
		{
			m_BudgetSettingsMap.Insert(budgetType, budget);
			int currentBudgetValue = budget.GetCurrentBudget();
			Event_OnBudgetUpdated.Invoke(budgetType, currentBudgetValue, currentBudgetValue, maxBudget);
			Event_OnBudgetMaxUpdated.Invoke(budgetType, currentBudgetValue, maxBudget);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsBudgetCapEnabled()
	{
	#ifdef ENABLE_DIAG
		return DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_BUDGET_CAP);
	#else
		return true;
	#endif
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorDebug(array<string> debugTexts)
	{
		SCR_EntityBudgetValue maxBudget;
		foreach (EEditableEntityBudget type, SCR_EditableEntityCoreBudgetSetting budgetSettings : m_BudgetSettingsMap)
		{
			GetMaxBudget(type, maxBudget);
			debugTexts.Insert(string.Format("%1: %2 / %3", typename.EnumToString(EEditableEntityBudget, type), budgetSettings.GetCurrentBudget(), maxBudget.GetBudgetValue()));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnEditorInitServer()
	{
		m_EntityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		m_EntityCore.Event_OnEntityBudgetUpdated.Insert(OnEntityCoreBudgetUpdated);
		
		//overwrite the nulls with the actual values where valid
		foreach	(SCR_EntityBudgetValue maxBudget : m_MaxBudgets)
		{
			m_maxBudgetsInternal.Insert(maxBudget.GetBudgetType(), maxBudget);			
		}
		
		foreach (SCR_EditableEntityCoreBudgetSetting budgetSetting : m_BudgetSettingsMap)
		{
			budgetSetting.SetBudgetComponent(this);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnEditorDeleteServer()
	{
		if (m_EntityCore)
			m_EntityCore.Event_OnEntityBudgetUpdated.Remove(OnEntityCoreBudgetUpdated);
		
//		if (m_DestroyedEntityFilter)
//			m_DestroyedEntityFilter.GetOnChanged().Remove(OnDestroyedChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnEditorActivate()
	{
//		SCR_EntitiesManagerEditorComponent entitiesManager = SCR_EntitiesManagerEditorComponent.Cast(SCR_EntitiesManagerEditorComponent.GetInstance(SCR_EntitiesManagerEditorComponent, true));
//		if (entitiesManager)
//		{
//			//m_DestroyedEntityFilter = entitiesManager.GetFilter(EEditableEntityState.DESTROYED);
//			//m_DestroyedEntityFilter.GetOnChanged().Insert(OnDestroyedChanged);
//		}

		RefreshBudgetSettings();

		if (m_RplComponent && m_RplComponent.Role() != RplRole.Authority)
			Rpc(RpcServer_UpdateBudget);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnEditorInit()
	{
		m_EntityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));

	#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_BUDGET_CHANGES, "", "Log Budget Changes", "Editable Entities");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_BUDGET_CAP, "", "Enable Budget Cap", "Editable Entities");
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_BUDGET_CAP, 1);
	#endif
		
		
		//overwrite the nulls with the actual values where valid
		foreach	(SCR_EntityBudgetValue maxBudget : m_MaxBudgets)
		{
			m_maxBudgetsInternal.Insert(maxBudget.GetBudgetType(), maxBudget);			
		}
		
		foreach (SCR_EditableEntityCoreBudgetSetting budgetSetting : m_BudgetSettingsMap)
		{
			budgetSetting.SetBudgetComponent(this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_BudgetEditorComponent()
	{
	#ifdef ENABLE_DIAG
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_BUDGET_CHANGES);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_BUDGET_CAP);
	#endif
	}
}