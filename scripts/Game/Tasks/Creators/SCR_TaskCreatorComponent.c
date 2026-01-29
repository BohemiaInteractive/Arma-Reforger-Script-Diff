[ComponentEditorProps(category: "GameScripted/Tasks", description: "Component that takes care of auto creating tasks.")]
class SCR_TaskCreatorComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_TaskCreatorComponent : SCR_BaseGameModeComponent
{
	[Attribute(desc: "Array of task creators")]
	protected ref array<ref SCR_TaskCreator> m_aTaskCreators;

	//------------------------------------------------------------------------------------------------
	override void OnGameModeStart()
	{
		RplComponent rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (rplComponent && !rplComponent.IsMaster())
			return;

		foreach (SCR_TaskCreator taskCreator : m_aTaskCreators)
		{
			taskCreator.Init(this);
		}
	}

	override void OnGameEnd()
	{
		RplComponent rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (rplComponent && !rplComponent.IsMaster())
			return;

		foreach (SCR_TaskCreator taskCreator : m_aTaskCreators)
		{
			taskCreator.Deinit();
		}
	}
}
