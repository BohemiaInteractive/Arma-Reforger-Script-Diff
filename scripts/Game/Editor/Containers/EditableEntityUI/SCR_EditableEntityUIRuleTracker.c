/** @ingroup Editor_Editable_Entity_UI
*/
/*!
Tracks specific SCR_EntitiesEditorUIRule.
Controlled by SCR_EditableEntitySlotManagerUIComponent.
Manages creation and deletion of SCR_EditableEntityBaseSlotUIComponent on entities.
*/
class SCR_EditableEntityUIRuleTracker
{
	protected SCR_EntitiesEditorUIRule m_Rule;
	protected SCR_EditableEntitySlotManagerUIComponent m_SlotManager;
	
	SCR_EntitiesEditorUIRule GetRule()
	{
		return m_Rule;
	}
	bool HasState(SCR_EditableEntityComponent entity)
	{
		return (entity.GetEntityStates() | m_SlotManager.GetForcedStates()) & m_Rule.GetStates();
	}
	void AddEntity(SCR_EditableEntityComponent entity)
	{
		if (!HasState(entity))
			return;
		
		SCR_EditableEntityBaseSlotUIComponent entitySlot = m_SlotManager.FindSlot(entity, true);
		if (!entitySlot)
			return;
		
		entitySlot.CreateWidget(entity, m_Rule);
	}
	protected void OnChanged(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		if (entitiesRemove)
		{
			foreach (SCR_EditableEntityComponent entity: entitiesRemove)
			{
				//--- If some other state is active, don't delete
				if (HasState(entity))
					continue;
				
				SCR_EditableEntityBaseSlotUIComponent entitySlot = m_SlotManager.FindSlot(entity);
				if (!entitySlot)
					continue;
				
				entitySlot.DeleteWidget(m_Rule);
				m_SlotManager.DeleteSlot(entity);
			}
		}
		if (entitiesInsert)
		{
			foreach (SCR_EditableEntityComponent entity: entitiesInsert)
			{
				SCR_EditableEntityBaseSlotUIComponent entitySlot = m_SlotManager.FindSlot(entity, true);
				if (entitySlot)
					entitySlot.CreateWidget(entity, m_Rule);
			}
		}
	}
	void SCR_EditableEntityUIRuleTracker(SCR_EntitiesEditorUIRule rule, SCR_EditableEntitySlotManagerUIComponent slotManager, bool canAddAllEntities)
	{
		SCR_EntitiesManagerEditorComponent filterManager = SCR_EntitiesManagerEditorComponent.Cast(SCR_EntitiesManagerEditorComponent.GetInstance(SCR_EntitiesManagerEditorComponent, true));
		if (!filterManager)
			return;
		
		m_Rule = rule;
		m_SlotManager = slotManager;
		
		//--- Register entities
		array<int> states = {};
		m_Rule.GetStatesArray(states);
		foreach (EEditableEntityState state: states)
		{
			SCR_BaseEditableEntityFilter filter = filterManager.GetFilter(state);
			if (!filter)
				continue;
			
			filter.GetOnChanged().Insert(OnChanged);
			
			if (canAddAllEntities)
			{
				set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>;
				filter.GetEntities(entities);
				OnChanged(state, entities, null);
			}
		}
	}
	void ~SCR_EditableEntityUIRuleTracker()
	{
		SCR_EntitiesManagerEditorComponent filterManager = SCR_EntitiesManagerEditorComponent.Cast(SCR_EntitiesManagerEditorComponent.GetInstance(SCR_EntitiesManagerEditorComponent));
		if (!filterManager)
			return;
		
		array<int> states = {};
		m_Rule.GetStatesArray(states);
		foreach (EEditableEntityState state: states)
		{
			SCR_BaseEditableEntityFilter filter = filterManager.GetFilter(state);
			if (!filter)
				continue;
			
			if (filter && filter.GetOnChanged())
				filter.GetOnChanged().Remove(OnChanged);
		}
	}
};