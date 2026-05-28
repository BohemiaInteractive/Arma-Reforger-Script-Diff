[EntityEditorProps(category: "GameScripted/Gadgets", description: "Binoculars gadget", color: "0 0 255 255")]
class SCR_BinocularsComponentClass : SCR_GadgetComponentClass
{
}

class SCR_BinocularsComponent : SCR_GadgetComponent
{
	protected static ref ScriptInvoker s_OnBinocToggled = new ScriptInvoker();
	
	protected Widget m_RootWidget = null;	// binoculars layout
	
	// Optics reference 
	protected SCR_2DOpticsComponent m_Optic;
	
	//------------------------------------------------------------------------------------------------
	//! Set whether the local character is zoomed in with the binoculars
	protected void SetZoomedViewPlayer(bool value)
	{
		SCR_PlayerController.s_pLocalPlayerController.SetIsBinocularsZoomed(value);
	}

	//------------------------------------------------------------------------------------------------
	//! Switch between zoomed view
	//! \param[in] state is desired state
	private void SetZoomedView(bool state)
	{
		// Check for optics 
		if (!m_Optic)	
			m_Optic = SCR_2DOpticsComponent.Cast(GetOwner().FindComponent(SCR_2DOpticsComponent));
		
		if (!m_Optic)
		{
			Print("Optic component in binoculars was not found!", LogLevel.ERROR);
			return;
		}
		
		// Activate optics 
		if (state)
			m_Optic.OnSightADSActivated();
		else
			m_Optic.OnSightADSDeactivated();
		
		SetZoomedViewPlayer(state);

		// Invoke use
		if (GetOwner().GetParent() == SCR_PlayerController.GetLocalControlledEntity())
			s_OnBinocToggled.Invoke(state);
	}

	//------------------------------------------------------------------------------------------------
	override void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		super.ModeSwitch(mode, charOwner); 
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
		
		if (m_iMode == EGadgetMode.IN_HAND && SCR_PlayerController.s_pLocalPlayerController && SCR_PlayerController.s_pLocalPlayerController.GetIsBinocularsZoomed())
			ToggleFocused(false);
		
		super.ModeClear(mode);
	}
			
	//------------------------------------------------------------------------------------------------
	override void ToggleFocused(bool enable)
	{
		super.ToggleFocused(enable);
		
		SetZoomedView(enable);
	}
	
	//------------------------------------------------------------------------------------------------
	override EGadgetType GetType()
	{
		return EGadgetType.BINOCULARS;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeRaised()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsUsingADSControls()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsVisibleEquipped()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_BinocularsComponent()
	{
		if (m_RootWidget)
		{
			m_RootWidget.SetVisible(false);
			delete m_RootWidget;
		}
	}
}
