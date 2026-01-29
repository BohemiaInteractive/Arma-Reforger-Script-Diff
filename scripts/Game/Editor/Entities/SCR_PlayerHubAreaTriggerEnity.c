[EntityEditorProps(category: "Editor", description: "Trigger entity for Player Hub.", color: "0 0 255 255")]
class SCR_PlayerHubAreaTriggerEntityClass: ScriptedGameTriggerEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_PlayerHubAreaTriggerEntity : ScriptedGameTriggerEntity
{
	protected int m_iWarningTimerDuration = 5;
	
	//Todo: Kill and trigger only works on server side and doesn't properly check if the entity that left the trigger was the player!
	//======================== TIMER COMPLETED/CANCELED ========================\\
	void TimerDone(IEntity playerToKill, bool canceled = false)
	{
		if (!canceled)
		{
			GenericEntity genericEntity = GenericEntity.Cast(playerToKill);
			if (genericEntity)
			{
				SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(genericEntity.FindComponent(SCR_CharacterControllerComponent));
				if (characterController)
					characterController.ForceDeath();
			}
		}
	}
	
	

	
	//****************//
	//OVERRIDE METHODS//
	//****************//
	
	//------------------------------------------------------------------------------------------------
	//! callback - activation - occurs when and entity which fulfills the filter definitions enters the Trigger
	override void OnActivate(IEntity ent)
	{
		//Always canceles timer no matter which hub is entered
		SCR_PlayerHubWarningEditorUIComponent.CancelWarningTimer(this);
	}
	
	//! callback - deactivation - occurs when and entity which was activated (OnActivate) leaves the Trigger
	override void OnDeactivate(IEntity ent)
	{
		//Always start timer if leaving hub (Todo: Breaks if trigger in hub overlaps)
		SCR_PlayerHubWarningEditorUIComponent.SetWarningTimer(m_iWarningTimerDuration, this, ent);
	}
	
	
	
	//************************//
	//CONSTRUCTOR / DESTRUCTOR//
	//************************//
	
	//------------------------------------------------------------------------------------------------
	void SCR_PlayerHubAreaTriggerEntity(IEntitySource src, IEntity parent)
	{
		//SetEventMask(EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_PlayerHubAreaTriggerEntity()
	{
		//Canceles timer (If current timer is from this hub)
		SCR_PlayerHubWarningEditorUIComponent.CancelWarningTimer(this, true);
	}

};
