class CharacterCameraBinoculars : CharacterCamera1stPerson
{
	// ! This is just a prototype functionality for binocs, should be moved to sightscomponent when possible
	
	protected float m_fBinocFOV;
	
	protected SCR_BinocularsComponent m_binoculars;
	protected SCR_2DOpticsComponent m_Optics;
	protected IEntity m_ePlayer;

	//-----------------------------------------------------------------------------
	// constructor
	void CharacterCameraBinoculars(CameraHandlerComponent pCameraHandler)
	{
	}
	
	//-----------------------------------------------------------------------------
	override void OnActivate(ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult)
	{
		super.OnActivate(pPrevCamera, pPrevCameraResult);
		
		m_ePlayer = SCR_PlayerController.GetLocalControlledEntity();
		if (!m_ePlayer)
			return;
		
		m_Optics = FindOpticsComponent(m_ePlayer);
		
		if (!m_Optics)
		{
			Print("Optics wasn't found in binoculars!", LogLevel.WARNING);
			return;
		}
		
		// Getting optics variables  
		m_fBinocFOV = m_Optics.GetFovZoomed();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Find optics component in binocular
	//! \param owner
	//! \return found 2D optics component, null if not found
	protected SCR_2DOpticsComponent FindOpticsComponent(IEntity owner)
	{
		// Finding gadget manager 
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.Cast( m_ePlayer.FindComponent(SCR_GadgetManagerComponent) );
		if (!gadgetManager)
			return null;
		
		// Getting binoculars componenet 
		SCR_BinocularsComponent binoculars = SCR_BinocularsComponent.Cast(gadgetManager.GetHeldGadgetComponent());
		if (!binoculars)
			return null;
		
		IEntity entityBinocular = binoculars.GetOwner();
		if (!entityBinocular)
			return null;
		
		// Getting optics component 
		SCR_2DOpticsComponent optic = SCR_2DOpticsComponent.Cast(entityBinocular.FindComponent(SCR_2DOpticsComponent));
		
		return optic;
	}
	
	//-----------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		super.OnUpdate(pDt, pOutResult);

		pOutResult.m_fFOV = m_fBinocFOV;
		pOutResult.m_bBlendFOV = false;
		
		if (m_pCompartmentAccess.IsInCompartment())
			pOutResult.m_fUseHeading = 0;
		
		// Near plane
		if (m_Optics)
			pOutResult.m_fNearPlane = m_Optics.GetNearPlane();
		else 
		{
			pOutResult.m_fNearPlane = 0.05;
		}
	}
}
