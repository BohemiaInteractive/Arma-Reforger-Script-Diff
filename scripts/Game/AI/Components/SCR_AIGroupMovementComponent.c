class SCR_AIGroupMovementComponentClass : AIGroupMovementComponentClass
{}

// includes debugs of the subgroups
class SCR_AIGroupMovementComponent : AIGroupMovementComponent
{
	//---------------------------------------------------------------------------------------------------
	void DiagDrawSubgroups(SCR_AIGroup myGroup)
	{
		int bgColors[8] = {
			Color.DARK_RED,
			Color.DARK_GREEN,
			Color.DARK_BLUE,
			Color.DARK_CYAN,
			Color.DARK_MAGENTA,
			Color.DARK_YELLOW,
			Color.DODGER_BLUE,
			Color.VIOLET
		};
		const int bgColorsCount = 8;
		
		array<AIAgent> members = {};
		myGroup.GetAgents(members);
		
		foreach (AIAgent agent : members)
		{
			IEntity e = agent.GetControlledEntity();
			if (!e)
				continue;
			int handleId = GetAgentMoveHandlerId(agent);
			int bgColor; 
			vector textPos = e.GetOrigin() + Vector (0, 0.5, 0);
			string strFtType = myGroup.ToString().Substring(8,11);
			string text;
			if (handleId == DEFAULT_HANDLER_ID)
			{
				text = string.Format("%1 default", strFtType);
				bgColor = 0x00000000;
			}	
			else
			{
				text = string.Format("%1 SubGr: %2", strFtType, handleId);
				bgColor= bgColors[handleId % bgColorsCount];
			}	
			if (GetHandlerLeaderAgent(handleId) == agent)
				text += " L";
			
			DebugTextWorldSpace.Create(GetGame().GetWorld(), text, DebugTextFlags.ONCE | DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA,
				textPos[0], textPos[1], textPos[2], color: Color.WHITE, bgColor: bgColor,
				size: 13.0);
		}
	}
}