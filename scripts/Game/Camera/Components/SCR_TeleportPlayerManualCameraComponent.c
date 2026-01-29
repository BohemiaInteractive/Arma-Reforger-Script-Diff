//! @ingroup ManualCamera

//! Teleports the player
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_TeleportPlayerManualCameraComponent : SCR_BaseManualCameraComponent
{
	//------------------------------------------------------------------------------------------------
	protected void TeleportPlayer()
	{
		if (!IsEnabled() || !GetCameraEntity().GetCameraParam().isManualInputEnabled)
			return;
		
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return;
		
		WorkspaceWidget workspace = game.GetWorkspace();
		if (!workspace)
			return;
	
		BaseWorld world = game.GetWorld();
		if (!world)
			return;
		
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (!player)
			return;
		
		InputManager inputManager = game.GetInputManager();
		if (!inputManager)
			return;
		
		float mouseX = inputManager.GetActionValue("MouseX");
		float mouseY = inputManager.GetActionValue("MouseY");
		
		vector outDir;
		vector startPos = workspace.ProjScreenToWorld(workspace.DPIUnscale(mouseX), workspace.DPIUnscale(mouseY), outDir, world, -1);
		outDir *= 10000;

		TraceParam trace = new TraceParam();
		trace.Start = startPos;
		trace.End = startPos + outDir;
		trace.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		trace.LayerMask = TRACE_LAYER_CAMERA;
		trace.Exclude = player;
		
		if (startPos[1] > world.GetOceanBaseHeight())
			trace.Flags = trace.Flags | TraceFlags.OCEAN;
		
		float traceDis = world.TraceMove(trace, null);
		if (traceDis == 1)
			return;
		
		vector endPos = startPos + outDir * traceDis;
		
		SCR_Global.TeleportLocalPlayer(endPos);
	}
	
//#ifdef WORKBENCH
//	//------------------------------------------------------------------------------------------------
//	override void EOnCameraFrame(SCR_ManualCameraParam param)
//	{
//		if (!param.isManualInputEnabled) return;
//
//		if (GetInputManager().GetActionTriggered("ManualCameraTeleportPlayer"))
//		{
//			TeleportPlayer();
//		}
//	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		GetInputManager().AddActionListener("ManualCameraTeleportPlayer", EActionTrigger.DOWN, TeleportPlayer);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraExit()
	{
		GetInputManager().RemoveActionListener("ManualCameraTeleportPlayer", EActionTrigger.DOWN, TeleportPlayer);
	}
//#endif // WORKBENCH
}
