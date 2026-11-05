#ifdef WORKBENCH
	#define SHOW_DISCLAIMER
#endif

[ComponentEditorProps(category: "GameScripted/Editor", description: "Limited camera for in-game editor. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CameraLimitedEditorComponentClass : SCR_CameraEditorComponentClass
{
}

/** @ingroup Editor_Components
*/
/*!
Limited version of camera manager.

Contains two prefabs - one used only when all editor modes have also limited camera, and the other when there is at least one unlimited camera.
*/
class SCR_CameraLimitedEditorComponent : SCR_CameraEditorComponent
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Prefab of class SCR_ManualCamera", "et", category: "Camera")]
	private ResourceName m_LimitedCameraPrefab;
	
	protected bool m_bIsLocked;
	protected bool m_bIsLimited;
	protected ChimeraCharacter m_Player;
	protected SCR_EditorManagerEntity m_editorManager;
	protected CharacterControllerComponent m_PlayerController;

#ifdef SHOW_DISCLAIMER
	private ref DebugTextScreenSpace m_Disclaimer;
	private float m_fDisclaimerCountdown;
#endif

	//------------------------------------------------------------------------------------------------
	override protected ResourceName GetCameraPrefab()
	{
		if (!m_editorManager)
			return ResourceName.Empty;

#ifdef SHOW_DISCLAIMER
		if (m_bIsLimited)
		{
			m_Disclaimer = DebugTextScreenSpace.Create(GetGame().GetWorld(), "", 0, 50, 50, 10, ARGBF(1, 1, 1, 1), ARGBF(1, 0, 0, 0));
			m_fDisclaimerCountdown = 15;
		}
#endif

		if (!m_bIsLimited)
			return super.GetCameraPrefab();
	
		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (settings)
		{
			ResourceName prefabOverride = settings.GetLimitedArmavisionCameraPrefabOverride();
			if (!prefabOverride.IsEmpty())
				return prefabOverride;
		}
	
		return m_LimitedCameraPrefab;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
#ifdef SHOW_DISCLAIMER
		if (m_Disclaimer)
		{
			m_fDisclaimerCountdown -= timeSlice;
			m_Disclaimer.SetText(string.Format("Armavision camera is restricted around player when no other editor modes are available.\n\nTo unlock it for testing, activate: Debug Menu > Manual Camera > Unlimited Movement\nRestart the editor afterwards.\n\nThis message will disappear in %1 s.", Math.Ceil(m_fDisclaimerCountdown)));
			if (m_fDisclaimerCountdown < 0) 
				delete m_Disclaimer;
		}
#endif

		if (m_bIsLocked && m_editorManager && (!m_PlayerController || m_PlayerController.IsUnconscious()))
			m_editorManager.Close();

		if (m_bIsLocked)
			return;

		super.EOnFrame(owner, timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorActivate()
	{
		super.EOnEditorActivate();
		if (m_bIsLocked)
			SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PHOTOMODE_IS_LOCKED);
		
#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_UNLIMITED, "", "Unlimited Movement", "Manual Camera");
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorPreActivate()
	{
		m_editorManager = SCR_EditorManagerEntity.GetInstance();
		if (m_editorManager)
			m_editorManager.GetOnLimitedChange().Insert(ReplaceCamera);

		UpdateCameraRestrictions();
		if (m_bIsLocked && m_editorManager && (!m_PlayerController || m_PlayerController.IsUnconscious()))
			m_editorManager.Close();

		super.EOnEditorPreActivate();
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorDeactivate()
	{
		super.EOnEditorDeactivate();

		if (m_editorManager)
			m_editorManager.GetOnLimitedChange().Remove(ReplaceCamera);

		//DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_UNLIMITED);

#ifdef SHOW_DISCLAIMER
		delete m_Disclaimer;
#endif
	}

	//------------------------------------------------------------------------------------------------
	// Whether multiplayer rules block the camera and entity overlays of this ArmaVision mode
	static bool IsArmavisionBlocked(SCR_EditorModeEntity mode)
	{
		if (!mode || (mode.GetModeType() != EEditorMode.PHOTO && mode.GetModeType() != EEditorMode.PHOTO_SAVE))
			return false;

		SCR_EditorManagerEntity editorManager = mode.GetManager();
		if (!editorManager || !editorManager.IsLimited())
			return false;

#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_UNLIMITED))
			return false;
#endif

		if (RplSession.Mode() == RplMode.None)
			return false;

		SCR_MissionHeader missionHeader = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
		return !missionHeader || !missionHeader.m_bIsArmavisionAllowedInMP;
	}

	//------------------------------------------------------------------------------------------------
	// Re-evaluate rights whenever the camera is opened or replaced
	protected void UpdateCameraRestrictions()
	{
		m_Player = ChimeraCharacter.Cast(EntityUtils.GetPlayer());
		m_PlayerController = null;
		if (m_Player)
			m_PlayerController = m_Player.GetCharacterController();

		m_bIsLimited = m_editorManager && m_editorManager.IsLimited();
#ifdef ENABLE_DIAG
		m_bIsLimited = m_bIsLimited && !DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_UNLIMITED);
#endif
		m_bIsLocked = IsArmavisionBlocked(SCR_EditorModeEntity.Cast(GetOwner()));
	}

	//------------------------------------------------------------------------------------------------
	override protected void CreateCamera()
	{
		UpdateCameraRestrictions();
		if (m_bIsLocked)
		{
			delete m_Camera;
			return;
		}

		super.CreateCamera();
	}
}
