[ComponentEditorProps(category: "GameScripted/Character", description: "Enables melee for character")]
class SCR_MeleeComponentClass : ScriptComponentClass
{
}

class SCR_MeleeHitDataClass
{
	IEntity m_Entity = null;
	IEntity m_Weapon = null;
	IEntity m_Bayonet = null;
	SurfaceProperties m_SurfaceProps = null;
	int	m_iNodeIndex = -1;
	float m_fDamage = 0;
	int m_iColliderIndex = -1;
	vector m_vHitPosition = vector.Zero;
	vector m_vHitDirection = vector.Zero;
	vector m_vHitNormal = vector.Zero;
}

void OnMeleePerformedDelegate(IEntity owner);
typedef func OnMeleePerformedDelegate;
typedef ScriptInvokerBase<OnMeleePerformedDelegate> OnMeleePerformedInvoker;

class SCR_MeleeComponent : ScriptComponent
{
	//#define SCR_MELEE_DEBUG				//! uncomment to enable melee debug draws
	//#define SCR_MELEE_DEBUG_MSG			//! uncomment to enable melee debug prints into console
#ifdef SCR_MELEE_DEBUG
	ref array<ref Shape> m_aDbgSamplePositionsShapes;
	ref array<ref Shape> m_aDbgCollisionShapes;
#endif
	
	protected ref SCR_MeleeHitDataClass m_MeleeHitData;		//! holds melee hit data
	protected bool m_bMeasurementDone;
	protected bool m_bAttackAlreadyExecuted = false;			//! attack execution limiter
	protected bool m_bMeleeAttackStarted = false;
	protected float m_fWeaponMeleeAccuracy;
	protected float m_fWeaponMeleeRange;
	protected ref OnMeleePerformedInvoker m_OnMeleePerformed;
	protected SCR_MeleeWeaponProperties m_MeleeWeaponProperties;
	
	[RplProp()]
	protected bool m_bHasBayonet;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool HasBayonet()
	{
		return m_bHasBayonet;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	OnMeleePerformedInvoker GetOnMeleePerformed()
	{
		if (!m_OnMeleePerformed)
			m_OnMeleePerformed = new OnMeleePerformedInvoker;
		
		return m_OnMeleePerformed;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called from character command handler, based on user input.
	void PerformAttack()
	{
		if (m_bAttackAlreadyExecuted)
			return;
		
#ifdef SCR_MELEE_DEBUG
		Do_ClearDbgShapes();
#endif
		
		//This can be attached to a script invoker, which tells us the player changed their weapon
		CollectMeleeWeaponProperties();
		m_bAttackAlreadyExecuted = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Enable or disable measuring of hit detection probes
	//! \param[in] state if is/is not allowed to measure
	void SetMeleeAttackStarted(bool started)
	{
		m_bMeleeAttackStarted = started;
		
		if (!started)
			Do_OnFrameCleanup();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool TraceFilter(notnull IEntity ent)
	{
		if (SCR_ChimeraCharacter.Cast(ent))
			return true;
				
		return !SCR_ChimeraCharacter.Cast(ent.GetRootParent()); // Ignore anything that is not a character but attached to one e.g. weapons, armor, etc.
	}
	
	//------------------------------------------------------------------------------------------------
	protected vector GetAimingVector(notnull ChimeraCharacter character)
	{
		vector aimMat[4];
		if (character)
			Math3D.AnglesToMatrix(character.GetCharacterController().GetInputContext().GetAimingAngles() * Math.RAD2DEG, aimMat);
	
		return aimMat[2];
	}

	//------------------------------------------------------------------------------------------------
	//! Primary collision check that uses TraceMove and single TraceSphere fired from character eye pos along the aim fwd vector
	//! \param[in] pHitData SCR_MeleeHitDataClass holder that keeps information about actual hit
	//! \return When hit is detected or it is miss
	protected bool CheckCollisionsSimple(out SCR_MeleeHitDataClass pHitData)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return false;
		
		TraceSphere param = new TraceSphere();
		param.LayerMask = EPhysicsLayerDefs.Projectile;
		param.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
		array<IEntity> ExcludeArray = {character};
		param.ExcludeArray = ExcludeArray;
		
		param.Radius = m_fWeaponMeleeAccuracy;		
		param.Start = character.EyePosition();
		param.End = param.Start + GetAimingVector(character) * m_fWeaponMeleeRange;

		// Trace stops on the first hit entity if you want to change this, change the filter callback
		float hit = GetOwner().GetWorld().TraceMove(param, TraceFilter);

		if (!param.TraceEnt)
			return false;
		
		vector dir = (param.End - param.Start);
		
		pHitData.m_Entity = param.TraceEnt;
		pHitData.m_iColliderIndex = param.ColliderIndex;
		pHitData.m_vHitPosition = dir * hit + param.Start;
		pHitData.m_vHitDirection = dir.Normalized();
		pHitData.m_vHitNormal = param.TraceNorm;
		pHitData.m_SurfaceProps = param.SurfaceProps;
		pHitData.m_iNodeIndex = param.NodeIndex;
		
#ifdef SCR_MELEE_DEBUG_MSG
		MCDbgPrint("Check collision from: "+param.Start);
		MCDbgPrint("Check collision to: "+param.End);
		MCDbgPrint("Check collision range: " +vector.Distance(param.End, param.Start));
		MCDbgPrint("Check collision entity: "+pHitData.m_Entity.ToString());
		MCDbgPrint("Check collision hitzone: "+pHitData.m_sHitZoneName);
		MCDbgPrint("Check collision hitPos: "+pHitData.m_vHitPosition);
#endif
		
		return true;
	}

	// TODO: call this from SCR_CharacterControllerComponent.OnWeaponSelected() when possible and remove from PerformAttack()
	//------------------------------------------------------------------------------------------------
	//! Prepare information from MeleeWeaponProperties component on weapon
	//! \param[in] weapon Equipped melee weapon we are getting the data from
	protected void CollectMeleeWeaponProperties()
	{
		BaseWeaponManagerComponent wpnManager = BaseWeaponManagerComponent.Cast(GetOwner().FindComponent(BaseWeaponManagerComponent));
		if (!wpnManager)
			return;
		
		WeaponSlotComponent currentSlot = WeaponSlotComponent.Cast(wpnManager.GetCurrent());
		if (!currentSlot)
			return;
		
		//! get current weapon and store it into SCR_MeleeHitDataClass instance
		m_MeleeHitData.m_Weapon = currentSlot.GetWeaponEntity();

		IEntity weaponEntity = currentSlot.GetWeaponEntity();
		if (!weaponEntity)
			return;
		
		m_MeleeWeaponProperties = SCR_MeleeWeaponProperties.Cast(weaponEntity.FindComponent(SCR_MeleeWeaponProperties));
		if (!m_MeleeWeaponProperties)
			return;
		
		m_MeleeHitData.m_fDamage = m_MeleeWeaponProperties.GetWeaponDamage();
		m_fWeaponMeleeRange = m_MeleeWeaponProperties.GetWeaponRange();
		m_fWeaponMeleeAccuracy = m_MeleeWeaponProperties.GetWeaponMeleeAccuracy();
			
		IEntity bayonet;
		IEntity child = weaponEntity.GetChildren();
		while (child)
		{
			if (child.FindComponent(SCR_BayonetComponent))
			{
				bayonet = child;
				break;
			}
			
			child = child.GetSibling();
		}
		
		m_MeleeHitData.m_Bayonet = bayonet;
		m_bHasBayonet = m_MeleeHitData.m_Bayonet != null;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handles playing of sound & setting of signals (AUDIO)
	protected void HandleMeleeSound()
	{
		IEntity weapon = m_MeleeHitData.m_Weapon;
		if (!weapon)
			return;
		
		GameMaterial material = m_MeleeHitData.m_SurfaceProps;		
		if (!material)	
			return;
		
		int surfaceInt = material.GetSoundInfo().GetSignalValue();
		
		PlaySound(m_MeleeHitData.m_Weapon, m_MeleeHitData.m_vHitPosition, m_MeleeHitData.m_iColliderIndex, surfaceInt);
		
		RplComponent weaponRpl = RplComponent.Cast(m_MeleeHitData.m_Weapon.FindComponent(RplComponent));
		if (!weaponRpl)
			return;
				
		Rpc(RPC_HandleMeleeSound, weaponRpl.Id(), m_MeleeHitData.m_vHitPosition, m_MeleeHitData.m_iColliderIndex, surfaceInt);
		
		#ifdef WORKBENCH
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_PRINT_MELEESURFACE))
		{
			Print(string.Format("HitZone: %1", m_MeleeHitData.m_iColliderIndex), LogLevel.NORMAL);
			Print(string.Format("Surface: %1", surfaceInt), LogLevel.NORMAL);
			Print(string.Format("-------------------------------------"), LogLevel.NORMAL);
		}
		#endif
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void PlaySound(IEntity weaponEntity, vector position, int signalValue0, int signalValue1)
	{
		SoundComponent soundComp = SoundComponent.Cast(weaponEntity.FindComponent(SoundComponent));
		if (!soundComp)
			return;
		
		SignalsManagerComponent signalsManagerComponent = SignalsManagerComponent.Cast(weaponEntity.FindComponent(SignalsManagerComponent));
		if (signalsManagerComponent)
		{
			signalsManagerComponent.SetSignalValue(signalsManagerComponent.AddOrFindSignal("HitZone"), signalValue0);
			signalsManagerComponent.SetSignalValue(signalsManagerComponent.AddOrFindSignal("Surface"), signalValue1);		
		}
		
		vector transform[4];
		Math3D.MatrixIdentity3(transform);
		transform[3] = position;
		
		if (m_bHasBayonet)
			soundComp.SoundEventTransform(SCR_SoundEvent.SOUND_MELEE_IMPACT_BAYONET, transform);
		else
			soundComp.SoundEventTransform(SCR_SoundEvent.SOUND_MELEE_IMPACT, transform);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	protected void RPC_HandleMeleeSound(RplId weaponID, vector position, int signalValue0, int signalValue1)
	{
		RplComponent weaponRpl = RplComponent.Cast(Replication.FindItem(weaponID));
		if (!weaponRpl)
			return;
		
		IEntity weaponEntity = weaponRpl.GetEntity();
		if (!weaponEntity)
			return;
		
		PlaySound(weaponEntity, position, signalValue0, signalValue1);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Goes upwards the hierarchy of startEntity searching for SCR_DamageManagerComponent to deal damage to
	//! \param[in] startEntity
	//! \param[out] hitZone
	//! \return the found SCR_DamageManagerComponent or null if not found or if startEntity is null
	protected SCR_DamageManagerComponent SearchHierarchyForDamageManager(IEntity startEntity, out HitZone hitZone)
	{
		if (!startEntity)
			return null;
		
		SCR_DamageManagerComponent damageManager;		
		while (startEntity)
		{
			damageManager = SCR_DamageManagerComponent.GetDamageManager(startEntity);
			if (damageManager)
				break;
			
			startEntity = startEntity.GetParent();
		}
		
		if (damageManager)
		{
			hitZone = damageManager.GetHitZoneByColliderID(m_MeleeHitData.m_iColliderIndex);
			if (!hitZone)
				hitZone = damageManager.GetDefaultHitZone();
		}
		
		return damageManager;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Processes the melee attack. Applies the damage if applicable
	protected void ProcessMeleeAttack()
	{
		RplComponent rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (rplComponent && rplComponent.IsProxy())
			return; // Don't call on clients
		
		if (!CheckCollisionsSimple(m_MeleeHitData))
			return;
		
		if (m_MeleeHitData.m_Weapon)
			HandleMeleeSound();
		
#ifdef SCR_MELEE_DEBUG
		m_aDbgSamplePositionsShapes.Clear();
		Debug_DrawSphereAtPos(m_MeleeHitData.m_vHitPosition, m_aDbgSamplePositionsShapes, 0xff00ff00, 0.03, ShapeFlags.NOZBUFFER);
#endif
		
		if (m_OnMeleePerformed)
			m_OnMeleePerformed.Invoke(GetOwner());
		
		// This callLater is necessary because the use of traces pushed the handledamage out of main thread, which causes VME's
		GetGame().GetCallqueue().CallLater(HandleDamageDelayed);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HandleDamageDelayed()
	{
		if (m_MeleeHitData.m_Bayonet && m_MeleeHitData.m_Entity)
		{
			SCR_BayonetComponent bayonet = SCR_BayonetComponent.Cast(m_MeleeHitData.m_Bayonet.FindComponent(SCR_BayonetComponent));
			if (bayonet && SCR_ChimeraCharacter.Cast(m_MeleeHitData.m_Entity))
				bayonet.AddBloodToBayonet();
			
			SCR_BayonetEffectComponent effectComponent = SCR_BayonetEffectComponent.Cast(m_MeleeHitData.m_Bayonet.FindComponent(SCR_BayonetEffectComponent));
			if (effectComponent)
				effectComponent.OnImpact(
					m_MeleeHitData.m_Entity,
					m_MeleeHitData.m_fDamage,
					m_MeleeHitData.m_vHitPosition,
					m_MeleeHitData.m_vHitNormal,
					m_MeleeHitData.m_SurfaceProps);
		}
		
		vector hitPosDirNorm[3];
		hitPosDirNorm[0] = m_MeleeHitData.m_vHitPosition;
		hitPosDirNorm[1] = m_MeleeHitData.m_vHitDirection;
		hitPosDirNorm[2] = m_MeleeHitData.m_vHitNormal;
		
		// check if the entity is destructible entity
		SCR_DestructibleEntity destructibleEntity = SCR_DestructibleEntity.Cast(m_MeleeHitData.m_Entity);
		if (destructibleEntity)
		{
			destructibleEntity.HandleDamage(EDamageType.MELEE, m_MeleeHitData.m_fDamage, hitPosDirNorm);
			return;
		}
		
		// check if the entity has the damage manager component
		HitZone hitZone;
		SCR_DamageManagerComponent damageManager = SearchHierarchyForDamageManager(m_MeleeHitData.m_Entity, hitZone);
		if (!hitZone || !damageManager)
			return;

		SCR_DamageContext context = new SCR_DamageContext(EDamageType.MELEE, m_MeleeHitData.m_fDamage, hitPosDirNorm, 
			damageManager.GetOwner(), hitZone, Instigator.CreateInstigator(GetOwner()), 
			m_MeleeHitData.m_SurfaceProps, m_MeleeHitData.m_iColliderIndex, m_MeleeHitData.m_iNodeIndex);

		if (m_MeleeHitData.m_Bayonet)
			context.damageSource = m_MeleeHitData.m_Bayonet;
		else
			context.damageSource = m_MeleeHitData.m_Weapon;

		if (m_MeleeHitData.m_Bayonet)
			context.damageSource = m_MeleeHitData.m_Bayonet;
		else
			context.damageSource = m_MeleeHitData.m_Weapon;
		
		context.damageEffect = new SCR_MeleeDamageEffect();
		damageManager.HandleDamage(context);
	}
	
	//------------------------------------------------------------------------------------------------
	//! cleanup
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Cleanup of several variables called on frame
	protected void Do_OnFrameCleanup()
	{
		m_bAttackAlreadyExecuted = false;
		m_bMeasurementDone = false;
	}
	
#ifdef SCR_MELEE_DEBUG
	// dbg
	//------------------------------------------------------------------------------------------------
	//! Cleanup of several variables called on frame
	protected void Do_ClearDbgShapes()
	{
		m_aDbgSamplePositionsShapes.Clear();
		m_aDbgCollisionShapes.Clear();
	}
#endif

	//------------------------------------------------------------------------------------------------
	//! DEBUG methods
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Drawing of sphere if specified attributes on given vector pos
	protected void Debug_DrawSphereAtPos(vector v, array<ref Shape> dbgShapes, int color = COLOR_BLUE, float size = 0.03, ShapeFlags shapeFlags = ShapeFlags.VISIBLE)
	{
		shapeFlags = ShapeFlags.NOOUTLINE | shapeFlags;
		
		vector matx[4];
		Math3D.MatrixIdentity4(matx);
		matx[3] = v;
		Shape s = Shape.CreateSphere(color, shapeFlags, GetOwner().GetOrigin(), size);
		s.SetMatrix(matx);
		dbgShapes.Insert(s);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Debug_DrawSphereAtPos(vector pos, int color, array<ref Shape> dbgShapes)
	{
		vector matx[4];
		Math3D.MatrixIdentity4(matx);
		matx[3] = pos;
		int shapeFlags = ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP;
		Shape s = Shape.CreateSphere(color, shapeFlags, pos, 0.05);
		s.SetMatrix(matx);
		dbgShapes.Insert(s);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Debug_DrawLineSimple(vector start, vector end, array<ref Shape> dbgShapes)
	{
		vector p[2];
		p[0] = start;
		p[1] = end;

		int shapeFlags = ShapeFlags.NOOUTLINE;
		Shape s = Shape.CreateLines(ARGBF(1, 1, 1, 1), shapeFlags, p, 2);
		dbgShapes.Insert(s);
	}
	
#ifdef SCR_MELEE_DEBUG_MSG
	//------------------------------------------------------------------------------------------------
	//! Melee-specific prints only
	//! \param[in] msg the message to display
	protected void MCDbgPrint(string msg = "")
	{
		Print("\\_ SCR_MELEE_DEBUG] " + GetGame().GetTickCount().ToString() + " ** " + msg, LogLevel.NORMAL);
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	//! Event methods
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] timeSlice
	void Update(float timeSlice)
	{
		if (!m_bAttackAlreadyExecuted || !m_bMeleeAttackStarted || m_bMeasurementDone)
			return;
		
		ProcessMeleeAttack();
		m_bMeasurementDone = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_MeleeHitData = new SCR_MeleeHitDataClass;

		if (!GetGame().GetWorldEntity())
			return;
		
	#ifdef ENABLE_DIAG 
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_PRINT_MELEESURFACE, "", "Print Melee Surface", "Sounds"); 
	#endif
		
#ifdef SCR_MELEE_DEBUG
		m_aDbgSamplePositionsShapes = {};
		m_aDbgCollisionShapes = {};
#endif
	}
}
