[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
/** @ingroup Editor_Components_Entities
*/
/*!
Entities which are killed or destroyed.
*/
class SCR_DestroyedEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	[Attribute("100", desc: "When destroyed, entity draw distance will be set to this value so it doesn't clutter the screen.")]
	protected float m_fDrawDistance;
	
	protected float m_fDrawDistanceSq;
	protected ref set<SCR_EditableEntityComponent> m_EntitiesWithHandlers;
	
	protected void OnDestroyed(IEntity owner)
	{
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.GetEditableEntity(owner);
		if (entity)
			Add(entity);
	}
	protected void OnPredecessorChanged(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		if (!entitiesRemove) return;
		
		//--- Remove the handler from entities which were removed from predecessor's state
		foreach (SCR_EditableEntityComponent entity: entitiesRemove)
		{
			GenericEntity owner = entity.GetOwner();
			if (!owner) continue;
			
			EventHandlerManagerComponent eventManager = EventHandlerManagerComponent.Cast(owner.FindComponent(EventHandlerManagerComponent));
			if (!eventManager) continue;
			
			eventManager.RemoveScriptHandler("OnDestroyed", entity.GetOwner(), OnDestroyed);
			
			int index = m_EntitiesWithHandlers.Find(entity);
			if (index != -1) m_EntitiesWithHandlers.Remove(index);
		}
	}
	
	override bool CanAdd(SCR_EditableEntityComponent entity)
	{
		GenericEntity owner = entity.GetOwner();
		if (!owner)
			return false;
		
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(owner.FindComponent(DamageManagerComponent));
		if (!damageManager)
			return false;
		
		//--- Check if already destroyed. If not, add OnDestroyed event handler and listen to the change
		if (damageManager.GetState() == EDamageState.DESTROYED)
			return true;
		
		//--- Already has the handler, ignore
		if (m_EntitiesWithHandlers.Find(entity) != -1)
			return false;
		
		//-- Find event handler manager so we can track destroyed change
		EventHandlerManagerComponent eventManager = EventHandlerManagerComponent.Cast(entity.GetOwner().FindComponent(EventHandlerManagerComponent));
		if (!eventManager)
		{
			//entity.Log("Entity has DamageManagerComponent, but not EventHandlerManagerComponent!", true, LogLevel.WARNING);
			return false;
		}
		
		//--- Add the handler
		eventManager.RegisterScriptHandler("OnDestroyed", entity.GetOwner(), OnDestroyed);
		m_EntitiesWithHandlers.Insert(entity);
		return false;
	}
	override void OnChanged(set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		super.OnChanged(entitiesInsert, entitiesRemove);
		if (m_fDrawDistance >= 0 && entitiesInsert)
		{
			foreach (SCR_EditableEntityComponent entity: entitiesInsert)
			{
				if (entity.GetMaxDrawDistanceSq() != m_fDrawDistanceSq)
					entity.SetMaxDrawDistance(m_fDrawDistance);
			}
		}
	}
	
	override void EOnEditorActivate()
	{
		SCR_BaseEditableEntityFilter predecessor = GetPredecessor();
		if (predecessor) predecessor.GetOnChanged().Insert(OnPredecessorChanged);
		
		m_EntitiesWithHandlers = new set<SCR_EditableEntityComponent>;
	}
	override void EOnEditorDeactivate()
	{
		SCR_BaseEditableEntityFilter predecessor = GetPredecessor();
		if (predecessor) predecessor.GetOnChanged().Remove(OnPredecessorChanged);
		
		m_EntitiesWithHandlers = null;
	}
	void SCR_DestroyedEditableEntityFilter()
	{
		m_fDrawDistanceSq = m_fDrawDistance * m_fDrawDistance;
	}
};