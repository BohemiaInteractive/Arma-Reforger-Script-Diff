[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableMineComponentClass : SCR_EditableSystemComponentClass
{
}

//! @ingroup Editable_Entities

//! Editable Mine
class SCR_EditableMineComponent : SCR_EditableSystemComponent
{
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		//~ Activate Mine on server one frame later so the SCR_PressureTriggerComponent can initialize
		if (IsServer())
			GetGame().GetCallqueue().CallLater(ActivateMine);
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Make sure the mine is activated so it explodes when a vehicle drives over it
	protected void ActivateMine()
	{
		if (!GetOwner())
			return;
		
		SCR_PressureTriggerComponent pressureTriggerComponent = SCR_PressureTriggerComponent.Cast(GetOwner().FindComponent(SCR_PressureTriggerComponent));
		if (!pressureTriggerComponent)
			return;
		
		SetInstigator(GetAuthorPlayerID());
		pressureTriggerComponent.ActivateTrigger();
		if (pressureTriggerComponent.GetArmingTime() > 0)
			pressureTriggerComponent.ResetTimeout();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsDestroyed()
	{
		//~ If this component exists it is not destroyed
		return false;		
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanDestroy()
	{
		//~ If this component exists it can be destroyed
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Destroy(int editorPlayerID)
	{		
		if (!IsServer())
			return false;
		
		SCR_PressureTriggerComponent pressureTriggerComponent = SCR_PressureTriggerComponent.Cast(GetOwner().FindComponent(SCR_PressureTriggerComponent));
		if (pressureTriggerComponent)
		{
			SetInstigator(editorPlayerID);
			pressureTriggerComponent.TriggerManuallyServer();
			return true;
		}

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetAuthor(int playerID)
	{
		super.SetAuthor(playerID);
		
		SetInstigator(GetAuthorPlayerID());
	}

	//------------------------------------------------------------------------------------------------
	void SetInstigator(int playerID)
	{
		SCR_PressureTriggerComponent pressureTriggerComponent = SCR_PressureTriggerComponent.Cast(GetOwner().FindComponent(SCR_PressureTriggerComponent));
		if (pressureTriggerComponent)
			pressureTriggerComponent.SetInstigator(Instigator.CreateInstigatorGM(playerID));
	}
}