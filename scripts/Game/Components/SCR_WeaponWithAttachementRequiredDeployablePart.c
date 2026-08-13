[SCR_BaseContainerCountedResourceNameTitle("m_iNumberOfRequiredPrefabs", "m_sPrefab", "one %2 with attachment")]
class SCR_WeaponWithAttachementRequiredDeployablePart : SCR_RequiredDeployablePart
{
	[Attribute(desc: "Required attachement prefab", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	protected ResourceName m_sAttachementPrefab;

	[Attribute(desc: "Localized string used to tell the player which part he is missing when it is not found")]
	protected LocalizedString m_sAttachmentName;

	protected bool m_bWeaponHasAttachment;

	//------------------------------------------------------------------------------------------------
	override bool EvaluateFoundEntity(vector origin, float distanceSqToFoundEntity, notnull BaseContainer foundPrefab, notnull IEntity foundEntity)
	{
		IEntity foundAttachment;
		bool matchesRequirement = IsSearchedWeapon(foundPrefab, foundEntity, foundAttachment);
		if (!matchesRequirement)
			matchesRequirement = IsSearchedAttachment(foundPrefab, foundEntity);

		if (!matchesRequirement)
			return false;

		if (!m_aFoundEntities || m_aFoundEntities.IsEmpty())
		{
			AddFoundEntity(foundEntity);
			if (foundAttachment)
			{
				m_bWeaponHasAttachment = true;
				AddFoundEntity(foundAttachment);
			}

			return true;
		}

		if (foundAttachment || !m_bWeaponHasAttachment)
		{
			if (TryReplaceFoundEntity(foundEntity, origin, distanceSqToFoundEntity, foundAttachment != null) && foundAttachment)
				TryReplaceFoundEntity(foundAttachment, origin, distanceSqToFoundEntity, true);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Tries to replace previously found entity of such type if it is more suitable
	//! \param[in] foundEntity
	//! \param[in] origin of the main deployable part
	//! \param[in] distanceSqToFoundEntity from the main deployable part
	//! \param[in] isAttached if attachment is attached to the weapon
	//! \return true if provided entity was able to replace one that was there before, otherwise false
	protected bool TryReplaceFoundEntity(notnull IEntity foundEntity, vector origin, float distanceSqToFoundEntity, bool isAttached)
	{
		EntityPrefabData prefabData = foundEntity.GetPrefabData();
		bool unmatched = true;
		foreach (int id, IEntity foundElement : m_aFoundEntities)
		{
			if (prefabData != foundElement.GetPrefabData())
				continue;

			unmatched = false;
			if (distanceSqToFoundEntity != 0 && (!isAttached || m_bWeaponHasAttachment) && vector.DistanceSq(foundElement.GetOrigin(), origin) <= distanceSqToFoundEntity)
				continue;

			if (!m_bWeaponHasAttachment)
				m_bWeaponHasAttachment = isAttached;

			ReplaceFoundEntity(foundEntity, id);
			return true;
		}

		if (unmatched)
			AddFoundEntity(foundEntity);

		return unmatched;
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if found entity matches the weapon prefab requirement
	//! \param[in] foundPrefab
	//! \param[in] foundEntity
	//! \param[out] foundAttachment which matches the requirement and is present on found weapon
	//! \return true if found weapon matches this requirement, otherwise false
	protected bool IsSearchedWeapon(notnull BaseContainer foundPrefab, notnull IEntity foundEntity, out IEntity foundAttachment)
	{
		Resource res = Resource.Load(m_sPrefab);
		if (!res.IsValid())
			return false;

		BaseResourceObject resObject = res.GetResource();
		if (!resObject)
			return false;

		BaseContainer requiredPrefab = resObject.ToBaseContainer();
		if (requiredPrefab != foundPrefab)
			return false;

		array<Managed> components = {};
		foundEntity.FindComponents(AttachmentSlotComponent, components);
		AttachmentSlotComponent slot;
		BaseContainer attachmentPrefab;
		IEntity attachment;
		foreach (Managed comp : components)
		{
			slot = AttachmentSlotComponent.Cast(comp);
			if (!slot)
				continue;

			attachment = slot.GetAttachedEntity();
			if (!attachment)
				continue;

			attachmentPrefab = attachment.GetPrefabData().GetPrefab();
			if (attachmentPrefab && IsSearchedAttachment(attachmentPrefab, attachment))
			{
				foundAttachment = attachment;
				break;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if provided entity matches the attachment that is required for this
	//! \param[in] foundPrefab
	//! \param[in] foundEntity
	//! \return true if this is what we are looking for, otherwise false
	protected bool IsSearchedAttachment(notnull BaseContainer foundPrefab, notnull IEntity foundEntity)
	{
		Resource res = Resource.Load(m_sAttachementPrefab);
		if (!res.IsValid())
			return false;

		BaseResourceObject resObject = res.GetResource();
		if (!resObject)
			return false;

		BaseContainer requiredPrefab = resObject.ToBaseContainer();
		return foundPrefab == requiredPrefab;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsRequirementMet(out int difference)
	{
		if (!m_aFoundEntities || m_aFoundEntities.Count() < 2)
			return false;

		bool foundWeapon, foundAttachment;
		ResourceName prefab;
		foreach (IEntity ent : m_aFoundEntities)
		{
			prefab = ent.GetPrefabData().GetPrefabName();
			if (prefab == m_sPrefab)
				foundWeapon = true;
			else if (prefab == m_sAttachementPrefab)
				foundAttachment = true;
		}

		if (!foundAttachment || !foundWeapon)
			difference = m_iNumberOfRequiredPrefabs;
		
		return foundAttachment && foundWeapon;
	}

	//------------------------------------------------------------------------------------------------
	override LocalizedString GetMissingPartName()
	{
		ResourceName prefab;
		foreach (IEntity ent : m_aFoundEntities)
		{
			prefab = ent.GetPrefabData().GetPrefabName();
			if (prefab == m_sPrefab)
				return m_sAttachmentName;
		}

		return m_sPartName;
	}

	//------------------------------------------------------------------------------------------------
	override bool DeleteFoundEntity(int id, notnull IEntity user)
	{
		if (!m_aFoundEntities || !m_aFoundEntities.IsIndexValid(id))
			return false;

		IEntity deletedEntity = m_aFoundEntities[id];
		if (!deletedEntity || deletedEntity.IsDeleted())
		{ // if attachment was attached to the weapon, then it might be already deleted
			m_aFoundEntities.Remove(id);
			return true;
		}

		if (m_bDetachMagazinesWhenUsed)
			SCR_MultiPartDeployableItemComponent.TryRemoveMagazineFromWeapons(deletedEntity, user);

		GetGame().GetCallqueue().CallLater(SCR_EntityHelper.DeleteEntityAndChildren, param1: deletedEntity); // to give the time for the RPC calls to be sent
		m_aFoundEntities.Remove(id);
		return true;
	}
}