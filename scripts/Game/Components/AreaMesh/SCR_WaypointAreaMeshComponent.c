[ComponentEditorProps(category: "GameScripted/Area Mesh", description: "")]
class SCR_WaypointAreaMeshComponentClass : SCR_BaseAreaMeshComponentClass
{
}

class SCR_WaypointAreaMeshComponent : SCR_BaseAreaMeshComponent
{
	//------------------------------------------------------------------------------------------------
	override float GetRadius()
	{
		AIWaypoint waypoint = AIWaypoint.Cast(GetOwner());
		return waypoint.GetCompletionRadius();
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!owner.IsInherited(AIWaypoint))
		{
			Debug.Error2("SCR_WaypointAreaMeshComponent", "Component must be attached to AIWaypoint!");
			return;
		}
			
		GenerateAreaMesh();
	}
}
