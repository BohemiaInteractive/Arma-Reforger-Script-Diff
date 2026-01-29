[ComponentEditorProps(category: "GameScripted/Area Mesh", description: "")]
class SCR_SpawnPointAreaMeshComponentClass : SCR_BaseAreaMeshComponentClass
{
}

class SCR_SpawnPointAreaMeshComponent : SCR_BaseAreaMeshComponent
{
	//------------------------------------------------------------------------------------------------
	override float GetRadius()
	{
		SCR_SpawnPoint spawnPoint = SCR_SpawnPoint.Cast(GetOwner());
		return spawnPoint.GetSpawnRadius();
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!owner.IsInherited(SCR_SpawnPoint))
		{
			Debug.Error2("SCR_SpawnPointAreaMeshComponent", "Component must be attached to SCR_SpawnPoint!");
			return;
		}
		
		GenerateAreaMesh();
	}
}
