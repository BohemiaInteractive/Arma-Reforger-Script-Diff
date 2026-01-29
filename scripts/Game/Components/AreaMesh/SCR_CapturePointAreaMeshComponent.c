[ComponentEditorProps(category: "GameScripted/Area Mesh", description: "")]
class SCR_CapturePointAreaMeshComponentClass : SCR_BaseAreaMeshComponentClass
{
}

class SCR_CapturePointAreaMeshComponent : SCR_BaseAreaMeshComponent
{
	SCR_SeizingComponent m_SeizingComponent;
	
	//------------------------------------------------------------------------------------------------
	override float GetRadius()
	{
		return m_SeizingComponent.GetRadius();
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{	
		m_SeizingComponent = SCR_SeizingComponent.Cast(GetOwner().FindComponent(SCR_SeizingComponent));
		
		// This should only help with radius setup, do not show in play mode.
		if (GetGame().InPlayMode())
			return;
		
		if (!m_SeizingComponent)
		{
			Debug.Error2("SCR_CapturePointAreaMeshComponent", "SCR_SeizingComponent not found on owner entity!");
			return;
		}
		
		GenerateAreaMesh();
	}
}
