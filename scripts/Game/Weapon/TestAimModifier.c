class TestAimModifier: ScriptedWeaponAimModifier
{
	//------------------------------------------------------------------------------------------------
	private void Register(SCR_DebugMenuID id, string name, bool angles = false)
	{
		const string angleLimits = "-180, 180, 0, 0.01";
		const string linearLimits = "-10, 10, 0, 0.0001";
	
		if (angles)
			DiagMenu.RegisterRange(id, "", name, "TestAimModifier", angleLimits);
		else
			DiagMenu.RegisterRange(id, "", name, "TestAimModifier", linearLimits);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnInit(IEntity weaponEnt) 
	{
		// Reg menu
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_MENU, "TestAimModifier","Weapons");
		
		// Linear 
		{
			Register(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_LIN_X, "Linear X");
			Register(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_LIN_Y, "Linear Y");
			Register(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_LIN_Z, "Linear Z");
		}
		// Angular 
		{
			Register(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_ANG_X, "Angular X", true);
			Register(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_ANG_Y, "Angular Y", true);
			Register(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_ANG_Z, "Angular Z", true);
		}
		// Turn
		{
			Register(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_TURN_X, "Turn X", true);
			Register(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_TURN_Y, "Turn Y", true);
		}
		
		// Reset
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_RESET, "", "Reset aim mod", "TestAimModifier");
	}
	
	//------------------------------------------------------------------------------------------------
	private void SetLinear(vector v)
	{
		DiagMenu.SetRangeValue(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_LIN_X, v[0]);
		DiagMenu.SetRangeValue(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_LIN_Y, v[1]);
		DiagMenu.SetRangeValue(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_LIN_Z, v[2]);
	}
	
	//------------------------------------------------------------------------------------------------
	private vector GetLinear()
	{
		return Vector(
			DiagMenu.GetRangeValue(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_LIN_X),
			DiagMenu.GetRangeValue(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_LIN_Y),
			DiagMenu.GetRangeValue(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_LIN_Z)
		);
	}
	
	//------------------------------------------------------------------------------------------------
	private void SetAngular(vector v)
	{
		DiagMenu.SetRangeValue(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_ANG_X, v[0]);
		DiagMenu.SetRangeValue(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_ANG_Y, v[1]);
		DiagMenu.SetRangeValue(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_ANG_Z, v[2]);
	}
	
	//------------------------------------------------------------------------------------------------
	private vector GetAngular()
	{
		return Vector(
			DiagMenu.GetRangeValue(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_ANG_X),
			DiagMenu.GetRangeValue(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_ANG_Y),
			DiagMenu.GetRangeValue(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_ANG_Z)
		);
	}
	
	//------------------------------------------------------------------------------------------------
	private void SetTurn(vector v)
	{
		DiagMenu.SetRangeValue(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_TURN_X, v[0]);
		DiagMenu.SetRangeValue(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_TURN_Y, v[1]);
	}
	
	//------------------------------------------------------------------------------------------------
	private vector GetTurn()
	{
		return Vector(
			DiagMenu.GetRangeValue(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_TURN_X),
			DiagMenu.GetRangeValue(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_TURN_Y),
			0
		);
	}
	//------------------------------------------------------------------------------------------------
	// called every frame after aim offset is calculated
	protected override void OnCalculate(IEntity owner, WeaponAimModifierContext context, float timeSlice, out vector translation, out vector rotation, out vector turnOffset)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_RESET))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_AIMMODIFIER_RESET, 0);
			SetLinear(vector.Zero);
			SetAngular(vector.Zero);
			SetTurn(vector.Zero);
		}
		
		translation = GetLinear();
		rotation = GetAngular();
		turnOffset = GetTurn();
		
		if (m_weaponManagerComponent)
		{
			BaseWeaponComponent weaponComponent = m_weaponManagerComponent.GetCurrentWeapon();
			if (weaponComponent)
			{
				BaseSightsComponent bsc = weaponComponent.GetSights();
				if (bsc)
				{
					bsc.ForceSightsZeroValue(translation, rotation, turnOffset);
				}
			}
		}
	}
	//------------------------------------------------------------------------------------------------
	// called when weapon is fired
	protected override void OnWeaponFired() 
	{
	}
	
	
	//------------------------------------------------------------------------------------------------
	// called when weapon is activated
	protected override void OnActivated(IEntity weaponOwner)
	{
		m_weaponManagerComponent = BaseWeaponManagerComponent.Cast(weaponOwner.FindComponent(BaseWeaponManagerComponent));	
	}
	
	protected BaseWeaponManagerComponent m_weaponManagerComponent;
};
