class SCR_ScreenEffectsManager : SCR_InfoDisplayExtended
{
	// Pointer for getting SCR_ScreenEffectsManager
	private static SCR_ScreenEffectsManager s_pScreenEffectsManager;

	// Owner data
	protected ChimeraCharacter 						m_pCharacterEntity;

	//------------------------------------------------------------------------------------------------
	//! Static GetScreenEffectsDisplay will return this regardless of whether SCR_ScreenEffectsManager is running
	static SCR_ScreenEffectsManager GetScreenEffectsDisplay()
	{
		return s_pScreenEffectsManager;
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		s_pScreenEffectsManager = this;

		GetGame().OnUserSettingsChangedInvoker().Insert(SettingsChanged);
		SettingsChanged();

		// Clear all effects that are applied otherwise
		ManagerClearEffects();
	}

	//------------------------------------------------------------------------------------------------
	protected void SettingsChanged()
	{
		array<BaseInfoDisplay> infoDisplays = {};
		GetInfoDisplays(infoDisplays);
		foreach (BaseInfoDisplay baseInfoDisplays: infoDisplays)
		{
			SCR_BaseScreenEffect screenEffect = SCR_BaseScreenEffect.Cast(baseInfoDisplays);
			if (screenEffect)
			{
				if (!screenEffect.GetRootWidget())
					screenEffect.SetRootWidget(GetRootWidget());
				
				screenEffect.SettingsChanged();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
 	override void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
		if (to)
			m_pCharacterEntity = ChimeraCharacter.Cast(to);
	}

	//------------------------------------------------------------------------------------------------
	override event void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (!GetGame().GetCameraManager())
			return;
		
		CameraBase currentCamera = GetGame().GetCameraManager().CurrentCamera();
		PlayerCamera playerCamera = GetGame().GetPlayerController().GetPlayerCamera();
		if (!playerCamera || currentCamera != playerCamera)
			m_wRoot.SetVisible(false);
		else
			m_wRoot.SetVisible(true);
		
		array<BaseInfoDisplay> infoDisplays = {};
		GetInfoDisplays(infoDisplays);
		foreach (BaseInfoDisplay baseInfoDisplays: infoDisplays)
		{
			SCR_BaseScreenEffect screenEffect = SCR_BaseScreenEffect.Cast(baseInfoDisplays);
			if (screenEffect)
				screenEffect.UpdateEffect(timeSlice);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Call ClearEffects() in every child inheriting from SCR_BaseScreenEffect
	protected void ManagerClearEffects()
	{
		array<BaseInfoDisplay> infoDisplays = {};
		GetInfoDisplays(infoDisplays);
		foreach (BaseInfoDisplay baseInfoDisplays: infoDisplays)
		{
			SCR_BaseScreenEffect screenEffect = SCR_BaseScreenEffect.Cast(baseInfoDisplays);
			if (screenEffect)
				screenEffect.ClearEffects();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return the effect is it exist
	BaseInfoDisplay GetEffect(typename effectType)
	{
		array<BaseInfoDisplay> infoDisplays = {};
		GetInfoDisplays(infoDisplays);
		
		foreach (BaseInfoDisplay baseInfoDisplays: infoDisplays)
		{
			if (baseInfoDisplays.Type() == effectType)
				return baseInfoDisplays;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override event void DisplayStopDraw(IEntity owner)
	{
		ManagerClearEffects();
		GetGame().OnUserSettingsChangedInvoker().Remove(SettingsChanged);

		s_pScreenEffectsManager = null;
	}
};