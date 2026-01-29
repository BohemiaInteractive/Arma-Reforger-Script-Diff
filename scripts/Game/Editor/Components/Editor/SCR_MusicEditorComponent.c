[EntityEditorProps(category: "GameScripted/Sound", description: "Music component", color: "0 0 255 255")]
class SCR_MusicEditorComponentClass: SCR_BaseEditorComponentClass
{
};
class SCR_MusicEditorComponent: SCR_BaseEditorComponent
{
	SCR_EditorManagerEntity m_EditorManager;
	protected bool m_bMusicWasMutedByEditor;
	protected MusicManager m_MusicManager;
	
	protected void OnEditorOpened()
	{
		//Add a short delay to make sure any ambient music that started to play is terminated correctly
		GetGame().GetCallqueue().CallLater(OpenedEditorDelay, 500);
	}
	
	protected void OpenedEditorDelay()
	{
		if (!m_EditorManager.IsOpened() || m_EditorManager.IsLimited())
			return;
		
		m_bMusicWasMutedByEditor = true;
		if (m_MusicManager)
		{
			m_MusicManager.MuteCategory(MusicCategory.Ambient, true, true);
			m_MusicManager.MuteCategory(MusicCategory.Menu, true, true);
			m_MusicManager.MuteCategory(MusicCategory.Misc, true, true);
		}
	}
	
	protected void OnEditorClosed()
	{
		//Checks this in cause the limited mode changed
		if (m_bMusicWasMutedByEditor && m_MusicManager)
		{
			m_MusicManager.MuteCategory(MusicCategory.Ambient, false, false);
			m_MusicManager.MuteCategory(MusicCategory.Menu, false, false);
			m_MusicManager.MuteCategory(MusicCategory.Misc, false, false);
		}
			
		
		m_bMusicWasMutedByEditor = false;
	}
	
	override void EOnEditorInit()
	{
		if (SCR_Global.IsEditMode()) 
			return;
		
		super.EOnEditorInit();
		
		ChimeraWorld world = GetGame().GetWorld();

		if (!world)
			return;
		
		m_MusicManager = world.GetMusicManager();
		if (!m_MusicManager)
			return;
		
		m_EditorManager = SCR_EditorManagerEntity.GetInstance();
		if (m_EditorManager)
		{
			m_EditorManager.GetOnOpened().Insert(OnEditorOpened);
			m_EditorManager.GetOnClosed().Insert(OnEditorClosed);
			
			if (m_EditorManager.IsOpened())
				OnEditorOpened();
		}
	}
	
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		
		if (m_EditorManager)
		{
			m_EditorManager.GetOnOpened().Remove(OnEditorOpened);
			m_EditorManager.GetOnClosed().Remove(OnEditorClosed);
		}
	}
};
