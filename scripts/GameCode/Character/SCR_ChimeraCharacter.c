class SCR_ChimeraCharacterClass : ChimeraCharacterClass
{
}

class SCR_ChimeraCharacter : ChimeraCharacter
{
	[NonSerialized()]
	float m_fFaceAlphaTest = 0; // Used to fade away the head when getting close with the 3rd person camera

	[NonSerialized()]
	FactionAffiliationComponent m_pFactionComponent;
	
	[NonSerialized()]
	SCR_CharacterPerceivableComponent m_pPerceivableComponent;
	
	[Attribute(defvalue: "1", desc: "Can this character be recruited by players"), RplProp(), NonSerialized()]
	protected bool m_bRecruitable;
	
	[NonSerialized()]
	protected ref array<IEntity> m_aContacts;
	
	[RplProp(), NonSerialized()]
	protected bool m_bIsRecruited = 0;

	protected static const string SIGNAL_NAME_SPECIAL_CONTACT = "SpecialContact";
	protected static const string SIGNAL_NAME_SPECIAL_ENTITY_HEIGHT = "SpecialContactEntityHeight";

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_pFactionComponent = FactionAffiliationComponent.Cast(FindComponent(FactionAffiliationComponent));
		m_pPerceivableComponent = SCR_CharacterPerceivableComponent.Cast(FindComponent(CharacterPerceivableComponent));
	}

	//------------------------------------------------------------------------------------------------
	Faction GetFaction()
	{
		if (m_pFactionComponent)
			return m_pFactionComponent.GetAffiliatedFaction();

		return null;
	}

	//------------------------------------------------------------------------------------------------
	string GetFactionKey()
	{
		Faction faction = GetFaction();
		if (!faction)
			return string.Empty;

		return faction.GetFactionKey();
	}

	//------------------------------------------------------------------------------------------------
	//! Using RPC here because it is only for sound, so we don't care when weapon is streamed in.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_SetIllumination_BC(bool state, RplId rplId)
	{
		RplComponent sourceRplComp = RplComponent.Cast(Replication.FindItem(rplId));
		if (!sourceRplComp)
			return;

		IEntity soundEntity = sourceRplComp.GetEntity();
		if (!soundEntity)
			return;

		SoundComponent soundComp = SoundComponent.Cast(soundEntity.FindComponent(SoundComponent));
		if (!soundComp)
			return;

		if (state)
			soundComp.SoundEvent(SCR_SoundEvent.SOUND_SCOPE_ILLUM_ON);
		else
			soundComp.SoundEvent(SCR_SoundEvent.SOUND_SCOPE_ILLUM_OFF);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RPC_SetIllumination_S(bool state, RplId rplId)
	{
		RPC_SetIllumination_BC(state, rplId);
		//Broadcast to everybody
		Rpc(RPC_SetIllumination_BC, state, rplId);
	}

	//------------------------------------------------------------------------------------------------
	void SetIllumination(bool illuminated, RplId rplId)
	{
		//Ask the server to broadcast to everybody.
		Rpc(RPC_SetIllumination_S, illuminated, rplId);
	}

	//------------------------------------------------------------------------------------------------
	//! Using RPC here because it is only for sound, so we don't care when weapon is streamed in.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_SetNewZoomLevel_BC(int zoomLevel, bool increased, RplId rplId)
	{
		WeaponComponent wpnComp = WeaponComponent.Cast(Replication.FindItem(rplId));
		if (wpnComp)
		{
			IEntity wpnEntity = wpnComp.GetOwner();
			if (wpnEntity)
			{
				WeaponSoundComponent wpnSoundComp = WeaponSoundComponent.Cast(wpnEntity.FindComponent(WeaponSoundComponent));
				if (wpnSoundComp)
				{
					if (increased)
						wpnSoundComp.SoundEvent(SCR_SoundEvent.SOUND_SCOPE_ZOOM_IN);
					else
						wpnSoundComp.SoundEvent(SCR_SoundEvent.SOUND_SCOPE_ZOOM_OUT);
				}
			}
		}
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RPC_SetNewZoomLevel_S(int zoomLevel, bool increased, RplId rplId)
	{
		RPC_SetNewZoomLevel_BC(zoomLevel, increased, rplId);
		//Broadcast to everybody
		Rpc(RPC_SetNewZoomLevel_BC, zoomLevel, increased, rplId);
	}

	void SetNewZoomLevel(int zoomLevel, bool increased, RplId rplId)
	{
		Rpc(RPC_SetNewZoomLevel_S, zoomLevel, increased, rplId);
	}

	//------------------------------------------------------------------------------------------------	
	/*!
	Returns true if character is in driver compartment, optionally can check if vehicle is moving fast enough
	\param minSpeedSq Minimum speed squared [m/s]
	*/
	bool IsDriving(float minSpeedSq = -1)
	{
		if (!IsInVehicle())
			return false;

		CompartmentAccessComponent access = GetCompartmentAccessComponent();
		if (!access)
			return false;

		PilotCompartmentSlot pilotSlot = PilotCompartmentSlot.Cast(access.GetCompartment());
		if (!pilotSlot)
			return false;

		Vehicle vehicle = Vehicle.Cast(pilotSlot.GetOwner());
		if (!vehicle)
			return false;

		if (vehicle.GetPilotCompartmentSlot() != pilotSlot)
			return false;

		if (minSpeedSq <= 0)
			return true;

		Physics physics = vehicle.GetPhysics();
		if (!physics)
			return false;

		return physics.GetVelocity().LengthSq() >= minSpeedSq;
	}
	
	//------------------------------------------------------------------------------------------------	
	bool IsRecruitable()
	{
		return m_bRecruitable;
	}
	
	//------------------------------------------------------------------------------------------------	
	void SetRecruitable(bool isRecruitable)
	{
		m_bRecruitable = isRecruitable;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsRecruited()
	{
		return m_bIsRecruited;
	}
	
	//------------------------------------------------------------------------------------------------	
	void SetRecruited(bool recruited)
	{
		m_bIsRecruited = recruited;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! This is called locally by the owner of the character
	override void OnSpecialContactsChagned(notnull array<IEntity> contacts)
	{
		RplComponent characterRplComp = GetRplComponent();
		bool isAuthority = characterRplComp && !characterRplComp.IsProxy();

		RplComponent changedEntityRplComp;
		if (m_aContacts)
		{
			RplId changedEntityRplId;
			for (int i = m_aContacts.Count() - 1; i >= 0; i--)
			{
				if (!m_aContacts.IsIndexValid(i) || contacts.Contains(m_aContacts[i]))
					continue;

				if (!isAuthority)
				{
					if (m_aContacts[i])
						changedEntityRplComp = SCR_EntityHelper.GetEntityRplComponent(m_aContacts[i]);

					if (changedEntityRplComp)
						changedEntityRplId = changedEntityRplComp.Id();
					else
						changedEntityRplId = RplId.Invalid();

					Rpc(Rpc_ContactChanged, changedEntityRplId, false);
				}

				RemoveOldContact(m_aContacts[i]);
			}
		}

		SCR_SpecialCollisionHandlerComponent specialCollisionComponent;
		foreach (IEntity newContact : contacts)
		{
			if (!m_aContacts || !m_aContacts.Contains(newContact))
			{
				specialCollisionComponent = SCR_SpecialCollisionHandlerComponent.Cast(newContact.FindComponent(SCR_SpecialCollisionHandlerComponent));
				if (!specialCollisionComponent)
					continue;//only store and replicate information about important contacts

				if (!isAuthority)
				{
					changedEntityRplComp = SCR_EntityHelper.GetEntityRplComponent(newContact);
					if (changedEntityRplComp)
						Rpc(Rpc_ContactChanged, changedEntityRplComp.Id(), true);
				}
				
				AddNewContact(newContact);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_ContactChanged(RplId changedEntityRplId, bool wasAdded)
	{
		if (RplId.Invalid() == changedEntityRplId)
		{//remove contact with entity that no longer exists
			RemoveOldContact();
			return;
		}

		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(changedEntityRplId));
		if (!rplComp)
			return;

		IEntity changedEntity = rplComp.GetEntity();
		if (!changedEntity)
			return;

		if (wasAdded && (!m_aContacts || !m_aContacts.Contains(changedEntity)))
			AddNewContact(changedEntity);
		else if (!wasAdded && m_aContacts && m_aContacts.Contains(changedEntity))
			RemoveOldContact(changedEntity);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddNewContact(notnull IEntity newContact)
	{
		if (!m_aContacts)
			m_aContacts = {};
		
		m_aContacts.Insert(newContact);
		
		array <SCR_SpecialCollisionDamageEffect> specialDamageEffects = {};
		
		SCR_SpecialCollisionHandlerComponent specialCollisionComponent = SCR_SpecialCollisionHandlerComponent.Cast(newContact.FindComponent(SCR_SpecialCollisionHandlerComponent));
		if (!specialCollisionComponent)
			return;

		SCR_SpecialCollisionHandlerComponentClass data = SCR_SpecialCollisionHandlerComponentClass.Cast(specialCollisionComponent.GetComponentData(newContact));
		if (!data)
			return;

		RplComponent characterRplComp = GetRplComponent();
		bool isAuthority = characterRplComp && characterRplComp.Role() == RplRole.Authority;
		if (isAuthority)
		{
			SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(GetDamageManager());
			if (!damageMgr)
				return;

			data.GetSpecialCollisionDamageEffects(specialDamageEffects);
	
			foreach(SCR_SpecialCollisionDamageEffect effect : specialDamageEffects)
			{
				effect.SetResponsibleEntity(newContact);
				damageMgr.AddSpecialContactEffect(effect);
			}
		}

		specialCollisionComponent.OnContactStart(this);
		SignalsManagerComponent signalsMgr = SignalsManagerComponent.Cast(FindComponent(SignalsManagerComponent));
		if (!signalsMgr)
			return;

		int contactSignalId = signalsMgr.AddOrFindMPSignal(SIGNAL_NAME_SPECIAL_CONTACT, 1, 1);
		if (contactSignalId < 0 || data.GetContactType() < 1)
			return;

		signalsMgr.SetSignalValue(contactSignalId, data.GetContactType());

		int heightSignalId = signalsMgr.AddOrFindMPSignal(SIGNAL_NAME_SPECIAL_ENTITY_HEIGHT, 0.2, 1);
		if (heightSignalId < 0)
			return;

		signalsMgr.SetSignalValue(heightSignalId, specialCollisionComponent.GetContactHeight());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveOldContact(IEntity oldContact = null)
	{
		if (!m_aContacts)
			return;

		m_aContacts.RemoveItemOrdered(oldContact);

		RplComponent characterRplComp = GetRplComponent();
		bool isAuthority = characterRplComp && characterRplComp.Role() == RplRole.Authority;
		if (isAuthority)
		{
			SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(GetDamageManager());
			if (!damageMgr)
				return;
			
			array<ref SCR_PersistentDamageEffect> effects = {};
			damageMgr.GetPersistentEffects(effects);
			
			SCR_SpecialCollisionDamageEffect specialEffect;
			
			foreach (SCR_PersistentDamageEffect effect : effects)
			{
				specialEffect = SCR_SpecialCollisionDamageEffect.Cast(effect);
				if (!specialEffect)
					continue;
				
				if (oldContact == specialEffect.GetResponsibleEntity())
					specialEffect.Terminate();		
			}
		}

		SignalsManagerComponent signalsMgr = SignalsManagerComponent.Cast(FindComponent(SignalsManagerComponent));
		if (!signalsMgr)
			return;

		int contactSignalId = signalsMgr.FindSignal(SIGNAL_NAME_SPECIAL_CONTACT);
		if (contactSignalId < 0)
			return;

		int heightSignalId = signalsMgr.FindSignal(SIGNAL_NAME_SPECIAL_ENTITY_HEIGHT);
		if (m_aContacts.IsEmpty())
		{
			signalsMgr.SetSignalValue(contactSignalId, 0);
			if (heightSignalId >= 0)
				signalsMgr.SetSignalValue(heightSignalId, 0);

			return;
		}

		IEntity lastContact;
		for (int lastId = m_aContacts.Count() - 1; lastId >= 0; lastId--)
		{
			lastContact = m_aContacts[lastId];
			if (lastContact)
				break;

			m_aContacts.Remove(lastId);
		}

		if (!lastContact)
		{//if there is no other contact that should control the signals then we need to clear them
			signalsMgr.SetSignalValue(contactSignalId, 0);
			if (heightSignalId >= 0)
				signalsMgr.SetSignalValue(heightSignalId, 0);

			return;
		}

		SCR_SpecialCollisionHandlerComponent specialCollisionComponent;
		SCR_SpecialCollisionHandlerComponentClass data;
		if (!data)
			return;

		if (oldContact)
		{
			specialCollisionComponent = SCR_SpecialCollisionHandlerComponent.Cast(oldContact.FindComponent(SCR_SpecialCollisionHandlerComponent));
			if (specialCollisionComponent)
			{
				specialCollisionComponent.OnContactEnd(this);
				data = SCR_SpecialCollisionHandlerComponentClass.Cast(specialCollisionComponent.GetComponentData(oldContact));
			}

			if (data && signalsMgr.GetSignalValue(contactSignalId) != data.GetContactType())
				return;//if something changed signal value then we should leave it as is

			if (oldContact && lastContact.GetPrefabData() == oldContact.GetPrefabData())
				return;//if last remaining contact is the same prefab as old contact then there is no reason to change values of signals
		}

		specialCollisionComponent = SCR_SpecialCollisionHandlerComponent.Cast(lastContact.FindComponent(SCR_SpecialCollisionHandlerComponent));
		if (!specialCollisionComponent)
			return;

		data = SCR_SpecialCollisionHandlerComponentClass.Cast(specialCollisionComponent.GetComponentData(lastContact));
		signalsMgr.SetSignalValue(contactSignalId, data.GetContactType());
		if (heightSignalId < 0)
			return;

		float height;
		if (data.GetContactType() != 0)
			height = specialCollisionComponent.GetContactHeight();

		signalsMgr.SetSignalValue(heightSignalId, height);
	}
}
