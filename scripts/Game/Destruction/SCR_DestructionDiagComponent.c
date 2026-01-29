[EntityEditorProps(category: "GameScripted/Destruction")]
class SCR_DestructionDiagComponentClass : ScriptComponentClass
{
}

class SCR_DestructionDiagComponent : ScriptComponent
{
#ifdef ENABLE_DIAG
	protected static const string DAMAGE_TYPE_NAMES[10] = { "TRUE", "COLLISION", "MELEE", "KINETIC", "FRAGMENTATION", "EXPLOSIVE", "INCENDIARY", "FIRE", "REGENERATION", "BLEEDING" };
	protected ref array<IEntity> m_aToDamage = {};
	
	//------------------------------------------------------------------------------------------------
	override void EOnDiag(IEntity owner, float timeSlice)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_DESTRUCTION_ENABLE_DIAG))
		{
			TraceParam params = new TraceParam();
			TraceEntity(params);
			
			DbgUI.Begin("Destruction");
			
			float damageToDeal = 1000, range = 20;
			EDamageType damageType;
			DbgUI.InputFloat("Damage to deal", damageToDeal);
			
			DbgUI.InputFloat("Range", range);
			
			DbgUI.InputInt("Damage type", damageType);
			
			DbgUI.Spacer(8);
			
			DbgUI.Text("Damage types:");
			DbgUI.Text("0 = TRUE");
			DbgUI.Text("1 = COLLISION");
			DbgUI.Text("2 = MELEE");
			DbgUI.Text("3 = KINETIC");
			DbgUI.Text("4 = FRAGMENTATION");
			DbgUI.Text("5 = EXPLOSIVE");
			DbgUI.Text("6 = INCENDIARY");
			DbgUI.Text("7 = FIRE");
			DbgUI.Text("8 = REGENERATION");
			DbgUI.Text("9 = BLEEDING");
			
			DbgUI.Spacer(8);
			
			if (params.TraceEnt)
			{
				DbgUI.Text("Damage threshold: " + GetDamageThreshold(params));
				DbgUI.Text("Damage reduction: " + GetDamageReduction(params));
				ShowDamageTypeMultipliers(params);
			}
			
			if (DbgUI.Button("Deal damage in range"))
				DealDamageRange(damageToDeal, range, damageType);
			
			if (DbgUI.Button("Deal damage under cursor"))
				DealDamageUnderCursor(damageToDeal, damageType, params);
			
			if (DbgUI.Button("Set 1 HP in range"))
				SetOneHitPoint(range);
			DbgUI.End();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] params
	void ShowDamageTypeMultipliers(TraceParam params)
	{
		SCR_DestructibleEntity destructibleEntity = SCR_DestructibleEntity.Cast(params.TraceEnt);
		if (destructibleEntity)
		{
			for (int i = 0; i < 10; i++)
			{
				DbgUI.Text(DAMAGE_TYPE_NAMES[i] + ": " + destructibleEntity.GetDamageMultiplier(i));
			}
		}
		
		SCR_DestructionDamageManagerComponent destructionComponent = SCR_DestructionDamageManagerComponent.Cast(params.TraceEnt.FindComponent(SCR_DestructionDamageManagerComponent));
		if (destructionComponent)
		{
			HitZone hitzone = destructionComponent.GetHitZone(params.ColliderName);
			
			if (!hitzone)
				hitzone = destructionComponent.GetDefaultHitZone();
			
			if (!hitzone)
				return; // No hitzone found
			
			for (int i = 0; i < 10; i++)
			{
				DbgUI.Text(DAMAGE_TYPE_NAMES[i] + ": " + hitzone.GetDamageMultiplier(i));
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] params
	//! \return
	float GetDamageReduction(TraceParam params)
	{
		SCR_DestructibleEntity destructibleEntity = SCR_DestructibleEntity.Cast(params.TraceEnt);
		if (destructibleEntity)
			return destructibleEntity.GetDamageReduction();
		
		SCR_DestructionDamageManagerComponent destructionComponent = SCR_DestructionDamageManagerComponent.Cast(params.TraceEnt.FindComponent(SCR_DestructionDamageManagerComponent));
		if (destructionComponent)
		{
			HitZone hitZone = destructionComponent.GetHitZone(params.ColliderName);
			if (!hitZone)
				hitZone = destructionComponent.GetDefaultHitZone();
			
			if (!hitZone)
				return -1; // No hitzone found!
			
			return hitZone.GetDamageReduction();
		}
		
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] params
	//! \return
	float GetDamageThreshold(TraceParam params)
	{
		SCR_DestructibleEntity destructibleEntity = SCR_DestructibleEntity.Cast(params.TraceEnt);
		if (destructibleEntity)
			return destructibleEntity.GetDamageThreshold();
		
		SCR_DestructionDamageManagerComponent destructionComponent = SCR_DestructionDamageManagerComponent.Cast(params.TraceEnt.FindComponent(SCR_DestructionDamageManagerComponent));
		if (destructionComponent)
		{
			HitZone hitZone = destructionComponent.GetHitZone(params.ColliderName);
			if (!hitZone)
				hitZone = destructionComponent.GetDefaultHitZone();
			
			if (!hitZone)
				return -1; // No hitzone found!
			
			return hitZone.GetDamageThreshold();
		}
		
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] mat
	void GetCameraMat(out vector mat[4])
	{
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return;
		
		CameraBase currentCamera = cameraManager.CurrentCamera();
		if (!currentCamera)
			return;
		
		currentCamera.GetTransform(mat);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] params
	void TraceEntity(TraceParam params)
	{
		vector cameraMat[4];
		GetCameraMat(cameraMat);
		
		params.Start = cameraMat[3];
		params.End = params.Start + cameraMat[2] * 10; // 10m trace
		params.LayerMask = EPhysicsLayerDefs.Projectile;
		params.Exclude = GetGame().GetPlayerController().GetControlledEntity();
		params.Flags = TraceFlags.ENTS;
		
		GetOwner().GetWorld().TraceMove(params, null);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] damageToDeal
	//! \param[in] damageType
	//! \param[in] params
	void DealDamageUnderCursor(float damageToDeal, EDamageType damageType, notnull TraceParam params)
	{
		if (!params.TraceEnt)
			return;
		
		DealDamage(params.TraceEnt, damageToDeal, damageType, params.SurfaceProps);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] entity
	//! \return
	float GetHealth(IEntity entity)
	{
		SCR_DestructibleEntity destructibleEntity = SCR_DestructibleEntity.Cast(entity);
		if (destructibleEntity)
			return destructibleEntity.GetCurrentHealth();
		
		SCR_DestructionDamageManagerComponent destructionComponent = SCR_DestructionDamageManagerComponent.Cast(entity.FindComponent(SCR_DestructionDamageManagerComponent));
		if (destructionComponent)
			return destructionComponent.GetHealth();
		
		return 1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] range
	void SetOneHitPoint(float range)
	{
		vector cameraMat[4];
		GetCameraMat(cameraMat);
		
		GetGame().GetWorld().QueryEntitiesBySphere(cameraMat[3], range, AddEntity, FilterEntity);
		
		for (int i = m_aToDamage.Count() - 1; i >= 0; i--)
		{
			DealDamage(m_aToDamage[i], GetHealth(m_aToDamage[i]) - 1, EDamageType.TRUE, null);
		}
		
		m_aToDamage.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	//! \param[in] damageToDeal
	//! \param[in] damageType
	//! \param[in] surfaceProperties
	void DealDamage(IEntity entity, float damageToDeal, EDamageType damageType, SurfaceProperties surfaceProperties)
	{
		vector hitPosDirNorm[3];
		
		SCR_DestructibleEntity destructibleEntity = SCR_DestructibleEntity.Cast(entity);
		if (destructibleEntity)
			destructibleEntity.HandleDamage(damageType, damageToDeal, hitPosDirNorm);
		
		SCR_DestructionDamageManagerComponent destructionComponent = SCR_DestructionDamageManagerComponent.Cast(entity.FindComponent(SCR_DestructionDamageManagerComponent));
		if (destructionComponent)
		{
			SCR_DamageContext damageContext = new SCR_DamageContext(damageType, damageToDeal, hitPosDirNorm, entity, null, null, surfaceProperties, -1, -1);
			
			destructionComponent.HandleDamage(damageContext);
		}
	}			
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] damageToDeal
	//! \param[in] range
	//! \param[in] damageType
	void DealDamageRange(float damageToDeal, float range, EDamageType damageType)
	{
		vector cameraMat[4];
		GetCameraMat(cameraMat);
		
		GetGame().GetWorld().QueryEntitiesBySphere(cameraMat[3], range, AddEntity, FilterEntity);
		
		foreach (IEntity toDamage : m_aToDamage)
		{
			DealDamage(toDamage, damageToDeal, damageType, null);
		}
		
		m_aToDamage.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] e
	//! \return
	bool AddEntity(IEntity e)
	{
		m_aToDamage.Insert(e);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] e
	//! \return
	bool FilterEntity(IEntity e)
	{
		if (SCR_DestructibleEntity.Cast(e) || e.FindComponent(SCR_DestructionDamageManagerComponent))
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_DESTRUCTION_ENABLE_DIAG, "", "Enable destruction diag", "Destruction");
		
		ConnectToDiagSystem(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		DisconnectFromDiagSystem(owner);
		
		super.OnDelete(owner);
	}
#endif
}
