[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
/** @ingroup Editor_Components_Entities
*/
/*!
*/
class SCR_UnlockedEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	private SCR_AccessKeysEditorComponent m_AccessKeysManager;
	private EEditableEntityAccessKey m_AccessKeyCopy = -1;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Custom functions
	protected void OnEntityAccessKeyChanged(SCR_EditableEntityComponent entity)
	{
		Toggle(entity, true);
	}
	protected void OnEditorAccessKeyChanged()
	{
		if (!GetCore()) return;
		
		if (m_AccessKeysManager) m_AccessKeyCopy = m_AccessKeysManager.GetAccessKey();
		
		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>;
		GetCore().GetAllEntities(entities);
		
		foreach (SCR_EditableEntityComponent entity: entities)
		{
			Toggle(entity, true);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Default functions
	override bool CanAdd(SCR_EditableEntityComponent entity)
	{
		return entity.HasAccessInHierarchy(m_AccessKeyCopy);
	}
	override void EOnEditorActivate()
	{
		if (!GetCore()) return;
		GetCore().Event_OnEntityAccessKeyChanged.Insert(OnEntityAccessKeyChanged);
		
		m_AccessKeysManager = SCR_AccessKeysEditorComponent.Cast(SCR_AccessKeysEditorComponent.GetInstance(SCR_AccessKeysEditorComponent));
		if (m_AccessKeysManager)
		{
			m_AccessKeyCopy = m_AccessKeysManager.GetAccessKey();
			m_AccessKeysManager.Event_OnChanged.Insert(OnEditorAccessKeyChanged);
		}
	}
	override void EOnEditorDeactivate()
	{
		if (!GetCore()) return;
		GetCore().Event_OnEntityAccessKeyChanged.Remove(OnEntityAccessKeyChanged);
		
		if (m_AccessKeysManager)
		{
			m_AccessKeysManager.Event_OnChanged.Remove(OnEditorAccessKeyChanged);
		}
	}
};