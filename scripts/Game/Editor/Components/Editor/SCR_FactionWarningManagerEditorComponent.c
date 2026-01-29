[ComponentEditorProps(category: "GameScripted/Editor", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_FactionWarningManagerEditorComponentClass: SCR_BaseEditorComponentClass
{
};

/** @ingroup Editor_Components
Manager of warnings of factions such as spawnpoints and Tasks
*/
class SCR_FactionWarningManagerEditorComponent : SCR_BaseEditorComponent
{/*
	protected bool m_bIsListeningToEvents;
	protected SCR_BaseTaskManager m_TaskManager;

	
	protected override void EOnEditorInit()
	{
		if (RplSession.Mode() == RplMode.Client) return;
		
		m_TaskManager = SCR_BaseTaskManager.GetInstance();
		Print(m_TaskManager);
	}
	
	
	
	protected void SetServerHandler(bool listenToEvents)
	{
		if (RplSession.Mode() == RplMode.Client || m_bIsListeningToEvents == listenToEvents || !m_TaskManager) return;
		
		
		if (listenToEvents)
		{
			m_TaskManager.s_OnTaskUpdate.Insert(OnFactionTaskChanged);
			m_TaskManager.s_OnTaskDeleted.Insert(OnFactionTaskChanged);
		}
		else
		{
			m_TaskManager.s_OnTaskUpdate.Remove(OnFactionTaskChanged);
			m_TaskManager.s_OnTaskDeleted.Remove(OnFactionTaskChanged);
		}
		
		m_bIsListeningToEvents = listenToEvents;
	}
	
	protected void OnSpawnPointChanged()
	{
	
	}
	
	protected void OnTaskChanged(SCR_BaseTask task)
	{
	
	}
	
	
	override void EOnEditorOpenServer()
	{
		SetServerHandler(true);
	}
	override void EOnEditorCloseServer()
	{
		SetServerHandler(false);
	}
	
	protected override void OnDelete(IEntity owner)
	{
		SetServerHandler(false);
	}*/
};