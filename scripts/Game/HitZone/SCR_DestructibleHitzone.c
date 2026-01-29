//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// TODO: Visual destruction effects should be handled via separate component called by damage manager, not hitzone
#define ENABLE_BASE_DESTRUCTION
class SCR_DestructibleHitzone : SCR_HitZone
{
	[Attribute(desc: "Secondary explosion reference point", category: "Secondary damage")]
	protected ref PointInfo m_SecondaryExplosionPoint;

	[Attribute("", UIWidgets.Auto, desc: "Destruction Handler")]
	protected ref SCR_DestructionBaseHandler m_pDestructionHandler;

	[Attribute(desc: "Destruction sound event name", category: "Effects")]
	protected string m_sDestructionSoundEvent;

	[Attribute(desc: "Particle effect for destruction", params: "ptc", category: "Effects")]
	protected ResourceName m_sDestructionParticle;

	[Attribute(uiwidget: UIWidgets.Coords, "Position of the effect in model space", category: "Effects")]
	protected vector m_vParticleOffset;

	protected ParticleEffectEntity					m_DstParticle; // Destruction particle
	protected SCR_DamageManagerComponent			m_ParentDamageManager; // Damage manager of the direct parent
	protected SCR_DamageManagerComponent			m_RootDamageManager;
	protected SCR_BaseCompartmentManagerComponent	m_CompartmentManager; // The part may have occupants that we want to damage
	protected ref array<ChimeraCharacter>			m_aEjectedCharacters;

	//! Time after which we will no longer wait for players to leave destroyed vehicle and force start the destruction
	protected const int FORCED_DESTRUCTION_DELAY = 1000;

#ifdef ENABLE_BASE_DESTRUCTION
	//------------------------------------------------------------------------------------------------
	ref SCR_DestructionBaseHandler GetDestructionHandler()
	{
		return m_pDestructionHandler;
	}

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);

		m_CompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(pOwnerEntity.FindComponent(SCR_BaseCompartmentManagerComponent));

		// Parent damage manager
		IEntity parent = pOwnerEntity.GetParent();
		if (parent)
			m_ParentDamageManager = SCR_DamageManagerComponent.Cast(parent.FindComponent(SCR_DamageManagerComponent));

		// Root damage manager
		IEntity root = pOwnerEntity.GetRootParent();
		if (root)
			m_RootDamageManager = SCR_DamageManagerComponent.Cast(root.FindComponent(SCR_DamageManagerComponent));

		if (m_pDestructionHandler)
			m_pDestructionHandler.Init(pOwnerEntity, this);
		
		if (m_SecondaryExplosionPoint)
			m_SecondaryExplosionPoint.Init(pOwnerEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Get secondary explosion desired scale. It will determine the prefab retrieved from secondary explosion config.
	float GetSecondaryExplosionScale()
	{
		return GetMaxHealth();
	}

	//------------------------------------------------------------------------------------------------
	//! Get secondary explosion desired scale. It will determine the prefab retrieved from secondary explosion config.
	PointInfo GetSecondaryExplosionPoint()
	{
		return m_SecondaryExplosionPoint;
	}

	//------------------------------------------------------------------------------------------------
	//! Kill occupants and start destruction
	override void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP)
	{
		super.OnDamageStateChanged(newState, previousDamageState, isJIP);

		EDamageState state = GetDamageState();

		// Restrict AI from accessing this parts compartments
		if (m_CompartmentManager)
			m_CompartmentManager.SetCompartmentsAccessibleForAI(state != EDamageState.DESTROYED);

		// Change from destroyed can only mean repair
		if (GetPreviousDamageState() == EDamageState.DESTROYED)
		{
			StopDestruction();
			return;
		}

		if (state == EDamageState.DESTROYED)
			StartDestruction();

		if(isJIP)
			return;

		PlayDestructionSound(state);
		PlayDestructionParticle(state);
	}

	//------------------------------------------------------------------------------------------------
	//! Start destruction effects
	void StartDestruction(bool immediate = false)
	{
		if (m_CompartmentManager && GetHitZoneGroup() == EVehicleHitZoneGroup.HULL)
		{
			bool allEjectedImmediately;
			if (m_CompartmentManager.EjectRandomOccupants(1, true, allEjectedImmediately, true) && !allEjectedImmediately)//ejects all occupants of the vehicle
			{
				array<BaseCompartmentSlot> compartments = {};
				m_CompartmentManager.GetCompartments(compartments);
				ChimeraCharacter character;
				SCR_CompartmentAccessComponent access;
				PlayerManager playerMGR = GetGame().GetPlayerManager();
				foreach (BaseCompartmentSlot compartment : compartments)
				{
					if (!compartment)
						continue;

					character = ChimeraCharacter.Cast(compartment.GetOccupant());
					if (!character)
						continue;

					access = SCR_CompartmentAccessComponent.Cast(character.GetCompartmentAccessComponent());
					if (!access)
						continue;

					if (playerMGR && playerMGR.GetPlayerIdFromControlledEntity(character) == 0)
						continue;//we dont wait for AI as they are ejected immediately

					if (!m_aEjectedCharacters)
						m_aEjectedCharacters = {};

					m_aEjectedCharacters.Insert(character);
					access.GetOnPlayerCompartmentExit().Insert(OnCharacterEjectedFromDestroyedCompartment);
				}

				if (m_aEjectedCharacters && !m_aEjectedCharacters.IsEmpty())
				{
					//since its possible that client may fail the excecution of the getting out process (freeze/CTD/timeout/etc) then we have to have a backup plan
					GetGame().GetCallqueue().CallLater(ForceStartDestruction, FORCED_DESTRUCTION_DELAY);

					return;//now we wait for all characters to be ejected from the vehicle before we delete it
				}
			}
		}

		if (m_pDestructionHandler)
			m_pDestructionHandler.StartDestruction(immediate);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback that will be triggered when character will be removed from the destroyed vehicle
	//! \param[in] playerCharacter character that left the compartment
	//! \param[in] compartmentEntity
	protected void OnCharacterEjectedFromDestroyedCompartment(ChimeraCharacter playerCharacter, IEntity compartmentEntity)
	{
		if (GetOwner() && GetOwner().GetRootParent() != compartmentEntity.GetRootParent())
			return;

		SCR_CompartmentAccessComponent access = SCR_CompartmentAccessComponent.Cast(playerCharacter.GetCompartmentAccessComponent());
		if (!access)
			return;

		access.GetOnPlayerCompartmentExit().Remove(OnCharacterEjectedFromDestroyedCompartment);

		//since CompartmentAccessComponent and BaseCompartmentSlot will indicate that the character is still present when OnPlayerCompartmentExit is invoked, we need to manually keep track of whom we are waiting for
		m_aEjectedCharacters.RemoveItem(playerCharacter);

		foreach (ChimeraCharacter character : m_aEjectedCharacters)
		{//if for some reason some character was deleted before it left then we need to remove such entry
			if (!character)
			{
				m_aEjectedCharacters.RemoveItem(null);
				break;
			}
		}

		if (m_pDestructionHandler && (!m_aEjectedCharacters || m_aEjectedCharacters.IsEmpty()))
		{
			GetGame().GetCallqueue().Remove(ForceStartDestruction);
			m_pDestructionHandler.StartDestruction(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Force begining of the destruction process
	protected void ForceStartDestruction()
	{
		if (m_pDestructionHandler)
			m_pDestructionHandler.StartDestruction(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Stop destruction effects
	protected void StopDestruction()
	{
		// Must not change model outside frame
		if (m_pDestructionHandler)
			GetGame().GetCallqueue().CallLater(m_pDestructionHandler.OnRepair);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_VehicleDamageManagerComponent FindParentVehicleDamageManager()
	{
		// Futile if there is no hitzonecontainer anymore
		HitZoneContainerComponent hitZoneContainer = GetHitZoneContainer();
		if (!hitZoneContainer)
			return null;

		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(hitZoneContainer);
		if (damageManager)
			return damageManager;

		damageManager = SCR_VehicleDamageManagerComponent.Cast(m_ParentDamageManager);
		if (damageManager)
			return damageManager;

		IEntity parent = GetOwner().GetParent();
		while (parent)
		{
			damageManager = SCR_VehicleDamageManagerComponent.Cast(parent.FindComponent(SCR_VehicleDamageManagerComponent));
			if (damageManager)
				return damageManager;

			parent = parent.GetParent();
		}

		return damageManager;
	}

	//------------------------------------------------------------------------------------------------
	//! Start destruction sounds
	protected void PlayDestructionSound(EDamageState damageState)
	{
		// Sounds are only relevant for non-dedicated clients
		if (System.IsConsoleApp())
			return;

		if (damageState != EDamageState.DESTROYED)
			return;

		if (m_sDestructionSoundEvent.IsEmpty())
			return;

		IEntity owner = GetOwner();
		if (!owner)
			return;


		IEntity mainParent = SCR_EntityHelper.GetMainParent(owner);
		if (mainParent)
		{
			// Get position offset for slotted entities
			vector mins;
			vector maxs;
			owner.GetBounds(mins, maxs);
			vector center = vector.Lerp(mins, maxs, 0.5);

			// Add bone offset
			EntitySlotInfo slotInfo = EntitySlotInfo.GetSlotInfo(owner);
			if (slotInfo)
			{
				vector mat[4];
				slotInfo.GetModelTransform(mat);
				center = center + mat[3];
			}

			SoundComponent soundComponent = SoundComponent.Cast(mainParent.FindComponent(SoundComponent));
			if (!soundComponent)
				return;

			soundComponent.SoundEventOffset(m_sDestructionSoundEvent, center);
		}
		else
		{
			SoundComponent soundComponent = SoundComponent.Cast(owner.FindComponent(SoundComponent));
			if (!soundComponent)
				return;

			soundComponent.SoundEvent(m_sDestructionSoundEvent);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Start destruction particle
	void PlayDestructionParticle(EDamageState state)
	{
		// Particles are only relevant for non-dedicated clients
		if (System.IsConsoleApp())
			return;

		if (state == EDamageState.DESTROYED && !m_DstParticle && !m_sDestructionParticle.IsEmpty())
		{
			ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
			spawnParams.Transform[3] = m_vParticleOffset;
			spawnParams.Parent = GetOwner();
			spawnParams.UseFrameEvent = true;
			m_DstParticle = ParticleEffectEntity.SpawnParticleEffect(m_sDestructionParticle, spawnParams);
		}
	}
#endif
}
//---- REFACTOR NOTE END ----
