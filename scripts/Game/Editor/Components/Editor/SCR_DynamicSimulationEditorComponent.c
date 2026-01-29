[ComponentEditorProps(category: "GameScripted/Editor", description: "System to force streaming of all editable entities to client who has opened editor. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_DynamicSimulationEditorComponentClass: SCR_BaseEditorComponentClass
{
};

/** @ingroup Editor_Components
*/
/*!
System to force streaming of all editable entities to client who has opened editor.
*/
class SCR_DynamicSimulationEditorComponent : SCR_BaseEditorComponent
{
	protected RplIdentity m_EditorIdentity = RplIdentity.Invalid();
	
	protected ref array<EEditableEntityType> m_LocalSkipStreamingRules = {};
	
	protected bool m_bStreamingRulesInitialized = false;
	
	protected void EnableStreaming(bool enable)
	{
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return;
		
		//--- Get editor owner's RplIdentity
		if (!m_EditorIdentity.IsValid())
		{
			PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(GetManager().GetPlayerID());
			if (playerController)
				m_EditorIdentity = playerController.GetRplIdentity();
		}
		
		//--- Ignore when it's local on server, no need to configure streaming in such case
		if (m_EditorIdentity == RplIdentity.Local())
			return;
		
		//--- Get all entities (ToDo: Only limited subsection e.g., players in admin mode)
		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>();
		core.GetAllEntities(entities);
		for (int i = 0, count = entities.Count(); i < count; i++)
		{
			EnableStreamingForEntity(entities[i], enable);
		}
	}
	protected void EnableStreamingForEntity(SCR_EditableEntityComponent entity, bool enable)
	{
		//--- Check if the entity was meanwhile deleted (e.g., when placing a waypoint that's instantly completed)
		if (!entity || !entity.GetOwner())
			return;
		
		if(!m_bStreamingRulesInitialized)
		{
			SCR_EditorSettingsEntity editorSettingsEntity = SCR_EditorSettingsEntity.GetInstance();
			if(editorSettingsEntity && m_LocalSkipStreamingRules)
			{
				editorSettingsEntity.GetSkipStreamingRules(m_LocalSkipStreamingRules);
				m_bStreamingRulesInitialized = true;
			}
		}	
		
		if(m_LocalSkipStreamingRules.Contains(entity.GetEntityType()))
			return;
		
		RplComponent entityRpl = RplComponent.Cast(entity.GetOwner().FindComponent(RplComponent));
		if (entityRpl)
		{
			entityRpl.EnableStreamingConNode(m_EditorIdentity, enable);
			/*
			if (enable)
				entity.Log(string.Format("Enable streaming of %1 for %2",entityRpl, m_EditorIdentity), true, LogLevel.DEBUG);
			else
				entity.Log(string.Format("Disable streaming of %1 for %2",entityRpl, m_EditorIdentity), true, LogLevel.WARNING);
			*/
		}
	}
	
	protected void OnEntityRegistered(SCR_EditableEntityComponent entity)
	{
		if (GetManager().IsOpened() && m_EditorIdentity != RplIdentity.Local())
			GetGame().GetCallqueue().CallLater(EnableStreamingForEntity, 1, false, entity, false); //--- Give replication time to initialize
	}
	protected void OnEntityUnregistered(SCR_EditableEntityComponent entity)
	{
		if (GetManager().IsOpened() && m_EditorIdentity != RplIdentity.Local())
			EnableStreamingForEntity(entity, true);
	}
	
	override void EOnEditorActivateServer()
	{
		EnableStreaming(false);
		
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
			core.Event_OnEntityRegistered.Insert(OnEntityRegistered);
	}
	override void EOnEditorDeactivateServer()
	{
		EnableStreaming(true);
		
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
			core.Event_OnEntityRegistered.Remove(OnEntityRegistered);
	}
};