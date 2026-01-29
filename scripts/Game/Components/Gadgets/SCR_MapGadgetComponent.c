[EntityEditorProps(category: "GameScripted/Gadgets", description: "Map gadget", color: "0 0 255 255")]
class SCR_MapGadgetComponentClass : SCR_GadgetComponentClass
{
	[Attribute("0.3", UIWidgets.EditBox, desc: "seconds, delay before map gets activated giving time for the animation to be visible", params: "1 1000", category: "Map")]
	protected float m_fActivationDelay;

	[Attribute("{FC855D20AA561819}UI/Textures/Map/ProtractorScale.edds", UIWidgets.ResourcePickerThumbnail, desc: "Image of that will be used as a texture for a protractor.", params: "edds")]
	protected ResourceName m_sProtractorTexture;

	[Attribute(defvalue: "1994", uiwidget: UIWidgets.EditBox, desc: "[px] Exact length of the ruler that coresponds to 1km of distance")]
	protected float m_fRulerLength;

	//------------------------------------------------------------------------------------------------
	float GetActivationDelay()
	{
		return m_fActivationDelay;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetProtractorTexture()
	{
		return m_sProtractorTexture;
	}

	//------------------------------------------------------------------------------------------------
	float GetRulerLength()
	{
		return m_fRulerLength;
	}
}

//! Map gadget component
class SCR_MapGadgetComponent : SCR_GadgetComponent
{

	protected bool m_bIsMapOpen;
	protected bool m_bIsFirstTimeOpened = true;		// whether the map has bene opened since put into a lot
	protected SCR_MapEntity m_MapEntity;			// map instance
	protected SCR_FadeInOutEffect m_FadeInOutEffect;

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	//! Switch between map view
	//! \param[in] state is desired state: true = open, false = close
	void SetMapMode(bool state)
	{		
		if (!m_MapEntity || !m_CharacterOwner || !GetGame().GetGameMode())
			return;
		
		SCR_MapGadgetComponentClass data = SCR_MapGadgetComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;
		
		// no delay/fade for forced cancel
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(m_CharacterOwner.FindComponent(SCR_CharacterControllerComponent));
		ECharacterLifeState lifeState = controller.GetLifeState();
		float delay; 
		if (lifeState != ECharacterLifeState.DEAD) 
			delay = data.GetActivationDelay() * 1000;

		GetGame().GetCallqueue().Remove(ToggleMapGadget);
		
		if (state)
		{
			GetGame().GetCallqueue().CallLater(ToggleMapGadget, delay, false, true);
			
			if (m_FadeInOutEffect)
				m_FadeInOutEffect.FadeOutEffect(true, data.GetActivationDelay()); // fade out after map open
		}
		else		
		{
			GetGame().GetCallqueue().CallLater(ToggleMapGadget, delay, false, false);
			
			if (lifeState != ECharacterLifeState.DEAD && m_FadeInOutEffect) 
			{
				if (m_MapEntity.IsOpen())
					m_FadeInOutEffect.FadeOutEffect(true, data.GetActivationDelay()); // fade out on map close
				else 
					m_FadeInOutEffect.FadeOutEffect(false, data.GetActivationDelay()); // in case map is closed fast before it opens, fade in from close map wont trigger, so it has to happen here
			}
		}
	}
//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	//! Open/close map
	//! \param[in] state is desired state: true = open, false = close
	protected void ToggleMapGadget(bool state)
	{			
		if (state)
		{
			if (m_MapEntity.IsOpen())
				return;
			
			SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
			SCR_MapEntity.GetOnMapClose().Insert(OnMapClose);
			
			MenuManager menuManager = g_Game.GetMenuManager();
			menuManager.OpenMenu(ChimeraMenuPreset.MapMenu);
			m_bIsMapOpen = true;
		}
		else
		{			
			MenuManager menuManager = g_Game.GetMenuManager();
			menuManager.CloseMenuByPreset(ChimeraMenuPreset.MapMenu);
			m_bIsMapOpen = false;
		}		
	}
				
	//------------------------------------------------------------------------------------------------
	//! SCR_MapEntity event
	//! \param[in] config
	protected void OnMapOpen(MapConfiguration config)
	{
		if (SCR_WeaponSwitchingBaseUI.s_bOpened)
			SCR_WeaponSwitchingBaseUI.GetWeaponSwitchingBaseUI().CloseQuickSlots(); // force close it as WeaponSelectionContext will obstruct input for MapContext
		
		SCR_MapGadgetComponentClass data = SCR_MapGadgetComponentClass.Cast(GetComponentData(GetOwner()));
		if (data && m_FadeInOutEffect)
			m_FadeInOutEffect.FadeOutEffect(false, data.GetActivationDelay()); // fade in after map open
		
		// first open
		if (!m_bIsFirstTimeOpened)
			return;
		
		m_bIsFirstTimeOpened = false;
		m_MapEntity.ZoomOut();
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapEntity event
	//! \param[in] config
	protected void OnMapClose(MapConfiguration config)
	{
		SCR_MapGadgetComponentClass data = SCR_MapGadgetComponentClass.Cast(GetComponentData(GetOwner()));
		if (data && m_FadeInOutEffect)
			m_FadeInOutEffect.FadeOutEffect(false, data.GetActivationDelay()); // fade in after map close 
		
		SCR_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Remove(OnMapClose);
	}
	
	//------------------------------------------------------------------------------------------------
	override void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		super.ModeSwitch(mode, charOwner);
			
		if (mode == EGadgetMode.ON_GROUND)
			m_bIsFirstTimeOpened = true;
				
		// not current player	
		IEntity controlledEnt = SCR_PlayerController.GetLocalControlledEntity();
		if ( !controlledEnt || controlledEnt != m_CharacterOwner)
			return;
		
 		if (mode != EGadgetMode.IN_HAND)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.Cast(playerController.GetHUDManagerComponent());
		if (hudManager)
			m_FadeInOutEffect = SCR_FadeInOutEffect.Cast(hudManager.FindInfoDisplay(SCR_FadeInOutEffect));
		
		ToggleFocused(true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void ModeClear(EGadgetMode mode)
	{	
		// not current player	
		IEntity controlledEnt = SCR_PlayerController.GetLocalControlledEntity();		
		if ( !controlledEnt || controlledEnt != m_CharacterOwner )
		{
			super.ModeClear(mode);
			return;
		}
		
		if (mode == EGadgetMode.IN_HAND)
		{
			if (m_bFocused)
				ToggleFocused(false);
		}
		
		super.ModeClear(mode);
	}
	
	//------------------------------------------------------------------------------------------------
	override void ToggleFocused(bool enable)
	{
		super.ToggleFocused(enable);

		SetMapMode(enable);		
	}
			
	//------------------------------------------------------------------------------------------------
	override EGadgetType GetType()
	{
		return EGadgetType.MAP;
	}
		
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{		
		if (!m_MapEntity)
			m_MapEntity = SCR_MapEntity.GetMapInstance();
	}
}
