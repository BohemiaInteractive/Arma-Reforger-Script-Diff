class SCR_AIOpenDoor : AITaskScripted
{
	static const string PORT_DOOR_ENTITY = "DoorEntity";
	static const float ANGLE_EPSILON = 0.01;
	
	[Attribute("0")]
	protected bool m_bOpenSiblingDoors;

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity doorEntity;
		GetVariableIn(PORT_DOOR_ENTITY, doorEntity);
		
		if (!doorEntity)
			return ENodeResult.FAIL;
		
		array<ScriptedUserAction> doorActions = {};
		FindDoorActions(doorEntity, doorActions, m_bOpenSiblingDoors);
		
		if (doorActions.IsEmpty())
			return ENodeResult.FAIL;
		
		IEntity actionPerformer = owner.GetControlledEntity();
		if (!actionPerformer)
			actionPerformer = owner;
		
		bool isOpened = true;

		foreach (ScriptedUserAction action : doorActions)
		{
			IEntity actionOwner = action.GetOwner();
			if (!actionOwner)
				continue;
			DoorComponent doorComp = DoorComponent.Cast(actionOwner.FindComponent(DoorComponent));
			if (!doorComp)
				continue;
			
			float angle = doorComp.GetAngleRange();
			float state = doorComp.GetDoorState();
			
			bool isTotallyOpen = Math.AbsFloat(doorComp.GetAngleRange() - doorComp.GetDoorState()) < ANGLE_EPSILON;
			
			if (!isTotallyOpen && !doorComp.IsOpening())
			{
				action.PerformAction(actionOwner, actionPerformer);			
				isOpened = false
			}
		}
		if (isOpened)
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.RUNNING;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Searches doorEntity and all its siblings (children of same parent) for SCR_DoorUserAction
	static void FindDoorActions(notnull IEntity doorEntity, notnull array<ScriptedUserAction> outActions, bool includeSiblings)
	{
		outActions.Clear();
		
		IEntity doorParent = doorEntity.GetParent();
		if (!doorParent || !includeSiblings)
		{
			// Door is in world root, or we don't care for siblings
			ScriptedUserAction action = FindDoorUserAction(doorEntity);
			if (action)
				outActions.Insert(action);
			return;
		}
		
		IEntity doorSibling = doorParent.GetChildren();
		
		// Door is in some hierarchy, check all children of parent of the door
		while (doorSibling)
		{
			ScriptedUserAction action = FindDoorUserAction(doorSibling);
			if (action)
				outActions.Insert(action);
			
			doorSibling = doorSibling.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_DoorUserAction FindDoorUserAction(notnull IEntity entity)
	{
		ActionsManagerComponent actionsMgr = ActionsManagerComponent.Cast(entity.FindComponent(ActionsManagerComponent));
		
		if (!actionsMgr)
			return null;
		
		array<BaseUserAction> actions = {};
		actionsMgr.GetActionsList(actions);
		foreach (BaseUserAction action : actions)
		{
			SCR_DoorUserAction doorAction = SCR_DoorUserAction.Cast(action);
			if (doorAction)
				return doorAction;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette() { return true; }
	
	protected static override string GetOnHoverDescription() { return "Opens the given door. Can also open its sibling doors"; }
	
	protected static ref TStringArray s_aVarsIn = {	PORT_DOOR_ENTITY };
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
};