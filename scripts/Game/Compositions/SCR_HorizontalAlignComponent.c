[ComponentEditorProps(category: "GameScripted/Compositions", description: "If entity is placed in a slot or converted to editableEntity then the system makes sure that the entity is always horizontally aligned")]
class SCR_HorizontalAlignComponentClass : ScriptComponentClass
{
}

class SCR_HorizontalAlignComponent : ScriptComponent
{
	//Force horizontal alignment in WB
	#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override void _WB_SetTransform(IEntity owner, inout vector mat[4], IEntitySource src)
	{
		owner.GetWorldTransform(mat);
		vector angles = Math3D.MatrixToAngles(mat);
		angles[1] = 0;
		angles[2] = 0;
		Math3D.AnglesToMatrix(angles, mat);
	}
	#endif
}
