[ComponentEditorProps(category: "GameScripted/Area Mesh", description: "")]
class SCR_TriggerAreaMeshComponentClass : SCR_BaseAreaMeshComponentClass
{
}

class SCR_TriggerAreaMeshComponent : SCR_BaseAreaMeshComponent
{
	//------------------------------------------------------------------------------------------------
	override float GetRadius()
	{
		BaseGameTriggerEntity trigger = BaseGameTriggerEntity.Cast(GetOwner());
		return trigger.GetSphereRadius();
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!owner.IsInherited(BaseGameTriggerEntity))
		{
			Debug.Error2("SCR_TriggerAreaMeshComponent", "Component must be attached to BaseGameTriggerEntity!");
			return;
		}
		
		GenerateAreaMesh();
	}
}
