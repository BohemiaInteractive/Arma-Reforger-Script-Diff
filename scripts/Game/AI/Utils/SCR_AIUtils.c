//------------------------------------------------------------------------------------------------------------------------------------------------------------
// Helper functions regarding ports of nodes
class SCR_AINodePortsHelpers
{
	// gets a string from value given in port
	// basically workaround that we can't cast void from GetVariableIn into type of that variable
	//------------------------------------------------------------------------------------------------
	static string GetStringFromPort(Node node, string port)
	{
		const string UNASSIGNED_VARIABLE = "Unassigned";
		if ( node.GetVariableType(true, port) == string )
		{
			string value;
			if(!node.GetVariableIn(port, value))
				value = UNASSIGNED_VARIABLE;
			return value;		
		}
		else if ( node.GetVariableType(true, port) == int )
		{
			int value;
			if(!node.GetVariableIn(port, value))
				return UNASSIGNED_VARIABLE;
			return value.ToString();
		}
		else if ( node.GetVariableType(true, port) == float )
		{
			float value;
			if(!node.GetVariableIn(port, value))
				return UNASSIGNED_VARIABLE;
			return value.ToString();
		}
		else if ( node.GetVariableType(true, port) == bool )
		{
			bool value;
			if(!node.GetVariableIn(port, value))
				return UNASSIGNED_VARIABLE;
			return value.ToString();
		}
		else if ( node.GetVariableType(true, port) == vector )
		{
			vector value;
			if(!node.GetVariableIn(port, value))
				return UNASSIGNED_VARIABLE;
			return value.ToString();
		}
		else if ( node.GetVariableType(true, port).IsInherited(Managed) )
		{
			Managed value;
			if(!node.GetVariableIn(port, value))
				return UNASSIGNED_VARIABLE;
			if (value)
				return value.ToString();
			else 
				return "NULL";
		}
		return "";
	};
	
	// merges two TStringsArray into one
	//------------------------------------------------------------------------------------------------------------------------------------------------------------
	static TStringArray MergeTwoArrays(TStringArray first, TStringArray second)
	{
		ref TStringArray merged = {};
		merged.Copy(first);
		foreach (string s: second)
			merged.Insert(s);
		return merged;
	}
}

// use this for generating random numbers for AI, no need to create another instance
//------------------------------------------------------------------------------------------------------------------------------------------------------------
static ref RandomGenerator s_AIRandomGenerator = new RandomGenerator;

//------------------------------------------------------------------------------------------------------------------------------------------------------------
// AI stance
class SCR_AIStanceHandling
{
	// use this for interpretting relevant character stance based on threat level
	//------------------------------------------------------------------------------------------------
	static ECharacterStance GetStanceFromThreat(EAIThreatState threatState)
	{
		switch (threatState)
		{
			case EAIThreatState.THREATENED: return ECharacterStance.PRONE;
			case EAIThreatState.ALERTED: return ECharacterStance.CROUCH;
			case EAIThreatState.VIGILANT: return ECharacterStance.STAND;
			case EAIThreatState.SAFE: return ECharacterStance.STAND;
			default: return ECharacterStance.STAND;
		}
		return ECharacterStance.STAND;
	}
	
	//------------------------------------------------------------------------------------------------
	[Obsolete("Use SetStance without Info Component")]
	static void SetStance(SCR_AIInfoComponent infoComp, CharacterControllerComponent charContrComp, ECharacterStance stance) 
	{
		charContrComp.SetStanceChange(ConvertStanceToStanceChange(stance));
	}
	
	static void SetStance(notnull CharacterControllerComponent charContrComp, ECharacterStance stance) 
	{
		charContrComp.SetStanceChange(ConvertStanceToStanceChange(stance));
	}
	
	// use this to convert stance enum to int
	//------------------------------------------------------------------------------------------------
	static int ConvertStanceToStanceChange(ECharacterStance stance)
	{
		switch (stance)
		{
			case ECharacterStance.STAND: return ECharacterStanceChange.STANCECHANGE_TOERECTED;
			case ECharacterStance.CROUCH: return ECharacterStanceChange.STANCECHANGE_TOCROUCH;
			case ECharacterStance.PRONE: return ECharacterStanceChange.STANCECHANGE_TOPRONE;
		}
		return 0;
	}
}

// use this to find all components in all children of an entity
//------------------------------------------------------------------------------------------------------------------------------------------------------------
static bool FindComponentsInAllChildren(typename typeName, IEntity parent, bool findFirst, int depth, int maxDepth, notnull inout array<Managed> outComponents)
{
	if (depth > maxDepth)
		return false;

	IEntity child = parent.GetChildren();
	
	while (child)
	{
		if (FindComponentsInAllChildren(typeName, child, findFirst, depth + 1, maxDepth, outComponents) && findFirst)
			return true;
		
		child = child.GetSibling();
	}
	
	if (parent)
	{
		Managed component = parent.FindComponent(typeName);
		if (component)
		{
			outComponents.Insert(component);
			return true;
		}
	}
	
	return false;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------
// Compartments on vehicles
class SCR_AICompartmentHandling
{
	//--------------------------------------------------------------------------------------------
	static ECompartmentType CompartmentClassToType(typename type)
	{
		if (type.IsInherited(CargoCompartmentSlot))
			return ECompartmentType.CARGO;
		else if (type.IsInherited(PilotCompartmentSlot))
			return ECompartmentType.PILOT;
		else if (type.IsInherited(TurretCompartmentSlot))
			return ECompartmentType.TURRET;
		return -1;
	}
	
	//--------------------------------------------------------------------------------------------
	static bool FindAvailableCompartmentInVehicles (array<IEntity> vehicles, ECompartmentType roleInVehicle, out BaseCompartmentSlot compartmentOut, out IEntity vehicleOut)
	{
		foreach (IEntity vehicle: vehicles)
		{
			if (!vehicle)
				continue;
	
			BaseCompartmentManagerComponent compartmentMan = BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(BaseCompartmentManagerComponent));
			if (!Vehicle.Cast(vehicle) || !compartmentMan)
				return false;
			ref array<BaseCompartmentSlot> compartments = {};
			compartmentMan.GetCompartments(compartments);
			foreach (BaseCompartmentSlot compartment: compartments)
			{
				if (SCR_AICompartmentHandling.CompartmentClassToType(compartment.Type()) == roleInVehicle)
				{
					if (!compartment.GetOccupant() && compartment.IsCompartmentAccessible() && !compartment.IsReserved())
					{
						compartmentOut = compartment;
						vehicleOut = vehicle;
						return true;
					}
				}
			}
		}
		return false;
	}
	
	//--------------------------------------------------------------------------------------------
	static bool IsInCompartment(notnull AIAgent agent)
	{
		IEntity controlledEntity = agent.GetControlledEntity();
		if (!controlledEntity)
			return false;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(controlledEntity);
		if (!character)
			return false;
		
		return character.IsInVehicle();
	}
	
	//--------------------------------------------------------------------------------------------
	static IEntity GetAgentVehicleAndCompartment(notnull AIAgent agent, out BaseCompartmentSlot outCompartmentSlot)
	{
		outCompartmentSlot = null;
		
		IEntity controlledEntity = agent.GetControlledEntity();
		if (!controlledEntity)
			return null;
		
		CompartmentAccessComponent compartmentAccessComp = CompartmentAccessComponent.Cast(controlledEntity.FindComponent(CompartmentAccessComponent));
		if (!compartmentAccessComp)
			return null;
		
		BaseCompartmentSlot slot = compartmentAccessComp.GetCompartment();
		
		if (!slot)
			return null;
		
		outCompartmentSlot = slot;
		return outCompartmentSlot.GetVehicle();
	}
	
	//--------------------------------------------------------------------------------------------
	static int GetAvailableCompartmentCount (notnull IEntity vehicle)
	{
		int availableCompCount;
		BaseCompartmentManagerComponent compartmentMan = BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(BaseCompartmentManagerComponent));
		if (!compartmentMan)
			return 0;
		ref array<BaseCompartmentSlot> compartments = {};
		compartmentMan.GetCompartments(compartments);
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			if (!compartment.GetOccupant() && compartment.IsCompartmentAccessible() && !compartment.IsReserved())
			{
				availableCompCount++;
			}
		}
		return availableCompCount;
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
// Check if vehicle can move or is not on fire
class SCR_AIVehicleUsability
{
	//--------------------------------------------------------------------------------------------
	static void TurnOnVehicleHazardLights(IEntity vehicle)
	{
		BaseLightManagerComponent vehicleLightManager = BaseLightManagerComponent.Cast(vehicle.FindComponent(BaseLightManagerComponent));
		if (vehicleLightManager)
			vehicleLightManager.SetLightsState(ELightType.Hazard, true);
	}
	
	//--------------------------------------------------------------------------------------------
	static bool VehicleCanMove(IEntity vehicle, SCR_DamageManagerComponent damageManager = null)
	{
		if (!vehicle)
			return false;
		
		if (!damageManager)
		{
			damageManager = SCR_DamageManagerComponent.GetDamageManager(vehicle);
			if (!damageManager)
				return false;
		}
		
		return damageManager.GetMovementDamage() < 1;
	}
	
	//--------------------------------------------------------------------------------------------
	static bool VehicleIsOnFire(IEntity vehicle, SCR_DamageManagerComponent damageManager = null)
	{
		if(!vehicle)
			return false;
		
		if (!damageManager)
		{
			damageManager = SCR_DamageManagerComponent.GetDamageManager(vehicle);
			if (!damageManager)
				return false;
		}
		
		return damageManager.IsOnFire(damageManager.GetDefaultHitZone());
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
// Weapon Handling
class SCR_AIWeaponHandling
{
	//------------------------------------------------------------------------------------------------
	static EMuzzleType GetWeaponCurrentMuzzleType(notnull BaseWeaponComponent weapon)
	{
		BaseMuzzleComponent currentMuzzle = weapon.GetCurrentMuzzle();
		if (currentMuzzle)
			return currentMuzzle.GetMuzzleType();
		
		return EMuzzleType.MT_BaseMuzzle;
	}
	
	//------------------------------------------------------------------------------------------------
	static EWeaponType GetWeaponType(notnull BaseWeaponComponent weapon, bool overrideWithMuzzle = false)
	{
		EWeaponType weaponType = weapon.GetWeaponType();
		
		if (!overrideWithMuzzle)
			return weaponType;
		
		EMuzzleType muzzleType = SCR_AIWeaponHandling.GetWeaponCurrentMuzzleType(weapon);
		
		// We treat RPG muzzle as rocket launcher
		if (muzzleType == EMuzzleType.MT_RPGMuzzle)
			weaponType = EWeaponType.WT_ROCKETLAUNCHER;
		
		// We treat UGL muzzle as grenade launcher
		if (muzzleType == EMuzzleType.MT_UGLMuzzle)
			weaponType = EWeaponType.WT_GRENADELAUNCHER;
		
		return weaponType;
	}	
	
	//--------------------------------------------------------------------------------------------
	static int GetCurrentMuzzleId(BaseWeaponManagerComponent weapMgr)
	{
		BaseWeaponComponent weaponComp = weapMgr.GetCurrentWeapon();
		
		if (!weaponComp)
			return -1;
				
		// Find muzzle ID
		array<BaseMuzzleComponent> muzzles = {};
		weaponComp.GetMuzzlesList(muzzles);
		BaseMuzzleComponent currentMuzzle = weaponComp.GetCurrentMuzzle();
		
		if (!currentMuzzle || muzzles.IsEmpty())
			return -1;
		
		int currentMuzzleId = muzzles.Find(currentMuzzle);
		return currentMuzzleId;
	}
	
	//--------------------------------------------------------------------------------------------
	static BaseMagazineComponent GetCurrentMagazineComponent(BaseWeaponManagerComponent weapMgr)
	{
		BaseWeaponComponent weaponComp = weapMgr.GetCurrentWeapon();
		
		if (!weaponComp)
			return null;
		
		BaseMuzzleComponent	currentMuzzle = weaponComp.GetCurrentMuzzle();
		if (!currentMuzzle)
			return null;
		
		return currentMuzzle.GetMagazine();
	}
	
	//--------------------------------------------------------------------------------------------
	//! defvalue - value returned if chamberring of this muzzle is not possible
	static bool IsCurrentMuzzleChambered(BaseWeaponManagerComponent weapMgr, bool defValue = false)
	{
		BaseWeaponComponent weaponComp = weapMgr.GetCurrentWeapon();
		
		if (!weaponComp)
			return false;
		
		BaseMuzzleComponent	currentMuzzle = weaponComp.GetCurrentMuzzle();
		if (!currentMuzzle)
			return false;
		
		if (!currentMuzzle.IsChamberingPossible())
			return defValue;
		
		return currentMuzzle.IsCurrentBarrelChambered();
	}
	
	//--------------------------------------------------------------------------------------------
	//! Although trivial, there are several ways to get current weapon, thus let's keep this function
	static BaseWeaponComponent GetCurrentWeaponComponent(BaseWeaponManagerComponent weapMgr)
	{
		return weapMgr.GetCurrentWeapon();
	}
	
	//--------------------------------------------------------------------------------------------
	static void StartMuzzleSwitch(CharacterControllerComponent controller, int newMuzzleId)
	{
		controller.SetMuzzle(newMuzzleId);
	}
	
	//--------------------------------------------------------------------------------------------
	static void StartMagazineSwitchCharacter(CharacterControllerComponent controller, BaseMagazineComponent newMagazineComp)
	{
		controller.ReloadWeaponWith(newMagazineComp.GetOwner());
	}
	
	//--------------------------------------------------------------------------------------------
	static void StartMagazineSwitchTurret(TurretControllerComponent controller, BaseMagazineComponent newMagazineComp)
	{
		controller.DoReloadWeaponWith(newMagazineComp.GetOwner());
	}
	
	//--------------------------------------------------------------------------------------------
	static void StartWeaponSwitchCharacter(CharacterControllerComponent controller, BaseWeaponComponent newWeaponComp)
	{
		controller.TryEquipRightHandItem(newWeaponComp.GetOwner(), EEquipItemType.EEquipTypeWeapon, false);
	}
	
	//--------------------------------------------------------------------------------------------
	static void StartWeaponSwitchTurret(TurretControllerComponent controller, BaseWeaponComponent newWeaponComp, IEntity turretOperator)
	{
		// Find weapon slot which has this weapon
		IEntity newWeaponEntity = newWeaponComp.GetOwner();
		BaseWeaponManagerComponent weaponMgr = controller.GetWeaponManager();
		if (!weaponMgr)
			return;
		
		WeaponSlotComponent newWeaponSlot;
		array<WeaponSlotComponent> slots = {};
		weaponMgr.GetWeaponsSlots(slots);
		foreach (WeaponSlotComponent slot : slots)
		{
			if (slot.GetWeaponEntity() == newWeaponEntity)
				newWeaponSlot = slot;
		}
		
		if (!newWeaponSlot)
		{
			// There is no slot which has this weapon
			return;
		}
		
		controller.SelectWeapon(turretOperator, newWeaponSlot);
	}
}

class SCR_AIDamageHandling
{
	//---------------------------------------------------------------------------------------------------------
	static bool IsConscious(IEntity entity)
	{
		if (!entity)
			return false;
		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		if (character)
		{	
			CharacterControllerComponent contr = character.GetCharacterController();
			if (!contr)
				return false;
			return contr.GetLifeState() == ECharacterLifeState.ALIVE;
		}
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(entity.FindComponent(DamageManagerComponent));
		if (!damageManager)
			return true;
		return damageManager.GetState() != EDamageState.DESTROYED;
	}
	
	//---------------------------------------------------------------------------------------------------------
	static bool IsAlive(IEntity entity)
	{
		if (!entity)
			return false;
		
		DamageManagerComponent damageManager;
		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		if (character)
			damageManager = character.GetDamageManager();
		else
			damageManager = DamageManagerComponent.Cast(entity.FindComponent(DamageManagerComponent));
		
		if (!damageManager)
			return true;
		
		return damageManager.GetState() != EDamageState.DESTROYED;
	}
	
	// This method abstracts away the internals of damage system for usage in various AI behaviors.
	static bool IsCharacterWounded(IEntity entity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		if (!character)
			return false;
		
		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (!damageManager)
			return false;
		
		return damageManager.IsBleeding();
	}
}

class SCR_AISmartActionHandling
{
	//! Finds first smart action component with given tag on this entity, does not search hierarchy
	static SCR_AISmartActionComponent FindSmartAction(notnull IEntity entity, string tag, bool checkAccessibility)
	{
		array<Managed> components = {};
		entity.FindComponents(SCR_AISmartActionComponent, components);
		
		array<string> tags = {};
		
		foreach (Managed c : components)
		{
			SCR_AISmartActionComponent saComp = SCR_AISmartActionComponent.Cast(c);
			
			if (checkAccessibility && !saComp.IsActionAccessible())
				continue;
			
			tags.Clear();
			saComp.GetTags(tags);
			
			if (tags.Contains(tag))
				return saComp;
		}
		
		return null;
	}
}

class SCR_AIMessageHandling
{
	//------------------------------------------------------------------------------------
	static void SendDismountMessage(notnull AIAgent agent, notnull IEntity vehicleEntity, int soldierId, SCR_AIActivityBase relatedActivity, 
									notnull AICommunicationComponent myComms, string sendFrom = string.Empty)
	{
		float dismountDelay = 0.9 * soldierId;
		
		SCR_AIBoardingParameters bParams = new SCR_AIBoardingParameters();
		SCR_AIMessage_GetOut msg = SCR_AIMessage_GetOut.Create(vehicleEntity, bParams, relatedActivity, delay_s: dismountDelay);
		msg.SetReceiver(agent);
		#ifdef AI_DEBUG
		msg.m_sSentFromBt = sendFrom;
		#endif
		myComms.RequestBroadcast(msg, agent);
	}
	
	//------------------------------------------------------------------------------------
	static void SendGetInMessage(notnull AIAgent agent, notnull IEntity vehicleEntity, EAICompartmentType roleInVehicle,
								SCR_AIActivityBase relatedActivity, notnull AICommunicationComponent myComms, string sendFrom = string.Empty)
	{
		SCR_AIBoardingParameters bParams = new SCR_AIBoardingParameters();
		int priorityLevel = SCR_AIActionBase.PRIORITY_LEVEL_NORMAL;
		if (relatedActivity)
			priorityLevel = relatedActivity.EvaluatePriorityLevel();
		SCR_AIMessage_GetIn msg = SCR_AIMessage_GetIn.Create(vehicleEntity, bParams, roleInVehicle, false, 
										priorityLevel: priorityLevel, null, relatedActivity: relatedActivity, compartmentSlot: null);
		
		msg.SetReceiver(agent);
		#ifdef AI_DEBUG
		msg.m_sSentFromBt = sendFrom;
		#endif
		myComms.RequestBroadcast(msg, agent);
	}
	
	//------------------------------------------------------------------------------------
	static void SendMoveMessage(notnull AIAgent agent, IEntity moveToEntity, SCR_AIActivityBase relatedActivity, 
									notnull AICommunicationComponent myComms, string sendFrom = string.Empty)
	{
		SCR_AIMessage_Move msg = SCR_AIMessage_Move.Create(moveToEntity, vector.Zero, EMovementType.SPRINT, true, relatedActivity);
		msg.SetReceiver(agent);
		#ifdef AI_DEBUG
		msg.m_sSentFromBt = sendFrom;
		#endif
		myComms.RequestBroadcast(msg, agent);
	}
	
	//------------------------------------------------------------------------------------
	static void SendCancelMessage(notnull AIAgent agent, SCR_AIActivityBase relatedActivity, 
									notnull AICommunicationComponent myComms, string sendFrom = string.Empty)
	{
		SCR_AIMessage_Cancel msg = SCR_AIMessage_Cancel.Create(relatedActivity);
		msg.SetReceiver(agent);
		#ifdef AI_DEBUG
		msg.m_sSentFromBt = sendFrom;
		#endif
		myComms.RequestBroadcast(msg, agent);
	}
	
	//------------------------------------------------------------------------------------
	static void SendAnimateMessage(notnull AIAgent agent, notnull IEntity rootEntity, notnull SCR_AIAnimationScript script, SCR_AIActivityBase relatedActivity, ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction> relatedInvoker,
									notnull AICommunicationComponent myComms, string sendFrom = string.Empty)
	{
		SCR_AIMessage_Animate msg = SCR_AIMessage_Animate.Create(rootEntity, script, relatedActivity, relatedInvoker);
		msg.SetReceiver(agent);
		#ifdef AI_DEBUG
		msg.m_sSentFromBt = sendFrom;
		#endif
		myComms.RequestBroadcast(msg, agent);
	}
}

class SCR_AIWorldHandling
{
	//! Returns true if according to ambient lv perception it's dark right now, used in vehicle lights and illum flares usage
	static bool IsLowLightEnvironment()
	{
		PerceptionManager pm = GetGame().GetPerceptionManager();
		if (!pm)
			return false;
		
		float directLV, ambientLV, totalAmbientLv;
		pm.GetAmbientLV(directLV, ambientLV, totalAmbientLv);
		
		return totalAmbientLv < -3.5;
	}
}

class SCR_AIFactionHandling
{
	//! Returns GetPerceivedFaction from PerceivableComponent - the faction which we should use in all AI scripts.
	static Faction GetEntityPerceivedFaction(notnull IEntity entity)
	{
		// Common case first
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(entity);
		if (character && character.m_pPerceivableComponent)
		{
			return character.m_pPerceivableComponent.GetPerceivedFaction();
		}
		
		Vehicle vehicle = Vehicle.Cast(entity);
		if (vehicle && vehicle.m_pPerceivableComponent)
		{
			return vehicle.m_pPerceivableComponent.GetPerceivedFaction();
		}
		
		// Worst case - some other entity, perform component search
		PerceivableComponent perceivableComponent = PerceivableComponent.Cast(entity.FindComponent(PerceivableComponent));
		if (perceivableComponent)
			return perceivableComponent.GetPerceivedFaction();
		
		return null;
	}
	
	//! Returns actual faction from FactionAffiliationComponent.
	static Faction GetEntityFaction(notnull IEntity entity)
	{
		// Common case first
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(entity);
		if (character && character.m_pFactionComponent)
		{
			return character.m_pFactionComponent.GetAffiliatedFaction();
		}
		
		Vehicle vehicle = Vehicle.Cast(entity);
		if (vehicle && vehicle.m_pFactionComponent)
		{
			return vehicle.m_pFactionComponent.GetAffiliatedFaction();
		}
		
		// Worst case - some other entity, perform component search
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(entity.FindComponent(FactionAffiliationComponent));
		if (factionAffiliation)
			return factionAffiliation.GetAffiliatedFaction();
		
		return null;
	}
}

class SCR_AIUtils
{
	static AIAgent GetAIAgent(notnull IEntity ent)
	{
		AIControlComponent controlComp = AIControlComponent.Cast(ent.FindComponent(AIControlComponent));
		if (!controlComp)
			return null;
		
		return controlComp.GetControlAIAgent();
	}
}