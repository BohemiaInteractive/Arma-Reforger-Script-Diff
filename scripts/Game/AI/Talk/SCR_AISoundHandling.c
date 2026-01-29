enum ECommunicationType
{
	NONE,
	REPORT_TARGET,
	REPORT_MOVE,
	REPORT_RETURN,
	REPORT_MOUNT,
	REPORT_MOUNT_AS,
	REPORT_MOUNT_NEAREST,
	REPORT_UNMOUNT,
	REPORT_STOP,
	REPORT_FLANK,
	REPORT_DEFEND,
	REPORT_CONTACT,	
	REPORT_NO_AMMO,
	REPORT_UNDER_FIRE,
	REPORT_CLEAR,
	REPORT_ENGAGING,
	REPORT_TARGET_DOWN,
	REPORT_TARGET_LOST,
	REPORT_RELOADING,
	REPORT_MOVING,
	REPORT_COVERING,
	REPORT_NEGATIVE,
	REPORT_SEARCH_AND_DESTROY,
	REPORT_IDLE,
}


class SCR_AISoundHandling
{
	static const int NORMAL_PRIORITY = 50;  				// priority of signals
	static const int SIGNAL_VALUE_SOLDIER_ALL = 1000;
	static const int DISTANCE_TYPE_MID = 50;
	static const int DISTANCE_TYPE_LONG = 2500;	
	
	//--------------------------------------------------------------------------------------------
	static bool SetSignalsAndTransmit(SCR_AITalkRequest request, IEntity speakerEntity, VoNComponent von, SignalsManagerComponent signalsMgr)
	{
		ref array<float> signals = {};
		SCR_CallsignCharacterComponent callsignComponent;
		int SN_Seed = signalsMgr.FindSignal("Seed");
		// randomize variant of the signal
		SetSignal(SN_Seed, Math.RandomFloat(0, 1), signals);
		// different signals depending on cummunication type
		if (request.m_Entity)
			 callsignComponent = SCR_CallsignCharacterComponent.Cast(request.m_Entity.FindComponent(SCR_CallsignCharacterComponent));
		
		ECommunicationType commType = request.m_eCommType;
		switch (commType)
		{
			case ECommunicationType.REPORT_TARGET:
			{
				if (!request.m_Entity)
				{
					ErrorMissingEntity(commType);
					return false;
				}
				SetSignal_TargetValues(request.m_Entity, signalsMgr, signals);
				SetSignal_SoldierCalledValues(signalsMgr, SIGNAL_VALUE_SOLDIER_ALL, signals);
				von.SoundEventPriority(SCR_SoundEvent.SOUND_CP_TARGET, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_MOVE:
			{
				if (request.m_vPosition == vector.Zero)
				{
					ErrorMissingPosition(commType);
					return false;
				}
				SetSignal_SoldierCalledValues(signalsMgr, SIGNAL_VALUE_SOLDIER_ALL, signals);
				
				float distanceType = SetSignal_PositionValues(request.m_Entity, signalsMgr, request.m_vPosition, speakerEntity.GetOrigin(), signals);
				if (distanceType < DISTANCE_TYPE_MID) 														// close range distance
					von.SoundEventPriority(SCR_SoundEvent.SOUND_CP_MOVE_CLOSE,  signals, NORMAL_PRIORITY);
				else if (distanceType < DISTANCE_TYPE_LONG) 												// mid range distance
					von.SoundEventPriority(SCR_SoundEvent.SOUND_CP_MOVE_MID, signals, NORMAL_PRIORITY);
				else  																						// long range distance
					von.SoundEventPriority(SCR_SoundEvent.SOUND_CP_MOVE_LONG, signals, NORMAL_PRIORITY);
				
				break;
			}
			case ECommunicationType.REPORT_CONTACT:
			{
				if (!request.m_Entity || request.m_vPosition == vector.Zero)
				{
					ErrorMissingEntityOrPosition(commType);
					return false;
				}
				SetSignal_ObjectType(request.m_Entity, signalsMgr, signals);
				SetSignal_TargetFactionValues(request.m_Entity, signalsMgr, signals);
				float distanceType = SetSignal_PositionValues(request.m_Entity, signalsMgr, request.m_vPosition, speakerEntity.GetOrigin(), signals);
				SetSignal_FactionValues(signalsMgr, 1, signals);
				if (distanceType < DISTANCE_TYPE_MID) 														
					von.SoundEventPriority(SCR_SoundEvent.SOUND_CP_SPOTTED_CLOSE,  signals, NORMAL_PRIORITY);
				else if (distanceType < DISTANCE_TYPE_LONG) 												
					von.SoundEventPriority(SCR_SoundEvent.SOUND_CP_SPOTTED_MID, signals, NORMAL_PRIORITY);
				else  																						
					von.SoundEventPriority(SCR_SoundEvent.SOUND_CP_SPOTTED_LONG, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_RETURN:
			{
				SetSignal_SoldierCalledValues(signalsMgr, SIGNAL_VALUE_SOLDIER_ALL, signals);
				von.SoundEventPriority(SCR_SoundEvent.SOUND_CP_RETURN, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_MOUNT:
			{
				if (!request.m_Entity)
				{
					ErrorMissingEntity(commType);
					return false;
				}
				SetSignal_TargetValues(request.m_Entity, signalsMgr, signals);
				SetSignal_SoldierCalledValues(signalsMgr, SIGNAL_VALUE_SOLDIER_ALL, signals);
				von.SoundEventPriority(SCR_SoundEvent.SOUND_CP_MOUNT_BOARD, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_MOUNT_AS:
			{
				if (!callsignComponent)
				{
					ErrorMissingCallsign(commType);
					return false;
				}
				SetSignal_MountAsValues(signalsMgr, callsignComponent.GetCharacterCallsignIndex(), RoleInVehicleToSoundRoleEnum(request.m_EnumSignal), signals);
				von.SoundEventPriority(SCR_SoundEvent.SOUND_CP_MOUNT_ROLE, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_MOUNT_NEAREST:
			{
				SetSignal_SoldierCalledValues(signalsMgr, SIGNAL_VALUE_SOLDIER_ALL, signals);
				von.SoundEventPriority(SCR_SoundEvent.SOUND_CP_MOUNT_NEAREST, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_UNMOUNT:
			{
				SetSignal_SoldierCalledValues(signalsMgr, SIGNAL_VALUE_SOLDIER_ALL, signals);
				von.SoundEventPriority(SCR_SoundEvent.SOUND_CP_MOUNT_GETOUT, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_STOP:
			{
				if (callsignComponent)
					SetSignal_SoldierCalledValues(signalsMgr, callsignComponent.GetCharacterCallsignIndex(), signals);
				von.SoundEventPriority(SCR_SoundEvent.SOUND_CP_STOP, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_FLANK:
			{
				if (!callsignComponent)
				{
					ErrorMissingCallsign(commType);
					return false;
				}
				SetSignal_SoldierCalledValues(signalsMgr, callsignComponent.GetCharacterCallsignIndex(), signals);
				SetSignal_FlankValues(signalsMgr, request.m_EnumSignal, signals);
				von.SoundEventPriority(SCR_SoundEvent.SOUND_CP_FLANK, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_SEARCH_AND_DESTROY:
			{
				SetSignal_SoldierCalledValues(signalsMgr, SIGNAL_VALUE_SOLDIER_ALL, signals);
				von.SoundEventPriority(SCR_SoundEvent.SOUND_CP_SEARCH_AND_DESTROY, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_NO_AMMO:
			{
				von.SoundEventPriority(SCR_SoundEvent.SOUND_REPORTS_STATUS_NOAMMO, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_CLEAR:
			{
				von.SoundEventPriority(SCR_SoundEvent.SOUND_REPORTS_STATUS_CLEAR, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_ENGAGING:
			{
				von.SoundEventPriority(SCR_SoundEvent.SOUND_REPORTS_STATUS_ENGAGING, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_TARGET_DOWN:
			{
				von.SoundEventPriority(SCR_SoundEvent.SOUND_REPORTS_STATUS_TARGETDOWN, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_RELOADING:
			{
				von.SoundEventPriority(SCR_SoundEvent.SOUND_REPORTS_ACTIONSTATUS_RELOAD, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_MOVING:
			{
				von.SoundEventPriority(SCR_SoundEvent.SOUND_REPORTS_ACTIONSTATUS_MOVE, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_COVERING:
			{
				von.SoundEventPriority(SCR_SoundEvent.SOUND_REPORTS_ACTIONSTATUS_COVER, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_DEFEND:
			{
				SetSignal_SoldierCalledValues(signalsMgr, SIGNAL_VALUE_SOLDIER_ALL, signals);
				von.SoundEventPriority(SCR_SoundEvent.SOUND_CP_DEFEND_POSITION, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_NEGATIVE:
			{
				von.SoundEventPriority(SCR_SoundEvent.SOUND_CP_NEGATIVEFEEDBACK_NEGATIVE, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_IDLE:
			{
				von.SoundEventPriority(SCR_SoundEvent.SOUND_CP_IDLE_SPEECH, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_TARGET_LOST:
			{
				if (callsignComponent)
					SetSignal_SoldierCalledValues(signalsMgr, callsignComponent.GetCharacterCallsignIndex(), signals);
				von.SoundEventPriority(SCR_SoundEvent.SOUND_CP_REPORTS_AREACLEAR, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_UNDER_FIRE:
			{
				if (callsignComponent)
					SetSignal_SoldierCalledValues(signalsMgr, callsignComponent.GetCharacterCallsignIndex(), signals);
				von.SoundEventPriority(SCR_SoundEvent.SOUND_CP_REPORTS_UNDER_FIRE, signals, NORMAL_PRIORITY);
				break;
			}
		}
		return true;
	}
	
	//--------------------------------------------------------------------------------------------
	static void ErrorMissingCallsign(ECommunicationType commType)
	{
		Print(string.Format("SCR_AISoundHandling: SetSignalsAndTransmit: Missing callsign for: %1", typename.EnumToString(ECommunicationType, commType)), LogLevel.ERROR);
	}
	
	//--------------------------------------------------------------------------------------------
	static void ErrorMissingEntity(ECommunicationType commType)
	{
		Print(string.Format("SCR_AISoundHandling: SetSignalsAndTransmit: Missing entity for: %1", typename.EnumToString(ECommunicationType, commType)), LogLevel.ERROR);
	}
	
	//--------------------------------------------------------------------------------------------
	static void ErrorMissingPosition(ECommunicationType commType)
	{
		Print(string.Format("SCR_AISoundHandling: SetSignalsAndTransmit: Missing position for: %1", typename.EnumToString(ECommunicationType, commType)), LogLevel.ERROR);
	}
	
	//--------------------------------------------------------------------------------------------
	static void ErrorMissingEntityOrPosition(ECommunicationType commType)
	{
		Print(string.Format("SCR_AISoundHandling: SetSignalsAndTransmit: Missing entity or position for: %1", typename.EnumToString(ECommunicationType, commType)), LogLevel.ERROR);
	}
	
	//--------------------------------------------------------------------------------------------
	static SCR_AIInfoComponent GetInfoComponent(IEntity entity)
	{
		if (!entity)
			return null;
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(entity);
		if (entity && !agent)
		{
			AIControlComponent cc = AIControlComponent.Cast(entity.FindComponent(AIControlComponent));
			if (!cc)
				return null;
			agent = SCR_ChimeraAIAgent.Cast(cc.GetAIAgent());
			if (!agent)
				return null;
		}
		return agent.m_InfoComponent;
	}
	
	//--------------------------------------------------------------------------------------------
	static int InfoUnitRoleToSoundCharacterEnum(SCR_AIInfoComponent info)
	{
		if (!info)
			return ECP_Characters.MAN;
		else if (info.HasRole(EUnitRole.MACHINEGUNNER))
			return ECP_Characters.MACHINE_GUNNER;
		else if (info.HasRole(EUnitRole.AT_SPECIALIST))
			return ECP_Characters.AT_SOLDIER;
		else if (info.HasRole(EUnitRole.SNIPER))
			return ECP_Characters.SNIPER;
		else if (info.HasRole(EUnitRole.RIFLEMAN))
			return ECP_Characters.SOLDIER;
		return ECP_Characters.MAN;
	}
	
	//--------------------------------------------------------------------------------------------
	static int RoleInVehicleToSoundRoleEnum(int role)
	{
		switch (role)
		{
			case ECompartmentType.PILOT:
			{
				return ECP_VehicleRoles.DRIVER;
			}
			case ECompartmentType.TURRET:
			{
				return ECP_VehicleRoles.GUNNER;
			}
			case ECompartmentType.CARGO:
			{
				return ECP_VehicleRoles.PASSANGER;
			}
		}
		return role;
	}
	
	//--------------------------------------------------------------------------------------------
	static int VehicleTypeToSoundVehicleFactionEnum(int role)
	{
		switch (role)
		{
			case EVehicleType.VEHICLE:
			{
				return ECP_VehicleTypes.VEHICLE;
			}
			case EVehicleType.CAR:
			{
				return ECP_VehicleTypes.CAR;
			}
			case EVehicleType.TRUCK:
			{
				return ECP_VehicleTypes.TRUCK;
			}
			case EVehicleType.APC:
			{
				return ECP_VehicleTypes.APC;
			}
			case EVehicleType.COMM_TRUCK:
			{
				return ECP_VehicleTypes.COMM_TRUCK;
			}
			case EVehicleType.FUEL_TRUCK:
			{
				return ECP_VehicleTypes.FUEL_TRUCK;
			}
			case EVehicleType.SUPPLY_TRUCK:
			{
				return ECP_VehicleTypes.SUPPLY_TRUCK;
			}
			case EVehicleType.MORTAR:
			{
				return ECP_VehicleTypes.MORTAR;
			}
			default:
			{
				return ECP_VehicleTypes.HELICOPTER;
			}
		}
		return role;
	}
	
	//--------------------------------------------------------------------------------------------	
	static void SetSignal_ObjectType(IEntity target, SignalsManagerComponent signalsManagerComponent, array<float> signals)
	{
		int SN_ObjectType = signalsManagerComponent.FindSignal("ObjectType");
		float value;
		if (target.IsInherited(ChimeraCharacter))
			value = 0.0;
		else if (target.IsInherited(Vehicle))
			value = 1.0;
		else
			value = 2.0; // Misc
		SetSignal(SN_ObjectType, value, signals);
	}
	
	//--------------------------------------------------------------------------------------------	
	static void SetSignal_TargetValues(IEntity target, SignalsManagerComponent signalsManagerComponent, array<float> signals)
	{
		if (target.IsInherited(Vehicle))
		{
			int SN_Vehicle = signalsManagerComponent.FindSignal("Vehicle");
			
			SetSignal(SN_Vehicle, VehicleTypeToSoundVehicleFactionEnum(Vehicle.Cast(target).m_eVehicleType), signals);
		}
		
		else if (target.IsInherited(ChimeraCharacter))
		{
			int SN_Character = signalsManagerComponent.FindSignal("Character");
			SCR_AIInfoComponent targetInfoComponent = GetInfoComponent(target);
			int unitRole = InfoUnitRoleToSoundCharacterEnum(targetInfoComponent);
			
			SetSignal(SN_Character, unitRole, signals);
		}	
		//TODO: do Miscellaneous - now not objects to attacks
	}
	
	//--------------------------------------------------------------------------------------------	
	static void SetSignal_TargetFactionValues(IEntity target, SignalsManagerComponent signalsManagerComponent, array<float> signals)
	{
		if (target.IsInherited(Vehicle))
		{
			int SN_VehicleFaction = signalsManagerComponent.FindSignal("VehicleFaction");
			
			SetSignal(SN_VehicleFaction, 100 + VehicleTypeToSoundVehicleFactionEnum(Vehicle.Cast(target).m_eVehicleType), signals);
		}
		
		else if (target.IsInherited(ChimeraCharacter))
		{
			int SN_CharacterFaction = signalsManagerComponent.FindSignal("CharacterFaction");
			SCR_AIInfoComponent targetInfoComponent = GetInfoComponent(target);
			int unitRole = InfoUnitRoleToSoundCharacterEnum(targetInfoComponent) + 100;
			
			SetSignal(SN_CharacterFaction, unitRole, signals);
		}	
		//TODO: do Miscellaneous - now not objects to attacks
	}
	
	//--------------------------------------------------------------------------------------------
	static float SetSignal_PositionValues(IEntity target, SignalsManagerComponent signalsManagerComponent, vector locationTarget, vector locationSelf, array<float> signals)
	{
		if (target)
			locationTarget = target.GetOrigin();
		
		float distance = vector.Distance(locationTarget,locationSelf);
		if (distance < DISTANCE_TYPE_MID)
		{
			vector dirVec = locationTarget - locationSelf;
			float directionAngle = dirVec.ToYaw();
			int SN_DirectionClose = signalsManagerComponent.FindSignal("DirectionClose");
			SetSignal(SN_DirectionClose, directionAngle, signals);
		}
		else if (distance < DISTANCE_TYPE_LONG)
		{
			vector dirVec = locationTarget - locationSelf;
			float directionAngle = dirVec.ToYaw();
			
			int SN_TargetDistance = signalsManagerComponent.FindSignal("TargetDistance");
			int SN_DirectionAngle = signalsManagerComponent.FindSignal("DirectionAngle");
			
			SetSignal(SN_TargetDistance, distance, signals);
			SetSignal(SN_DirectionAngle, directionAngle, signals);
		}
		else if (distance >= DISTANCE_TYPE_LONG)
		{
			SetSignal_GridValues(locationTarget, signalsManagerComponent, signals);
		}
		return distance;
	}
	
	//--------------------------------------------------------------------------------------------
	static void SetSignal_GridValues(vector locationTarget, SignalsManagerComponent signalsManagerComponent, array<float> signals)
	{
		array<int> gridXcoords = {};
		array<int> gridZcoords = {};
		int gridX, gridZ;
		SCR_MapEntity.GetGridPos(locationTarget, gridX: gridX, gridZ: gridZ);
							
		int SN_Grid = signalsManagerComponent.FindSignal("Grid");
		for (int i = 2; i >= 0; i--)
		{
			int mod = Math.Pow(10, i);
			gridXcoords.Insert(gridX / mod);
			gridZcoords.Insert(gridZ / mod);
			gridX = gridX % mod;
			gridZ = gridZ % mod;
		};
		for (int i = 0; i < 3; i++)
			SetSignal(SN_Grid, gridXcoords[i], signals);
		for (int i = 0; i < 3; i++)
			SetSignal(SN_Grid, gridZcoords[i], signals);
	}
	
	//--------------------------------------------------------------------------------------------
	static void SetSignal_SoldierCalledValues(SignalsManagerComponent signalsManagerComponent, int soldierCalled, array<float> signals)
	{
		int SN_SoldierCalled = signalsManagerComponent.FindSignal("SoldierCalled");
		
		SetSignal(SN_SoldierCalled, soldierCalled, signals);
	}
	
	//--------------------------------------------------------------------------------------------
	static void SetSignal_MountAsValues(SignalsManagerComponent signalsManagerComponent, int soldierCalledId, int soldierRole, array<float> signals)
	{
		int SN_SoldierCalled = signalsManagerComponent.FindSignal("SoldierCalled");
		int SN_Role = signalsManagerComponent.FindSignal("Role");
		
		SetSignal(SN_SoldierCalled, soldierCalledId, signals);
		SetSignal(SN_Role, soldierRole, signals);
	}
	
	//--------------------------------------------------------------------------------------------
	static void SetSignal_FlankValues(SignalsManagerComponent signalsManagerComponent, int flank, array<float> signals)
	{
		int SN_Flank = signalsManagerComponent.FindSignal("Flank");
		
		SetSignal(SN_Flank, flank, signals);
	}
	
	//--------------------------------------------------------------------------------------------
	static void SetSignal_FactionValues(SignalsManagerComponent signalsManagerComponent, int faction, array<float> signals)
	{
		int SN_Faction = signalsManagerComponent.FindSignal("Faction");
		
		SetSignal(SN_Faction, faction, signals);
	}
	
	//--------------------------------------------------------------------------------------------
	static void SetSignal(int index, float value, notnull array<float> signals)
	{
		signals.Insert(index);
		signals.Insert(value);
	}
	
	//--------------------------------------------------------------------------------------------
	static SCR_AICommsHandler FindCommsHandler(AIAgent agent)
	{
		SCR_ChimeraAIAgent chimeraAgent = SCR_ChimeraAIAgent.Cast(agent);
		if (!chimeraAgent)
			return null;
		return chimeraAgent.m_UtilityComponent.m_CommsHandler;
	}
}