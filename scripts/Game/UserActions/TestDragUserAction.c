// Script File
class TestDragUserAction : ScriptedUserAction
{	
	private SCR_InteractableBoxComponent m_BoxComponent = null;
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent) 
	{
		if (!GetGame().GetWorldEntity())
			return;
		
		GenericEntity genEnt = GenericEntity.Cast(pOwnerEntity);
		m_BoxComponent = SCR_InteractableBoxComponent.Cast(genEnt.FindComponent(SCR_InteractableBoxComponent));
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{		
		if (m_BoxComponent)
		{
			m_BoxComponent.PerformDragAction(pUserEntity);
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (m_BoxComponent)
		{
			outName = m_BoxComponent.GetActionName();
			return true;
		}
		
		else 
			return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!m_BoxComponent)
			return false;
		
		return m_BoxComponent.CanPerformDragAction();
	}
	
};