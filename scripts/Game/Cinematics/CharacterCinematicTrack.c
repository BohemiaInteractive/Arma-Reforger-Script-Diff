[CinematicTrackAttribute(name:"Character Track", description:"Allows you to control characters")]
class CharacterCinematicTrack : CinematicTrackBase
{
	
	[Attribute("0.0", params: "0.0 3.0")]
	float m_fMoveSpeed;
	
	[Attribute("1.0", params: "-1.0 1.0")]
	float m_fForwardBackward;
	
	[Attribute("0.0",  params: "-1.0 1.0")]
	float m_fStrafeLeftRight;
	
	[Attribute("0.0", params: "-3.0 3.0")]
	float m_fHeadingAngle;
	
	[Attribute("0", params: "-1.5 1.5")]
	float m_fAimingLeftRight;
	
	[Attribute("0.0", params: "-1.5 1.5")]
	float m_fAimingDownUp;
	
	[Attribute("1.0", params: "0.0 1.0")]
	float m_fStance;
	
	[Attribute("false")]
	bool m_bRollLeft;
	
	[Attribute("false")]
	bool m_bRollRight;
	
	[Attribute("true")]
	bool m_bWeaponRised;
	
	[Attribute("0", params: "0 1")]
	int m_iMuzzle;
	
	[Attribute("1", params: "1 3")]
	int m_iFireMode;
	
	[Attribute("")]
	string m_sWhatToEquip;
	
	[Attribute("")]
	string m_sEntityToAim;
	
	[CinematicEventAttribute()]
	void jump()
	{
		if (m_Character && m_Character_c)
			m_Character_c.SetJump(1.0);
	}
	
	[CinematicEventAttribute()]
	void reload()
	{
		if (m_Character_c)
		{
			m_Character_c.ReloadWeapon();
		}
	}
	
	[CinematicEventAttribute()]
	void equipMuzzle()
	{
		if (m_Character_c)
		{
			m_Character_c.SetMuzzle(m_iMuzzle);
			GetGame().GetCallqueue().CallLater(RefreshWeapon, 100);
		}
	}
	
	[CinematicEventAttribute()]
	void singleShoot()
	{
		if (m_Character_c)
		{
			m_Character_c.SetSafety(false,false);
			m_Character_c.SetMuzzle(m_iMuzzle);
			m_Character_c.SetFireMode(m_iFireMode);
			m_Character_c.SetFireWeaponWanted(1);
					
			GetGame().GetCallqueue().CallLater(RefreshWeapon, 100);
		}
	}
	
	[CinematicEventAttribute()]
	void autoShootStart()
	{
		if (m_Character_c)
		{
			m_Character_c.SetSafety(false,false);
			m_Character_c.SetMuzzle(m_iMuzzle);
			m_Character_c.SetFireMode(m_iFireMode);
			m_Character_c.SetFireWeaponWanted(1);
		}
	}
	
	[CinematicEventAttribute()]
	void autoShootEnd()
	{
		if (m_Character_c)
		{
			m_Character_c.SetSafety(false,false);
			m_Character_c.SetMuzzle(m_iMuzzle);
			m_Character_c.SetFireMode(m_iFireMode);
			m_Character_c.SetFireWeaponWanted(0);
		}
	}
	
	[CinematicEventAttribute()]
	void die()
	{
		if (m_Character_c)
		{
			m_Character_c.ForceDeath();
		}
	}
	
	[CinematicEventAttribute()]
	void throwStart()
	{
		CharacterGrenadeSlotComponent grenade = CharacterGrenadeSlotComponent.Cast(m_Character.FindComponent(CharacterGrenadeSlotComponent));		
		if (m_Character_c && grenade)
		{
			m_Character_c.SelectWeapon(grenade);
			m_Character_c.SetThrow(true, false);
		}
	}
	
	[CinematicEventAttribute()]
	void throwEnd()
	{
		CharacterGrenadeSlotComponent grenade = CharacterGrenadeSlotComponent.Cast(m_Character.FindComponent(CharacterGrenadeSlotComponent));		
		if (m_Character_c && grenade)
		{
			m_Character_c.SelectWeapon(grenade);
			m_Character_c.SetThrow(false, false);
		}
	}
	
	[CinematicEventAttribute()]
	void actionGetInPilot()
	{		
		getIn = true;
		compType = ECompartmentType.PILOT;
		if (m_Character)
			actualWorld.QueryEntitiesBySphere(m_Character.GetOrigin(), 3, PerformActionGetIn);
	}
	
	[CinematicEventAttribute()]
	void actionGetOutPilot()
	{
		getIn = false;
		compType = ECompartmentType.PILOT;
		if (m_Character)
			actualWorld.QueryEntitiesBySphere(m_Character.GetOrigin(), 3, PerformActionGetIn);
	}
	
	[CinematicEventAttribute()]
	void actionGetInCargo()
	{
		getIn = true;
		compType = ECompartmentType.CARGO;
		if (m_Character)
			actualWorld.QueryEntitiesBySphere(m_Character.GetOrigin(), 3, PerformActionGetIn);
	}
	
	[CinematicEventAttribute()]
	void actionGetOutCargo()
	{
		getIn = false;
		compType = ECompartmentType.CARGO;
		if (m_Character)
			actualWorld.QueryEntitiesBySphere(m_Character.GetOrigin(), 3, PerformActionGetIn);
	}
	
	[CinematicEventAttribute()]
	void actionGetInTurret()
	{
		getIn = true;
		compType = ECompartmentType.TURRET;
		if (m_Character)
			actualWorld.QueryEntitiesBySphere(m_Character.GetOrigin(), 3, PerformActionGetIn);
	}
	
	[CinematicEventAttribute()]
	void actionGetOutTurret()
	{
		getIn = false;
		compType = ECompartmentType.TURRET;
		if (m_Character)
			actualWorld.QueryEntitiesBySphere(m_Character.GetOrigin(), 3, PerformActionGetIn);
	}
	
	[CinematicEventAttribute()]
	void actionEquip()
	{
		if (m_Character)
			actualWorld.QueryEntitiesBySphere(m_Character.GetOrigin(), 3, PerformActionEquip);
	}
	
	[CinematicEventAttribute()]
	void actionActivate()
	{
		if (m_Character)
			actualWorld.QueryEntitiesBySphere(m_Character.GetOrigin(), 3, PerformActionActivate);
	}
	
	[CinematicEventAttribute()]
	void actionOpen()
	{
		if (m_Character)
			actualWorld.QueryEntitiesBySphere(m_Character.GetOrigin(), 3, PerformActionOpen);
	}
	
	private World actualWorld;
	private GenericEntity m_Character;
	private CharacterControllerComponent m_Character_c;
	private float previousMoveSpeed;
	private vector originalYawPitchRoll;
	private string m_sLastActionName;
	
	private bool getIn;
	private ECompartmentType compType;
	
	override void OnInit(World world)
	{
		// Find character entity by using name of track
		findCharacter(world);
		
		actualWorld = world;
		
		if (m_Character) {
		
			m_Character_c = CharacterControllerComponent.Cast(m_Character.FindComponent(CharacterControllerComponent));
			
			if (m_Character_c) {
				
				//Getting original heading angle of character
				originalYawPitchRoll = m_Character.GetYawPitchRoll();
			}
		}
	}
	
	void findCharacter(World world)
	{
		// Find particle entity by using name of track
		TStringArray strs = new TStringArray;
		GetTrackName().Split("_", strs, true);
		
		m_Character = GenericEntity.Cast(world.FindEntityByName(strs.Get(0)));
	}
	
	// Weapon need to be refreshed after while to keep possibility to fire again
	void RefreshWeapon()
	{
		if (m_Character_c)
			m_Character_c.SetFireWeaponWanted(0);
	}
	
	
	override void OnApply(float time)
	{
		
		if (m_Character && m_Character_c)
		{
			
			//Dynamic stance
			m_Character_c.SetDynamicStance(m_fStance);
			
			
			//Character animations are not smooth in the interval of (0, 0.5), setting to 0.5 for lower values
			if (m_fMoveSpeed > 0 && m_fMoveSpeed < 0.5)
				m_fMoveSpeed = 0.5;
			else if (m_fMoveSpeed > 2)
				m_bWeaponRised = false;
			
			//This is called only when the moving speed is changing on timeline
			if (previousMoveSpeed != m_fMoveSpeed)
			{					
				vector move = {m_fStrafeLeftRight, 0, m_fForwardBackward};	
				m_Character_c.SetMovement(m_fMoveSpeed, move);
			}
			
			//Checking expresion if moving value is changing on timeline
			previousMoveSpeed = m_fMoveSpeed;
			
			//Heading and Aiming angles
			m_Character_c.SetHeadingAngle( Math.DEG2RAD * originalYawPitchRoll[0] + m_fHeadingAngle, false);
			
			//Aiming angle
			if (m_sEntityToAim == "")
			{
				float x_rad = Math.DEG2RAD * originalYawPitchRoll[0] + m_fHeadingAngle + m_fAimingLeftRight;
				float y_rad = m_fAimingDownUp;
				vector aiming_vector = {x_rad, y_rad, 0};						
				m_Character_c.GetInputContext().SetAimingAngles( aiming_vector );
			}
			else
			{
				GenericEntity target = GenericEntity.Cast(actualWorld.FindEntityByName(m_sEntityToAim));
				if(target)
				{
					vector shooterTransform[4], aimingTransform[4], aimingRotation[3];
					vector direction = vector.Direction(m_Character.GetOrigin(), target.GetOrigin());
					m_Character.GetWorldTransform(shooterTransform);
					vector shooterUp = shooterTransform[1];
					Math3D.DirectionAndUpMatrix(direction,shooterUp,aimingTransform);
					aimingRotation[0] = aimingTransform[0];
					aimingRotation[1] = aimingTransform[1];
					aimingRotation[2] = aimingTransform[2];
					vector resultAngles = Math3D.MatrixToAngles(aimingRotation);
					vector finalRadAngles;
					finalRadAngles[0] = resultAngles[0] * Math.DEG2RAD;
					finalRadAngles[1] = resultAngles[1] * Math.DEG2RAD;
					finalRadAngles[2] = resultAngles[2] * Math.DEG2RAD;
						
					m_Character_c.GetInputContext().SetAimingAngles( finalRadAngles );
				}
							
			}			
			
			//Rolling to left
			if (m_bRollLeft)
				m_Character_c.SetRoll(1);
			else
				m_Character_c.SetRoll(0);
			
			//Rolling to right
			if (m_bRollRight)			
				m_Character_c.SetRoll(2);	
			else
				m_Character_c.SetRoll(0);
			
			//Rising weapon
			if (m_bWeaponRised)
				m_Character_c.SetWeaponRaised(true);
			else
				m_Character_c.SetWeaponRaised(false);
		}
	}
	
	bool PerformActionGetIn(IEntity entity)
	{	
			
	    SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(entity.FindComponent(SCR_BaseCompartmentManagerComponent));		
		if (!compartmentManager)
			return true;		
		array<BaseCompartmentSlot> slots = {};
		
		if (getIn)
			compartmentManager.GetFreeCompartmentsOfType(slots, compType);
		else
			compartmentManager.GetCompartmentsOfType(slots, compType);

		if (!slots.IsEmpty())
		{
		    BaseCompartmentSlot slot = slots[0];			
			BaseUserAction getInAction;
			
			if (getIn)
		    	 getInAction = slot.GetGetInAction();
			else
				 getInAction = slot.GetGetOutAction();
			
		    ActionsPerformerComponent actionsPerformer = ActionsPerformerComponent.Cast(m_Character.FindComponent(ActionsPerformerComponent));
			actionsPerformer.PerformAction(getInAction);
			
			return false;
		}
	
	    return true;
	}
	
	bool PerformActionEquip(IEntity entity)
	{				
	    ActionsManagerComponent actionsManager = ActionsManagerComponent.Cast(entity.FindComponent(ActionsManagerComponent));	
	    if (!actionsManager)
	        return true;
		
		array<BaseUserAction> outActions = {};		
	    actionsManager.GetActionsList(outActions);
		
		foreach (BaseUserAction openAction : outActions)
		{					
			if (openAction.GetOwner().GetName() == m_sWhatToEquip && openAction.GetActionName().Contains("Equip"))
			{						
				ActionsPerformerComponent actionsPerformer = ActionsPerformerComponent.Cast(m_Character.FindComponent(ActionsPerformerComponent));
	    		actionsPerformer.PerformAction(openAction);		
				return true;
			}
		}
	
	    return true;
	}
	
	bool PerformActionActivate(IEntity entity)
	{				
	    ActionsManagerComponent actionsManager = ActionsManagerComponent.Cast(entity.FindComponent(ActionsManagerComponent));	
	    if (!actionsManager)
	        return true;
		
		array<BaseUserAction> outActions = {};		
	    actionsManager.GetActionsList(outActions);
		
		foreach (BaseUserAction openAction : outActions)
		{					
			if (openAction.GetOwner().GetName() == m_sWhatToEquip && openAction.GetActionName().Contains("Activate"))
			{						
				ActionsPerformerComponent actionsPerformer = ActionsPerformerComponent.Cast(m_Character.FindComponent(ActionsPerformerComponent));
	    		actionsPerformer.PerformAction(openAction);		
				return true;
			}
		}
	
	    return true;
	}
	
	bool PerformActionOpen(IEntity entity)
	{	
			
	    ActionsManagerComponent actionsManager = ActionsManagerComponent.Cast(entity.FindComponent(ActionsManagerComponent));	
	    if (!actionsManager)
	        return true;
		
		array<BaseUserAction> outActions = {};		
	    actionsManager.GetActionsList(outActions);
		
		foreach (BaseUserAction openAction : outActions)
		{
			
			DoorComponent door = DoorComponent.Cast(openAction.GetOwner().FindComponent(DoorComponent));
			if (!door)
				continue;
			
			if (openAction.GetActionName().Contains("Open"))
			{		
				ActionsPerformerComponent actionsPerformer = ActionsPerformerComponent.Cast(m_Character.FindComponent(ActionsPerformerComponent));
	    		actionsPerformer.PerformAction(openAction);		
				return false;
			}
		}
	
	    return true;
	}
}