[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
/** @ingroup Editor_Components_Entities
*/
/*!
Dummy mentities manager.
Doesn't have functionality of its own, links variables to its parent.
Intended to set otherwise unused states without having to manage extra array.
*/
class SCR_DummyEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	protected void OnDummyChanged(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		if (entitiesRemove)
		{
			for (int i, count = entitiesRemove.Count(); i < count; i++)
			{
				entitiesRemove[i].SetEntityState(m_State, false);
			}
		}
		if (entitiesInsert)
		{
			for (int i, count = entitiesInsert.Count(); i < count; i++)
			{
				entitiesInsert[i].SetEntityState(m_State, true);
			}
		}
	}
	
	override int GetEntities(out set<SCR_EditableEntityComponent> entities, bool includeChildren = false, bool evaluate = true)
	{
		if (GetPredecessor()) return GetPredecessor().GetEntities(entities, includeChildren);
		if (entities) entities.Clear();
		return 0;
	}
	override int GetEntitiesPointer(out set<SCR_EditableEntityComponent> entities)
	{
		if (GetPredecessor()) return GetPredecessor().GetEntitiesPointer(entities);
		return 0;
	}
	override int GetEntitiesCount()
	{
		if (GetPredecessor()) return GetPredecessor().GetEntitiesCount();
		return 0;
	}
	override SCR_EditableEntityComponent GetFirstEntity()
	{
		if (GetPredecessor()) return GetPredecessor().GetFirstEntity();
		return null;
	}
	override bool Contains(SCR_EditableEntityComponent entity)
	{
		if (GetPredecessor()) return GetPredecessor().Contains(entity);
		return false;
	}
	override ScriptInvokerBase<SCR_BaseEditableEntityFilter_OnChange> GetOnChanged()
	{
		if (GetPredecessor())
			return GetPredecessor().GetOnChanged();
		else
			return super.GetOnChanged();
	}
	override protected void InitEntities()
	{
		//--- Deliberate nothing
	}
	override void EOnEditorActivate()
	{
		if (GetPredecessor())
		{
			GetPredecessor().GetOnChanged().Insert(OnDummyChanged);
			
			set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>();
			GetPredecessor().GetEntitiesPointer(entities);
			OnDummyChanged(m_State, entities, null);
		}
	}
	override void EOnEditorDeactivate()
	{
		if (GetPredecessor())
		{
			GetPredecessor().GetOnChanged().Remove(OnDummyChanged);
			
			set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>();
			GetPredecessor().GetEntitiesPointer(entities);
			OnDummyChanged(m_State, null, entities);
		}
	}
};