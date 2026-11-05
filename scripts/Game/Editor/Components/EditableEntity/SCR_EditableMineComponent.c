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

		// if there is no author, then it was spawned by the scenario, and in such case we need to claim that damage was done by the editor, otherwise player will be blamed for suicide in case of death
		bool isGM = !m_Author || m_Author && m_Author.m_bIsGM;
		SetInstigator(GetAuthorPlayerID(), isGM);
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
			bool isGM = GetGame().GetPlayerManager().HasPlayerRole(editorPlayerID, EPlayerRole.GAME_MASTER);
			SetInstigator(editorPlayerID, isGM);
			pressureTriggerComponent.TriggerManuallyServer();
			return true;
		}

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetAuthor(int playerID, bool isGM = false)
	{
		super.SetAuthor(playerID, isGM);

		int id = GetAuthorPlayerID();
		SetInstigator(id, isGM);
	}

	//------------------------------------------------------------------------------------------------
	void SetInstigator(int playerID, bool isGM = false)
	{
		SCR_PressureTriggerComponent pressureTriggerComponent = SCR_PressureTriggerComponent.Cast(GetOwner().FindComponent(SCR_PressureTriggerComponent));
		if (!pressureTriggerComponent)
			return;

		if (isGM)
			pressureTriggerComponent.SetInstigator(Instigator.CreateInstigatorGM(playerID));
		else
			pressureTriggerComponent.GetInstigator().SetInstigatorByPlayerID(playerID);
	}
}