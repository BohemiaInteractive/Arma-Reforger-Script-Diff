[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityLabel, "m_Label")]
class EditablePrefabsLabel_VehicleType : EditablePrefabsLabel_Base
{
	[Attribute()]
	protected float m_TruckMassValue;

	[Attribute()]
	protected float m_APCArmorValue;

	//------------------------------------------------------------------------------------------------
	override bool GetLabelValid(WorldEditorAPI api, IEntitySource entitySource, IEntityComponentSource componentSource, string targetPath, EEditableEntityType entityType, notnull array<EEditableEntityLabel> authoredLabels, out EEditableEntityLabel label)
	{
		if (entityType != EEditableEntityType.VEHICLE || authoredLabels.Contains(EEditableEntityLabel.VEHICLE_HELICOPTER))
			return false;

		IEntityComponentSource turretComponent = SCR_BaseContainerTools.FindComponentSource(entitySource, TurretComponent);
		IEntityComponentSource vehicleDamageComponent = SCR_BaseContainerTools.FindComponentSource(entitySource, SCR_VehicleDamageManagerComponent);
		if (turretComponent)
		{
			label = EEditableEntityLabel.VEHICLE_TURRET;
			return true;
		}

		if (!vehicleDamageComponent)
			return false;

		BaseContainerList vehicleHitzones = vehicleDamageComponent.GetObjectArray("Additional hit zones");
		if (vehicleHitzones)
		{
			BaseContainer vehicleTotalHitzone = vehicleHitzones.Get(0);
			int hpMax;
			if (vehicleTotalHitzone && vehicleTotalHitzone.Get("HPMax", hpMax) && hpMax >= m_APCArmorValue)
			{
				label = EEditableEntityLabel.VEHICLE_APC;
				return true;
			}
		}

		IEntityComponentSource rigidBodyComponent = SCR_BaseContainerTools.FindComponentSource(entitySource, "RigidBody");

		float vehicleMass;
		if (rigidBodyComponent && rigidBodyComponent.Get("Mass", vehicleMass))
		{
			if (vehicleMass >= m_TruckMassValue)
				label = EEditableEntityLabel.VEHICLE_TRUCK;
			else
				label = EEditableEntityLabel.VEHICLE_CAR;

			return true;
		}

		return false;
	}
}
