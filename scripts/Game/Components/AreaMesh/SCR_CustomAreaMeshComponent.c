[ComponentEditorProps(category: "GameScripted/Area Mesh", description: "")]
class SCR_CustomAreaMeshComponentClass : SCR_BaseAreaMeshComponentClass
{
}

class SCR_CustomAreaMeshComponent : SCR_BaseAreaMeshComponent
{
	[Attribute("1")]
	protected float m_fRadius;

	//------------------------------------------------------------------------------------------------
	override float GetRadius()
	{
		return m_fRadius;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		GenerateAreaMesh();
	}
}
