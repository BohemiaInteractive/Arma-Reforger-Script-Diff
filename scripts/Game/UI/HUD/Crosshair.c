// Todo: PerFrame Optimization and Refactor
class SCR_Crosshair : SCR_InfoDisplay
{
	private const float m_fFadeRate = 8;
	private SCR_CharacterControllerComponent m_CharacterController;
	private DamageManagerComponent m_DamageManager;
	private SCR_PlayerController m_PlayerController;
	private BaseInteractionHandlerComponent m_InteractionComponent;
	
	//------------------------------------------------------------------------------------------------
	override event void UpdateValues(IEntity owner, float timeSlice)
	{
		if (!m_CharacterController || !m_InteractionComponent)
			return;
		/*
		// Todo: Remove this per frame check and rewrite it at event base system
		// Todo: OnWeaponModeChanged
		bool ads = m_CharacterController.IsWeaponADS();
		// Todo: OnFreelookToggle( bool enabled )
		bool freeLook = m_CharacterController.IsFreeLookEnabled();
		// Todo: OnWeaponModeChanged
		bool weaponRaised = m_CharacterController.IsWeaponRaised();
		// Todo: OnCharacterStateChanged
		bool playingGesture = m_CharacterController.IsPlayingGesture();
		// Todo: OnCharacterStateChanged
		bool sprinting = m_CharacterController.GetSprinting();
		// Todo: OnCharacterDie
		bool alive = true;
		if (m_DamageManager && m_DamageManager.GetState() != EDamageState.ALIVE)
			alive = false;
		*/
	
		// TODO: Check why some methods are not working
		//bool show = /*weaponRaised && */!ads && !playingGesture && /*!sprinting &&*/ !freeLook && alive;
		
		bool show = m_InteractionComponent.IsInteractionAvailable();
		Show(show, m_fFadeRate);
	}

	//------------------------------------------------------------------------------------------------
	override event void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		if (m_wRoot)
		{
			m_wRoot.SetOpacity(0);
			m_bShown = false;
		}
		
		ChimeraGame g = GetGame();
		if (g)
		{
			PlayerController controller = g.GetPlayerController();
			if (controller)
				m_InteractionComponent = BaseInteractionHandlerComponent.Cast(controller.FindComponent(BaseInteractionHandlerComponent));
		}
		
		ChimeraCharacter character = ChimeraCharacter.Cast(owner);
		if (character)
		{
			m_CharacterController = SCR_CharacterControllerComponent.Cast(character.FindComponent(SCR_CharacterControllerComponent));
			m_DamageManager = DamageManagerComponent.Cast(character.FindComponent(DamageManagerComponent));
			
			if (!m_CharacterController)
			{
				Print(string.Format("Character Controller not found in %1 for %2, HUD will not work.", "CharacterUnitInfoDisplay", character.GetName()), LogLevel.WARNING);		
				return;
			}
		}
	}
};
