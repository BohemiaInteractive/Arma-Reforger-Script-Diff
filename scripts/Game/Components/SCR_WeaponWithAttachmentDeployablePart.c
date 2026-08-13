[SCR_BaseContainerCountedResourceNameTitle("m_iNumberOfPrefabs", "m_sPrefab", "%1 x %2 with attachment")]
class SCR_WeaponWithAttachmentDeployablePart : SCR_WeaponDeployablePart
{
	[Attribute(desc: "Attachment which is going to attached to the weapon after it is spawned", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	protected ResourceName m_sAttachment;

	//------------------------------------------------------------------------------------------------
	override void PostPrefabSpawn(notnull IEntity spawnedEntity, int authorPlayerId, IEntity user)
	{
		super.PostPrefabSpawn(spawnedEntity, authorPlayerId, user);

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(user);
		if (!character)
			return;

		SCR_WeaponAttachmentsStorageComponent weaponStorage = SCR_WeaponAttachmentsStorageComponent.Cast(spawnedEntity.FindComponent(SCR_WeaponAttachmentsStorageComponent));
		if (!weaponStorage)
			return;

		character.GetCharacterController().GetInventoryStorageManager().TrySpawnPrefabToStorage(m_sAttachment, weaponStorage);
	}
}