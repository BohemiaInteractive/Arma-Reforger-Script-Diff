class SCR_WeaponDeployablePart : SCR_AdditionalDeployablePart
{
	[Attribute(defvalue: "1", desc: "Delete magazines when spawned. Iterates throught all muzzle components and deletes all magazines that are found")]
	protected bool m_bDeleteMagazines;

	//------------------------------------------------------------------------------------------------
	//! Override if you would like to do something to the entity that was spawned after deployment
	//! \param[in] spawnedEntity
	override void PostPrefabSpawn(notnull IEntity spawnedEntity)
	{
		if (!m_bDeleteMagazines)
			return;

		array<Managed> muzzles = {};
		spawnedEntity.FindComponents(MuzzleComponent, muzzles);

		IEntity magazine;
		MuzzleComponent muzzle;
		BaseMagazineComponent magComp;
		foreach (Managed entry : muzzles)
		{
			muzzle = MuzzleComponent.Cast(entry);
			if (!muzzle)
				continue;

			magComp = muzzle.GetMagazine();
			if (!magComp)
				continue;

			magazine = magComp.GetOwner();
			RplComponent.DeleteRplEntity(magazine, false);
		}
	}
}