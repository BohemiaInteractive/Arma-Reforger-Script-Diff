class SCR_EditableEntitySlotManagerUIComponent : SCR_BaseEditorUIComponent
{
	[Attribute()]
	protected ref SCR_EditableEntityUIConfig m_EditableEntityUI;
	
	[Attribute(desc: "List of only states which will be tracked. When empty, all states will be tracked.\nUsed to show only specific icons, e.g., only the default one (RENDERED) and player overlay (PLAYER).", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EEditableEntityState))]
	protected EEditableEntityState m_WhitelistedStates;
	
	[Attribute(desc: "List of states which will not be tracked, even when they're part of Editable Entity UI config.", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EEditableEntityState))]
	protected EEditableEntityState m_BlacklistedStates;
	
	[Attribute(desc: "States which are considered always active.\nUsed for example to force RENDERED state even for entities which are not shown in the scene.", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EEditableEntityState))]
	protected EEditableEntityState m_ForcedStates;
	
	[Attribute("0", desc: "Register all entities which meet the rules at the beginning.")]
	protected bool m_bAddAllEntities;
	
	protected EEditableEntityState m_States;
	protected ref map<SCR_EditableEntityComponent, SCR_EditableEntityBaseSlotUIComponent> m_mEntitySlots = new map<SCR_EditableEntityComponent, SCR_EditableEntityBaseSlotUIComponent>();
	protected bool m_bInteractive;
	
	protected ref map<SCR_EntitiesEditorUIRule, ref SCR_EditableEntityUIRuleTracker> m_RuleTrackers;// = new map<SCR_EntitiesEditorUIRule, ref SCR_EditableEntityUIRuleTracker>();
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EEditableEntityState GetForcedStates()
	{
		return m_ForcedStates;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Registration
	////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	//! \param[in] createIfNull
	//! \return
	SCR_EditableEntityBaseSlotUIComponent FindSlot(SCR_EditableEntityComponent entity, bool createIfNull = false)
	{
		SCR_EditableEntityBaseSlotUIComponent slot;
		m_mEntitySlots.Find(entity, slot);
		return slot;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	//! \param[in] forced
	//! \return
	bool DeleteSlot(SCR_EditableEntityComponent entity, bool forced = false)
	{
		//--- If some other state is active, don't unregister
		if (!forced && (entity.GetEntityStates() | m_ForcedStates) & m_States)
			return false;
		
		//--- Unregister
		SCR_EditableEntitySceneSlotUIComponent entitySlot;
		if (m_mEntitySlots.Find(entity, entitySlot))
		{
			entitySlot.DeleteSlot();
			m_mEntitySlots.Remove(entity);
			return true;
		}
		else
		{
			return false;
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	//! \param[in] slot
	void InsertSlot(SCR_EditableEntityComponent entity, SCR_EditableEntityBaseSlotUIComponent slot)
	{
		slot.InitSlot(entity);
		m_mEntitySlots.Insert(entity, slot);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	//! \param[in] slot
	void InsertSlotExternal(SCR_EditableEntityComponent entity, SCR_EditableEntityBaseSlotUIComponent slot)
	{
		if (m_mEntitySlots.Contains(entity))
		{
			entity.Log("Attempting to create duplicate GUI slot for editable entity. Duplicates are not allowed!", true, LogLevel.WARNING);
			return;
		}
		
		slot.InitSlot(entity);
		m_mEntitySlots.Insert(entity, slot);
		
		InitSlotManager();
		
		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>();
		entities.Insert(entity);
		
		if (m_RuleTrackers)
		{
			foreach (SCR_EditableEntityUIRuleTracker ruleTracker : m_RuleTrackers)
			{
				ruleTracker.AddEntity(entity);
				//ruleTracker.OnChanged(entity.GetEntityStates(), entities, null);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	void ClearSlots()
	{
		m_mEntitySlots.Clear();
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_EditableEntityBaseSlotUIComponent GetEntitySlot(Widget w)
	{
		SCR_EditableEntityBaseSlotUIComponent entityUI;
		while (w)
		{
			entityUI = SCR_EditableEntityBaseSlotUIComponent.Cast(w.FindHandler(SCR_EditableEntityBaseSlotUIComponent));
			if (entityUI)
				return entityUI;
			
			w = w.GetParent();
		}

		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void InitSlotManager()
	{
		if (m_RuleTrackers)
			return;
		
		SCR_EntitiesManagerEditorComponent entityManager = SCR_EntitiesManagerEditorComponent.Cast(SCR_EntitiesManagerEditorComponent.GetInstance(SCR_EntitiesManagerEditorComponent, true));
		if (!entityManager)
			return;
		
		//--- Init rules
		SCR_EditableEntityUIConfig entityUIConfig = SCR_EditableEntityUIConfig.GetConfig(m_EditableEntityUI);
		if (entityUIConfig)
		{
			array<ref SCR_EntitiesEditorUIRule> rules = entityUIConfig.GetRules();
			if (rules && !rules.IsEmpty())
			{
				SCR_EditableEntityUIRuleTracker ruleTracker;
				m_RuleTrackers = new map<SCR_EntitiesEditorUIRule, ref SCR_EditableEntityUIRuleTracker>();
				foreach (SCR_EntitiesEditorUIRule rule: rules)
				{
					//--- Passes when all states are whitelisted (or no whitelist is defined) and when there is at least one state not blacklisted
					EEditableEntityState states = rule.GetStates();
					if ((m_WhitelistedStates == 0 || (states & m_WhitelistedStates)) && (states & ~m_BlacklistedStates) != 0)
					{
						ruleTracker = new SCR_EditableEntityUIRuleTracker(rule, this, m_bAddAllEntities);
						m_RuleTrackers.Insert(rule, ruleTracker);
						m_bInteractive |= rule.IsInteractive();
						
						if (!rule.IsDependent())
							m_States |= states;
					}
				}
			}
			else
			{
				Print("UI rules not defined in SCR_EditableEntityUIConfig!", LogLevel.WARNING);
			}
		}
		else
		{
			Print("SCR_EditableEntityUIConfig not defined!", LogLevel.WARNING);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Default methods
	////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	override bool IsUnique()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		super.HandlerAttachedScripted(w);
		if (SCR_Global.IsEditMode()) return; //--- Run-time only
		
		InitSlotManager();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		if (SCR_Global.IsEditMode())
			return; //--- Run-time only
		
		if (m_RuleTrackers)
		{
			SCR_EditableEntityUIConfig entityUIConfig = SCR_EditableEntityUIConfig.GetConfig(m_EditableEntityUI);
			if (entityUIConfig)
			{
				SCR_EditableEntityUIRuleTracker ruleTracker;
				foreach (SCR_EntitiesEditorUIRule rule: entityUIConfig.GetRules())
				{
					if (m_RuleTrackers.Find(rule, ruleTracker) && ruleTracker)
					{
						m_RuleTrackers.Remove(rule);
						delete ruleTracker;
					}
				}
			}
		}
	}
}
