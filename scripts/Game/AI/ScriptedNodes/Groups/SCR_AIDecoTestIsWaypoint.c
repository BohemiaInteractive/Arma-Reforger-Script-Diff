class SCR_AIDecoTestIsWaypoint : DecoratorTestScripted
{	
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		AIWaypoint waypoint = AIWaypoint.Cast(controlled);		

		return waypoint != null;
	}
};