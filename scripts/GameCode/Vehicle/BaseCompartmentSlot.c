class BaseCompartmentSlot : ExtBaseCompartmentSlot
{
	[Attribute("0", desc: "Switching seats is allowed only between compartments with matching section")]
	protected int m_iCompartmentSection;

	[Attribute("0", desc: "Set whether compartment is water tight. People inside may drown if false")]
	protected bool m_bIsWaterTight;

	[Attribute(desc: "Contains Default Prefab of character to be spawned into compartment", params: "et")]
	protected ref SCR_DefaultOccupantData m_DefaultOccupantData;
	
	[Attribute(desc: "Contains parameters for executing screenshake upon crashing vehicle for passengers", params: "et")]
	protected ref SCR_BaseScreenShakeData m_ScreenShakeData;
	
	[Attribute("-89.9", desc: "Aim limit angle override (in degrees) when free-looking down", params: "-89.9 0.05 0.05")]
	protected float m_fFreelookAimLimitOverrideDown;

	[Attribute("89.9", desc: "Aim limit angle override (in degrees) when free-looking up", params: "0.05 89.9 0.05")]
	protected float m_fFreelookAimLimitOverrideUp;

	[Attribute("-160.0", desc: "Aim limit angle override (in degrees) when free-looking left", params: "-160.0 0.05 0.05")]
	protected float m_fFreelookAimLimitOverrideLeft;

	[Attribute("160.0", desc: "Aim limit angle override (in degrees) when free-looking right", params: "0.05 160.0 0.05")]
	protected float m_fFreelookAimLimitOverrideRight;

	[Attribute("-1 1 1", desc: "First person camera local space offset scale from Neck1 bone")]
	protected vector m_vFreelookCameraNeckOffsetScale;

	[Attribute("1.0", desc: "Extent to which body should account for turret traverse.\nThe higher, the more neck will rotate around turret pivot\nExamples: BTR-70 neck does not pivot, tripod and M151 turret pivots")]
	protected float m_fFreelookCameraNeckFollowTraverse;	
	
	[Attribute("0.5", desc: "Chance the occupant will get ejected upon EjectOccupant being called.\nOnly applies to compartments without roof or open doors unless forced")]
	protected float m_fRandomEjectionChance;

	static const vector SPAWN_IN_VEHICLE_OFFSET = Vector(0, 250, 0); //~ Offset added when characters are spawned to add to vehicle. To make sure they are close and streamed but not on the vehicle as this would kill them

	protected bool m_bCompartmentAccessible = true;

	//------------------------------------------------------------------------------------------------
	override void DebugDrawPosition()
	{
		Color c = Color.FromInt(Color.WHITE);
		if (!m_bCompartmentAccessible)
			c = Color.FromInt(Color.BLACK);
		vector pos = GetPosition();
		if (pos != vector.Zero)
			Shape.CreateCylinder(c.PackToInt(), ShapeFlags.ONCE, GetPosition(), 0.05, 5);
	}

	//------------------------------------------------------------------------------------------------
	bool IsCompartmentAccessible()
	{
		return m_bCompartmentAccessible;
	}

	//------------------------------------------------------------------------------------------------
	void SetCompartmentAccessible(bool val)
	{
		m_bCompartmentAccessible = val;
	}

	//------------------------------------------------------------------------------------------------
	//! Switching seats is allowed only between compartments with matching area
	int GetCompartmentSection()
	{
		return m_iCompartmentSection;
	}

	//------------------------------------------------------------------------------------------------
	//!
	bool GetIsWaterTight()
	{
		return m_bIsWaterTight;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get vehicle this slot belongs to.
	May differ from GetOwner() if the slot is placed in vehicle's child entity, e.g., truck's cargo frame.
	\return Vehicle entity
	*/
	IEntity GetVehicle()
	{
		IEntity owner = GetOwner();
		IEntity vehicle;
		while (owner)
		{
			if (owner.FindComponent(BaseCompartmentManagerComponent))
				vehicle = owner;

			owner = owner.GetParent();
		}
		return vehicle;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get vehicle this slot belongs to.
	May differ from GetOwner() if the slot is placed in vehicle's child entity, e.g., truck's cargo frame.
	\param[out] compartmentID Variable to be filled with ID of the compartment relative to the vehicle. May differ from GetCompartmentSlotID() which returns only ID within the entity the slot belong sto.
	\return Vehicle entity
	*/
	IEntity GetVehicle(out int compartmentID)
	{
		IEntity owner = GetOwner();
		IEntity vehicle;
		Managed component, componentCandidate;
		while (owner)
		{
			componentCandidate = owner.FindComponent(BaseCompartmentManagerComponent);
			if (componentCandidate)
			{
				vehicle = owner;
				component = componentCandidate;
			}

			owner = owner.GetParent();
		}
		if (vehicle)
		{
			BaseCompartmentManagerComponent manager = BaseCompartmentManagerComponent.Cast(component);
			array<BaseCompartmentSlot> compartments = {};
			manager.GetCompartments(compartments);
			compartmentID = compartments.Find(this);
		}
		return vehicle;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get if compartment is occupied
	\return returns true if occupied else returns false
	*/
	bool IsOccupied()
	{
		return GetOccupant() != null;
	}

	//------------------------------------------------------------------------------------------------
	void DamageOccupant(float damage, EDamageType damageType, notnull Instigator instigator, bool damageWhileGetIn = true, bool damageWhileGetOut = true)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOccupant());
		if (!character)
			return;

		RplComponent rpl = character.GetRplComponent();
		if (rpl && rpl.IsProxy())
			return;

		// Ignore characters that only began to get in the vehicle
		CompartmentAccessComponent access = character.GetCompartmentAccessComponent();
		if (!damageWhileGetIn && access && access.IsGettingIn())
			return;

		if (!damageWhileGetOut && access && access.IsGettingOut())
			return;

		SCR_DamageManagerComponent damageManager = character.GetDamageManager();
		if (damageManager)
			damageManager.DamageRandomHitZones(damage, damageType, instigator);
	}
	
	//------------------------------------------------------------------------------------------------
	void ScreenShakeOccupant(float damage)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOccupant());
		if (!character)
			return;
		
		if (character != SCR_PlayerController.GetLocalControlledEntity())
			return;

		SCR_DamageManagerComponent damageManager = character.GetDamageManager();
		if (!damageManager)
			return;
		
		float inTime, sustainTime, outTime, maxScreenShakeHealthThreshold, linearMagnitude, angularMagnitude;
		m_ScreenShakeData.GetScreenShakeData(inTime, sustainTime, outTime, maxScreenShakeHealthThreshold, linearMagnitude, angularMagnitude);
		
		float maxScreenShake = damageManager.GetMaxHealth() * maxScreenShakeHealthThreshold;
		if (maxScreenShake == 0)
			return;
		
		SCR_CameraShakeManagerComponent.AddCameraShake(Math.Lerp(0, linearMagnitude, damage / maxScreenShake), Math.Lerp(0, angularMagnitude, damage / maxScreenShake), inTime, sustainTime, outTime);
	}

	//------------------------------------------------------------------------------------------------
	void KillOccupant(notnull Instigator instigator, bool eject = false, bool gettingIn = false, bool gettingOut = false)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOccupant());
		if (!character)
			return;

		SCR_DamageManagerComponent damageManager = character.GetDamageManager();
		if (!damageManager)
			return;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;

		CompartmentAccessComponent access = character.GetCompartmentAccessComponent();
		if (!gettingIn && access && access.IsGettingIn())
			return;

		if (!gettingOut && access && access.IsGettingOut())
			return;

		if (eject && access)
		{
			access.GetOutVehicle(EGetOutType.TELEPORT, -1, ECloseDoorAfterActions.INVALID, false);
			// because person needs to be out of vehicle for sure
			GetGame().GetCallqueue().CallLater(damageManager.Kill, 1, false, instigator);
		}
		else
		{
			damageManager.Kill(instigator);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Force eject occupant from this compartment
	//! \param[in] forceEject Force occupant out of compartment regardless of whether it makes sense realistically.
	//! \param[in] ejectUnconsciously Force ejected character to be unconscious
	//! \param[out] ejectedImmediately returns true if this client was capable of ejecting character that is in this compartment
	//! \param[in] ejectOnTheSpot if character should be ejected in the position where he currently is
	//! \return true if character was ejected
	bool EjectOccupant(bool forceEject = false, bool ejectUnconsciously = false, out bool ejectedImmediately = false, bool ejectOnTheSpot = false)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOccupant());
		if (!character)
			return false;

		RplComponent rpl = character.GetRplComponent();
		if (rpl && rpl.IsProxy())
			return false;
		
		// Ignore characters that only began to get in the vehicle
		SCR_CompartmentAccessComponent access = SCR_CompartmentAccessComponent.Cast(character.GetCompartmentAccessComponent());
		if (!access)
			return false;

		int nearestDoorInd;
		if (!ejectOnTheSpot)
		{
			nearestDoorInd = PickDoorIndexForPoint(character.GetOrigin());
			BaseCompartmentManagerComponent compartmentManager = GetManager();
			if (compartmentManager)
				ejectOnTheSpot = compartmentManager.IsDoorFake(nearestDoorInd);
		}
		
		if (ejectUnconsciously)
		{
			SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
			if (damageMan)
				damageMan.ForceUnconsciousness(damageMan.GetResilienceHitZone().GetDamageStateThreshold(ECharacterDamageState.STATE3));
		}
		
		if (forceEject || GetManager().IsDoorOpen(nearestDoorInd) || ShouldCharactersFallOutWhenFlipped())
		{
			if (!ejectOnTheSpot)
			{
				ejectedImmediately = access.GetOutVehicle(EGetOutType.TELEPORT, nearestDoorInd, ECloseDoorAfterActions.INVALID, false);
			}
			else
			{
				vector mat[4];
				character.GetTransform(mat);
				mat[3] = character.AimingPosition();//dont use root position as it may be f.e. under the vehicle floor

				ejectedImmediately = access.GetOutVehicle_NoDoor(mat, ejectUnconsciously, false);
			}

			if (!ejectedImmediately)
				access.AskOwnerToGetOutFromVehicle(EGetOutType.TELEPORT, nearestDoorInd, ECloseDoorAfterActions.INVALID, false, ejectOnTheSpot);

			return true;
		}
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get default occupent prefab data
	\return Default occupant data
	*/
	SCR_DefaultOccupantData GetDefaultOccupantData()
	{
		return m_DefaultOccupantData;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get default occupent prefab ResourceName
	\return Default occupant prefab
	*/
	ResourceName GetDefaultOccupantPrefab(bool checkIfValid = true)
	{
		if (!m_DefaultOccupantData || (checkIfValid && !m_DefaultOccupantData.IsValid()))
			return string.Empty;

		return SCR_EditableEntityComponentClass.GetRandomVariant(m_DefaultOccupantData.GetDefaultOccupantPrefab());
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Spawn default character within the compartment (Server only)
	Default characters are defined on the CompartmentSlot
	\param[in,out] group if Left empty it will create a new group and place the character in it and return it. Else it will place the character in the given group
	\param groupPrefabGroup prefab, Generally want to keep it as default value as faction and usch is set automaticly
	\return Returns spawned character
	*/
	IEntity SpawnDefaultCharacterInCompartment(inout AIGroup group, ResourceName groupPrefab = "{000CD338713F2B5A}Prefabs/AI/Groups/Group_Base.et")
	{
		return SpawnCharacterInCompartment(GetDefaultOccupantPrefab(), group, groupPrefab);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Spawn character within the compartment (Server only)
	\param characterPrefab Prefab to spawn in compartment
	\param[in,out] group if Left empty it will create a new group and place the character in it and return it. Else it will place the character in the given group
	\param createGroupForCharacter If a group should be created for the spawned character
	\return Returns spawned character
	*/
	IEntity SpawnCharacterInCompartment(ResourceName characterPrefab, inout AIGroup group, ResourceName groupPrefab = "{000CD338713F2B5A}Prefabs/AI/Groups/Group_Base.et")
	{
		if (characterPrefab.IsEmpty() || !IsCompartmentAccessible() || GetOccupant() != null)
			return null;

		//~ Spawn at vehicle position
		EntitySpawnParams params = new EntitySpawnParams();
		GetOwner().GetTransform(params.Transform);

		ChimeraWorld world = GetGame().GetWorld();

		//~ Spawn characters above the vehicle to make sure physics do not interact with the character and they die on contact of the vehicle
		params.Transform[3] + SPAWN_IN_VEHICLE_OFFSET;

		IEntity spawnedCharacter = GetGame().SpawnEntityPrefab(Resource.Load(characterPrefab), null, params);
		if (!spawnedCharacter)
		{
			Print(string.Format("'BaseCompartmentSlot' Failed to spawn character in compartment. Check if ResourceName is correct! Path: '%1'", characterPrefab), LogLevel.ERROR);
			return null;
		}

		AIControlComponent agentControlComponent = AIControlComponent.Cast(spawnedCharacter.FindComponent(AIControlComponent));
		if (!agentControlComponent)
		{
			Print("'BaseCompartmentSlot' Could not get AIControlComponent from spawned character, so entity is deleted!", LogLevel.ERROR);
			delete spawnedCharacter;
			return null;
		}

		agentControlComponent.ActivateAI();

		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(spawnedCharacter.FindComponent(CompartmentAccessComponent));
		if (!compartmentAccess)
		{
			Print("'BaseCompartmentSlot' Could not get CompartmentAccessComponent from spawned character, so entity is deleted!", LogLevel.ERROR);
			delete spawnedCharacter;
			return null;
		}

		//~ Could not move in compartment so delete
		if (!compartmentAccess.GetInVehicle(GetOwner(), this, true, -1, ECloseDoorAfterActions.INVALID, world.IsGameTimePaused()))
		{
			Print(string.Format("'BaseCompartmentSlot' Trying to spawn character in compartment but it could not be moved into it so character is deleted. Compartment type: %1", typename.EnumToString(ECompartmentType, GetType())), LogLevel.WARNING);
			delete spawnedCharacter;
			return null;
		}

		//~ Create new group
		if (!group)
		{
			IEntity groupEntity = GetGame().SpawnEntityPrefab(Resource.Load(groupPrefab));
			if (!groupEntity)
			{
				Print("'BaseCompartmentSlot' Could not create group for spawned character. Most likly incorrect group prefab given!", LogLevel.ERROR);
				return spawnedCharacter;
			}

			group = AIGroup.Cast(groupEntity);
			if (!group)
			{
				Print("'BaseCompartmentSlot' Could not create group for spawned character as spawned group prefab is missing AIGroup component!", LogLevel.ERROR);
				delete groupEntity;
				return spawnedCharacter;
			}

			//~ Set group's faction
			SCR_AIGroup groupScripted = SCR_AIGroup.Cast(group);
			if (groupScripted)
			{
				FactionAffiliationComponent factionComponent = FactionAffiliationComponent.Cast(spawnedCharacter.FindComponent(FactionAffiliationComponent));
				if (factionComponent)
				{
					Faction faction = factionComponent.GetAffiliatedFaction();
					if (faction)
						groupScripted.InitFactionKey(faction.GetFactionKey());
				}
			}
		}

		//~ Add the entity to the group
		group.AddAgent(agentControlComponent.GetControlAIAgent());

		return spawnedCharacter;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetFreelookAimLimitOverrideDown()
	{
		return m_fFreelookAimLimitOverrideDown;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetFreelookAimLimitOverrideUp()
	{
		return m_fFreelookAimLimitOverrideUp;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetFreelookAimLimitOverrideLeft()
	{
		return m_fFreelookAimLimitOverrideLeft;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetFreelookAimLimitOverrideRight()
	{
		return m_fFreelookAimLimitOverrideRight;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetFreelookCameraNeckOffsetScale()
	{
		return m_vFreelookCameraNeckOffsetScale;
	}

	//------------------------------------------------------------------------------------------------
	float GetFreelookCameraNeckFollowTraverse()
	{
		return m_fFreelookCameraNeckFollowTraverse;
	}	
	
	//------------------------------------------------------------------------------------------------
	float GetRandomEjectionChance()
	{
		return m_fRandomEjectionChance;
	}
}

[BaseContainerProps(), BaseContainerCustomTitleField("m_sDefaultOccupantPrefab")]
class SCR_DefaultOccupantData
{
	[Attribute(desc: "Default Prefab of character to be spawned into compartment", params: "et")]
	protected ResourceName m_sDefaultOccupantPrefab;

	[Attribute("1", desc: "This entry will be considered empty and is ignored if flase")]
	protected bool m_bEnabled;

	ResourceName GetDefaultOccupantPrefab()
	{
		return m_sDefaultOccupantPrefab;
	}

	bool IsValid()
	{
		return m_bEnabled && !m_sDefaultOccupantPrefab.IsEmpty();
	}
}

[BaseContainerProps(configRoot: true)]
class SCR_BaseScreenShakeData
{
	//ScreenShake parameters
	[Attribute(defvalue: "0.05", desc: "Screen shake effect fade in duration")]
	protected float m_fInTime;
	
	[Attribute(defvalue: "0.05", desc: "Screen shake effect peak intensity sustain duration")]
	protected float m_fSustainTime;
	
	[Attribute(defvalue: "0.01", desc: "Screen shake effect fade out duration")]
	protected float m_fOutTime;
	
	[Attribute(defvalue: "0.5", desc: "At this health (scaled) the maximum shake effect will occur")]
	protected float m_fMaxScreenShakeHealthThreshold;
	
	[Attribute(defvalue: "7", desc: "Magnitude of linear (positional change) shake")]
	protected float m_fLinearMagnitude;
	
	[Attribute(defvalue: "5", desc: "Magnitude of angular (rotational change) shake")]
	protected float m_fAngularMagnitude;

	void GetScreenShakeData(out float inTime, out float sustainTime, out float outTime, out float maxScreenShakeHealthThreshold, out float linearMagnitude, out float angularMagnitude)
	{
		inTime = m_fInTime;
		sustainTime = m_fSustainTime;
		outTime = m_fOutTime;
		maxScreenShakeHealthThreshold = m_fMaxScreenShakeHealthThreshold;
		linearMagnitude = m_fLinearMagnitude;
		angularMagnitude = m_fAngularMagnitude;
	}
}
