class SCR_LogitechLEDManager
{
	static ELogitechLEDState m_CurrentState = ELogitechLEDState.DEFAULT;
	static string s_sReloadKey;
	static string s_sVONDirectKey;
	static string s_sVONChannelKey;

	//------------------------------------------------------------------------------------------------	
	static void ActivateState(ELogitechLEDState state)
	{
		if (state == m_CurrentState)
			return;
		
		bool isLogiLEDEnabled;
		BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_GameplaySettings");
		if (settings)
			settings.Get("m_bLogitechSupport", isLogiLEDEnabled);
		
		if (!isLogiLEDEnabled)
			return;
		
		switch (state)
		{
			case ELogitechLEDState.DEFAULT:
				SetDefault();
				break;
			case ELogitechLEDState.BLEEDING:
				SetEffectBleeding();
				break;
			case ELogitechLEDState.UNCONSCIOUS:
				SetEffectUnconscious();
			case ELogitechLEDState.RELOAD:
				SetEffectReload();
				break;
			case ELogitechLEDState.COMMS:
				SetEffectComms();
				break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!this method changes the default coloring based on the faction
	static void SetFactionColor(Faction currentFaction)
	{
		if (!currentFaction)
			return;
		
		bool isLogiLEDEnabled;
		BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_GameplaySettings");
		if (settings)
			settings.Get("m_bLogitechSupport", isLogiLEDEnabled);
		
		if (!isLogiLEDEnabled)
			return;
		
		LogitechLED logiLed = LogitechLED.Get();
		if (!logiLed)
			return;
		 		
		int r,g,b;
		//colors are hardcoded coz our representation of colors in WB produces very different colors on logitech HW
		switch (currentFaction.GetFactionKey())
		{
			case "FIA":
				g = 30;
				break;
			case "US":
				b = 30;
				break;
			case "USSR":
				r = 30;
				break;
			case "CIV":
				r = 33;
				g = 15;
				b = 30;
				break;
		}
		
		logiLed.SetLighting(r, g, b);
	}
	
	//------------------------------------------------------------------------------------------------
	private static void SetEffectBleeding()
	{
		if (m_CurrentState == ELogitechLEDState.UNCONSCIOUS)
			return;
		
		LogitechLED logiLed = LogitechLED.Get();
		if (!logiLed)
			return;

		IEntity character = GetGame().GetPlayerManager().GetPlayerControlledEntity(SCR_PlayerController.GetLocalPlayerId());
		if (!character)
			return;
		
		SCR_CharacterInventoryStorageComponent storageManager = SCR_CharacterInventoryStorageComponent.Cast(character.FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!storageManager)
			return;
		
		int slotIndex = storageManager.GetFirstSlotIndexOf(ECommonItemType.BANDAGE);
		string quicksSlotActionName;
		
		//yes, has to be done painfully because someone thought its good idea to name actions based on their typical keybinds and not indexes
		switch (slotIndex)
		{
			case 0:
				quicksSlotActionName = "SwitchWeaponCategory1";
				break;
			case 1:
				quicksSlotActionName = "SwitchWeaponCategory2";
				break;
			case 2:
				quicksSlotActionName = "SwitchWeaponCategory3";
				break;
			case 3:
				quicksSlotActionName = "SwitchWeaponCategory4";
				break;
			case 4:
				quicksSlotActionName = "SwitchWeaponCategory5";
				break;
			case 5:
				quicksSlotActionName = "SwitchWeaponCategory6";
				break;
			case 6:
				quicksSlotActionName = "SwitchWeaponCategory7";
				break;
			case 7:
				quicksSlotActionName = "SwitchWeaponCategory8";
				break;
			case 8:
				quicksSlotActionName = "SwitchWeaponCategory9";
				break;
			case 9:
				quicksSlotActionName = "SwitchWeaponCategory0";
				break;
		}
		
		InputBinding binding = GetGame().GetInputManager().CreateUserBinding();
		array<string> bindings = {};
		
		binding.GetBindings(quicksSlotActionName, bindings, EInputDeviceType.KEYBOARD, "click", false);
		
		
		
		//setdefault to reset everything first, to override low prio things like reloading
		SetDefault();
		logiLed.SaveCurrentLighting();
		
		if (!bindings.IsEmpty())
			logiLed.FlashSingleKey(bindings[0], 0, 50, 0, 0, 500);		
		
		logiLed.PulseLighting(100, 0, 0, 0, 1000);
		
		m_CurrentState = ELogitechLEDState.BLEEDING;
	}
	
	//------------------------------------------------------------------------------------------------
	private static void SetDefault()
	{
		LogitechLED logiLed = LogitechLED.Get();
		if (!logiLed)
			return;
		
		if (m_CurrentState == ELogitechLEDState.RELOAD)
		{
			EndEffectReload();
			GetGame().GetCallqueue().Remove(EndEffectReload);
		} 
		else if (m_CurrentState == ELogitechLEDState.COMMS)
		{
			EndEffectComms();
			GetGame().GetCallqueue().Remove(EndEffectComms);
		}
		
		logiLed.StopEffects();
		logiLed.RestoreLighting();
		m_CurrentState = ELogitechLEDState.DEFAULT;
	}
	
	//------------------------------------------------------------------------------------------------
	private static void SetEffectUnconscious()
	{
		LogitechLED logiLed = LogitechLED.Get();
		if (!logiLed)
			return;
		
		//setdefault to reset everything first, to override low prio things like reloading
		SetDefault();
		
		//every temporary effect needs to save current state before they change it so default can retutrn to it
		logiLed.SaveCurrentLighting();
		logiLed.SetLighting(15, 15, 15);
		m_CurrentState = ELogitechLEDState.UNCONSCIOUS;
	}
	
	//------------------------------------------------------------------------------------------------
	private static void SetEffectReload()
	{
		if (m_CurrentState == ELogitechLEDState.BLEEDING || m_CurrentState == ELogitechLEDState.UNCONSCIOUS)
			return;
		
		LogitechLED logiLed = LogitechLED.Get();
		if (!logiLed)
			return;
		
		InputBinding binding = GetGame().GetInputManager().CreateUserBinding();
		array<string> bindings = {};
		
		binding.GetBindings("CharacterReload", bindings, EInputDeviceType.KEYBOARD, "click", false);
		
		if (bindings.IsEmpty())
			return;
		
		s_sReloadKey = bindings[0];
		
		//hardcode for text, change when cimopet responds
		logiLed.SaveCurrentLighting();
		logiLed.FlashSingleKey(s_sReloadKey, 65, 47, 2, 0, 500);
		m_CurrentState = ELogitechLEDState.RELOAD;
		
		GetGame().GetCallqueue().CallLater(EndEffectReload, 3000);
	}
	
	//------------------------------------------------------------------------------------------------
	private static void EndEffectReload()
	{
		if (m_CurrentState != ELogitechLEDState.RELOAD )
			return;
		
		LogitechLED logiLed = LogitechLED.Get();
		if (!logiLed)
			return;

		logiLed.StopEffectsOnKey(s_sReloadKey);
		logiLed.RestoreLighting();
		
		m_CurrentState = ELogitechLEDState.DEFAULT;		
	}
	
	//------------------------------------------------------------------------------------------------
	private static void SetEffectComms()
	{
		if (m_CurrentState == ELogitechLEDState.BLEEDING || m_CurrentState == ELogitechLEDState.UNCONSCIOUS)
			return;
		
		LogitechLED logiLed = LogitechLED.Get();
		if (!logiLed)
			return;

		
		InputBinding binding = GetGame().GetInputManager().CreateUserBinding();
		array<string> bindingsDirect = {};
		array<string> bindingsChannel = {};
		
		binding.GetBindings("VONDirect", bindingsDirect, EInputDeviceType.KEYBOARD, "hold", false);
		binding.GetBindings("VONChannel", bindingsChannel, EInputDeviceType.KEYBOARD, "hold", false);
		
		if (!bindingsDirect.IsEmpty())
			s_sVONDirectKey = bindingsDirect[0];
		
		if (!bindingsChannel.IsEmpty())
			s_sVONChannelKey = bindingsChannel[0];
		
		logiLed.SaveCurrentLighting();
		
		logiLed.FlashSingleKey(s_sVONDirectKey, 65, 47, 2, 0, 500);
		logiLed.FlashSingleKey(s_sVONChannelKey, 65, 47, 2, 0, 500);
		
		m_CurrentState = ELogitechLEDState.COMMS;
		
		GetGame().GetCallqueue().CallLater(EndEffectComms, 3000);
	}
	
	//------------------------------------------------------------------------------------------------
	private static void EndEffectComms()
	{
		if (m_CurrentState != ELogitechLEDState.COMMS )
			return;
		
		LogitechLED logiLed = LogitechLED.Get();
		if (!logiLed)
			return;
		
		logiLed.StopEffectsOnKey(s_sVONDirectKey);
		logiLed.StopEffectsOnKey(s_sVONChannelKey);
		logiLed.RestoreLighting();
		
		m_CurrentState = ELogitechLEDState.DEFAULT;	
	}
}


//in order of priority top to bottom!
enum ELogitechLEDState
{
	DEFAULT,
	UNCONSCIOUS,
	BLEEDING,
	RELOAD,
	COMMS
}