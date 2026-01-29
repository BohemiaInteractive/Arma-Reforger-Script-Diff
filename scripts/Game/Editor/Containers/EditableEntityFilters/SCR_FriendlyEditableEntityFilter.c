[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
/** @ingroup Editor_Components_Entities
*/
/*!
Filter entities which are friendly to player.
When the editor is not limited, all entities with a faction are accepted.
*/
class SCR_FriendlyEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	private SCR_PlayersManagerEditorComponent m_PlayersManager;
	private Faction m_Faction;
	private bool m_bIsLimited;
	private SCR_EditorManagerEntity m_EditorManager;
	
	protected void CachePlayerFaction(GenericEntity owner)
	{
		if (!owner) return;
		
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(owner.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliation) return;
		
		m_Faction = factionAffiliation.GetAffiliatedFaction();
	}
	protected bool IsFriendly(GenericEntity owner)
	{
		if (!owner || !m_Faction) return false;
		
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(owner.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliation) return false;
		
		Faction faction = factionAffiliation.GetAffiliatedFaction();
		if (!faction) return false;
		
		return !m_bIsLimited || m_Faction.IsFactionFriendly(faction);
	}
	
	protected void OnLimitedChange(bool isLimited)
	{
		m_bIsLimited = isLimited;
		SetFromPredecessor();
	}
	protected void OnSpawn(int playerID, SCR_EditableEntityComponent entity, SCR_EditableEntityComponent entityPrev)
	{
		if (playerID != SCR_PlayerController.GetLocalPlayerId())
			return;
		
		CachePlayerFaction(entity.GetOwner());
		SetFromPredecessor();
	}
	override bool CanAdd(SCR_EditableEntityComponent entity)
	{
		if (m_EditorManager.IsLimited())
			return IsFriendly(entity.GetOwner()); //--- Limited editor, show only friendly players
		else
			return true; //--- Unlimited editor, show all players
	}
	override void EOnEditorActivate()
	{
		m_EditorManager = GetManager().GetManager();
		
		m_PlayersManager = SCR_PlayersManagerEditorComponent.Cast(SCR_PlayersManagerEditorComponent.GetInstance(SCR_PlayersManagerEditorComponent, true));
		if (!m_PlayersManager) return;
		
		m_PlayersManager.GetOnSpawn().Insert(OnSpawn);
		CachePlayerFaction(GenericEntity.Cast(SCR_PlayerController.GetLocalControlledEntity()));
		
		//--- ToDo: Replace by sandbox solution usable in all filters?
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
		{
			OnLimitedChange(editorManager.IsLimited());
			editorManager.GetOnLimitedChange().Insert(OnLimitedChange);
		}
	}
	override void EOnEditorDeactivate()
	{
		if (m_PlayersManager) m_PlayersManager.GetOnSpawn().Remove(OnSpawn);
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager) editorManager.GetOnLimitedChange().Remove(OnLimitedChange);
	}
};