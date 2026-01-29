// Script File
class TestPushUserAction : ScriptedUserAction
{
	
	[Attribute("5", UIWidgets.Slider, "Force multiplier (force is equal to mass * multiplier)", "0 100 0.1")]
	float m_fForceMultiplier;	
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent) 
	{
		if (!GetGame().GetWorldEntity())
			return;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		Physics physics = pOwnerEntity.GetPhysics();
		if (physics)
		{
			GenericEntity genEnt = GenericEntity.Cast(pOwnerEntity);
			SCR_InteractableBoxComponent boxComponent = SCR_InteractableBoxComponent.Cast(genEnt.FindComponent(SCR_InteractableBoxComponent));
			if (boxComponent)
			{
				boxComponent.CancelDragAction();
			}
			
			
			vector userOrigin = pUserEntity.GetOrigin();
			vector thisOrigin = pOwnerEntity.GetOrigin();
			vector dir = thisOrigin-userOrigin;
			dir.Normalize();
			physics.ApplyImpulse(dir * physics.GetMass() * m_fForceMultiplier);
		}
	}
	
};