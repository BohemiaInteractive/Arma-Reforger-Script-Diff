[ComponentEditorProps(category: "GameScripted/Area Mesh", description: "")]
class SCR_ZoneRestrictionAreaMeshComponentClass : SCR_BaseAreaMeshComponentClass
{
}

class SCR_ZoneRestrictionAreaMeshComponent : SCR_BaseAreaMeshComponent
{
	[Attribute("0", desc: "Wether or not the Mesh area will get the inner zone (warning) or outer zone (death) radius")]
	protected bool GetWarningRadius;

	//------------------------------------------------------------------------------------------------
	//Shows the zone when the players are killed
	override float GetRadius()
	{		
		SCR_EditorRestrictionZoneEntity zoneRestrictor = SCR_EditorRestrictionZoneEntity.Cast(GetOwner());
		if (!zoneRestrictor)
			zoneRestrictor = SCR_EditorRestrictionZoneEntity.Cast(GetOwner().GetParent());			
		
		if (GetWarningRadius)
			return zoneRestrictor.GetWarningZoneRadius();
		else 
			return zoneRestrictor.GetRestrictionZoneRadius();
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		/*if (!owner.IsInherited(SCR_EditorRestrictionZoneEntity))
		{
			Debug.Error2("SCR_ZoneRestrictionAreaMeshComponentClass", "Component must be attached to SCR_EditorRestrictionZoneEntity!");
			return;
		}*/
		
		GenerateAreaMesh();
	}
}
