[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_OrientToSeaNormalContextAction : SCR_SelectedEntitiesContextAction
{
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return selectedEntity != null;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		GenericEntity owner = selectedEntity.GetOwner();
		if (!owner) return;
		
		vector transform[4];
		owner.GetTransform(transform);
		
		//--- Reset pitch and roll, but preserve yaw
		vector angles = Math3D.MatrixToAngles(transform);
		Math3D.AnglesToMatrix(Vector(angles[0], 0, 0), transform);
		
		owner.SetTransform(transform);
	}
};