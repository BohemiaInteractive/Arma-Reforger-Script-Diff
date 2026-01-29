[ComponentEditorProps(category: "GameScripted/Area Mesh", description: "")]
class SCR_MilitaryBaseAreaMeshComponentClass : SCR_BaseAreaMeshComponentClass
{
}

class SCR_MilitaryBaseAreaMeshComponent : SCR_BaseAreaMeshComponent
{
	SCR_MilitaryBaseComponent m_MilitaryBaseComponent;
	
	//------------------------------------------------------------------------------------------------
	override float GetRadius()
	{
		return m_MilitaryBaseComponent.GetRadius();
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_MilitaryBaseComponent = SCR_MilitaryBaseComponent.Cast(GetOwner().FindComponent(SCR_MilitaryBaseComponent));
		
		// This should only help with radius setup, do not show in play mode.
		if (GetGame().InPlayMode())
			return;
		
		if (!m_MilitaryBaseComponent)
		{
			Debug.Error2("SCR_MilitaryBaseAreaMeshComponent", "SCR_MilitaryBaseComponent not found on owner entity!");
			return;
		}
		
		GenerateAreaMesh();
	}
}
