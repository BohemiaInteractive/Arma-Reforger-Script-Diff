//------------------------------------------------------------------------------------------------
class SCR_VoNComponentClass : VoNComponentClass
{}

//------------------------------------------------------------------------------------------------
class SCR_VoNComponent : VoNComponent 
{
	private const float TRANSMISSION_TIMEOUT_MS = 500;
	private float m_fTransmitingTimeout;
	private float m_fTransmitRadioTimeout;
	private SCR_VonDisplay m_VONDisplay;
	
	ref ScriptInvoker m_OnReceivedVON = new ScriptInvoker();
	ref ScriptInvoker m_OnCaptureVON = new ScriptInvoker();
	protected static ref ScriptInvokerInt m_OnVoNUsed;
	protected static SCR_EditorManagerCore m_EditorManagerCore = null;
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvokerInt GetOnVoNUsed()
	{
		if (!m_OnVoNUsed)
			m_OnVoNUsed = new ScriptInvokerInt();

		return m_OnVoNUsed;
	}

	//------------------------------------------------------------------------------------------------
	//! Get Display Info script
	//! \return SCR_VonDisplay reference
	SCR_VonDisplay GetDisplay()
	{
		if (m_VONDisplay)
			return m_VONDisplay;
		
		SCR_VONController vonContr = SCR_VONController.Cast(GetGame().GetPlayerController().FindComponent(SCR_VONController));
		if (vonContr)
		{
			m_VONDisplay = vonContr.GetDisplay();
			return m_VONDisplay;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Simple getter for when transmission was recently active, we assume that player's avatar
	//! should not produce various sounds while he's talking.
	//! \return true if recently active
	bool IsTransmiting()
	{
		return m_fTransmitingTimeout > GetGame().GetWorld().GetWorldTime();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Radio transmission was recently active
	//! \return true if recently active
	bool IsTransmitingRadio()
	{
		return m_fTransmitRadioTimeout > GetGame().GetWorld().GetWorldTime();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected event void OnCapture(BaseTransceiver transmitter)
	{
		SCR_VonDisplay display = GetDisplay();
		
		if (display)
			display.OnCapture(transmitter);
		
		m_OnCaptureVON.Invoke(transmitter);
		
		m_fTransmitingTimeout = GetGame().GetWorld().GetWorldTime() + TRANSMISSION_TIMEOUT_MS;
		
		if (transmitter)
			m_fTransmitRadioTimeout = GetGame().GetWorld().GetWorldTime() + TRANSMISSION_TIMEOUT_MS;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected event void OnReceive(int playerId, bool isSenderEditor, BaseTransceiver receiver, int frequency, float quality)
	{
		SCR_VonDisplay display = GetDisplay();
		
		if (display)
			display.OnReceive(playerId, isSenderEditor, receiver, frequency, quality);

		m_OnReceivedVON.Invoke(playerId, receiver, frequency, quality);
	}

	//------------------------------------------------------------------------------------------------
	override protected event void OnVoNUsed(int senderId)
	{
		if (m_OnVoNUsed)
			m_OnVoNUsed.Invoke(senderId);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected event IEntity GetEditorEntity(int playerId)
	{
		const SCR_EditorManagerCore core = GetEditorManagerCore();
		if (!core)
			return null;

		return core.GetEditorManager(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected event vector GetEditorWorldLocation(int playerId)
	{	
		const SCR_EditorManagerCore core = GetEditorManagerCore();
		if (!core)
			return vector.Zero;
		
		SCR_EditorManagerEntity editorManager = core.GetEditorManager(playerId);
		if (!editorManager)
			return vector.Zero;
		
		vector mat[4];
		editorManager.GetWorldTransform(mat);
		return mat[3];
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsEntityActiveEditor(IEntity entity)
	{
		if (!entity)
			return false;
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.Cast(entity);
		if (!editorManager)
			return false;
		
		return editorManager.IsOpened();
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsLocalActiveEditor()
	{
		const SCR_EditorManagerCore core = GetEditorManagerCore();
		if (!core)
			return false;
		
		SCR_EditorManagerEntity editorManager = core.GetEditorManager();
		if (!editorManager)
			return false;
		
		return editorManager.IsOpened();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected IEntity GetActiveEditorEntity(int playerID)
	{
		const IEntity ent = GetEditorEntity(playerID);

		//entity is already nullchecked here
		const bool isActive = IsEntityActiveEditor(ent);
		if (!isActive)
			return null;
		
		return ent;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static SCR_EditorManagerCore GetEditorManagerCore()
	{
		//if editor manager core exists we return it
		if(m_EditorManagerCore)
			return m_EditorManagerCore;
		
		m_EditorManagerCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		return m_EditorManagerCore;
	}
};
