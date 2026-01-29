#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Playmode Entity Spawner", wbModules: { "WorldEditor" }, shortcut: "Ctrl+Alt+E", awesomeFontCode: 0xF055)]
class SCR_EntitySpawnerPlugin : WorkbenchPlugin
{
	[Attribute(desc: "Prefab of the entity to be spawned.", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	protected ResourceName m_sPrefab;

	[Attribute(desc: "When enabled, previously spawned entity will be deleted upon creation of the new one.")]
	protected bool m_bReplacePrevious;

	[Attribute(desc: "When enabled, the entity will be created on camera position and shot towards cursor.")]
	protected bool m_bShootFromCamera;

	[Attribute(desc: "When enabled, this window won't be shown again when you activate the plugin.\nYou can still access it Plugins > Settings > Entity Spawner.")]
	protected bool m_bDontShowThisAgain;

	protected IEntity m_Entity;

	protected static const string DESCRIPTION = "Spawn prefabs during play mode for easy debugging.\nWorks only when playing in viewport, not in full-screen.";

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (SCR_Global.IsEditMode())
		{
			Print("Cannot spawn entity in editor mode, only in play mode.", LogLevel.WARNING);
			return;
		}

		if (Replication.IsClient())
		{
			Print("Cannot spawn entity on client.", LogLevel.WARNING);
			return;
		}

		//--- Delete previous entity
		if (m_bReplacePrevious && m_Entity)
			SCR_EntityHelper.DeleteEntityAndChildren(m_Entity);

		//--- Find position
		InputManager inputManager = GetGame().GetInputManager();
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		BaseWorld world = GetGame().GetWorld();

		float cursorX, cursorY;
		if (inputManager.IsContextActive("MenuContext"))
		{
			//--- Cursor shown, use pointer position
			cursorX = inputManager.GetActionValue("MouseX");
			cursorY = inputManager.GetActionValue("MouseY");
		}
		else
		{
			//--- Cursor not shown, use center of the screen
			cursorX = workspace.GetWidth() * 0.5;
			cursorY = workspace.GetHeight() * 0.5;
		}

		//--- Trace position under cursor
		vector outDir;
		vector startPos = workspace.ProjScreenToWorld(workspace.DPIUnscale(cursorX), workspace.DPIUnscale(cursorY), outDir, world, -1);
		outDir *= 1000;

		TraceParam trace = new TraceParam();
		trace.Start = startPos;
		trace.End = startPos + outDir;
		trace.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		trace.LayerMask = TRACE_LAYER_CAMERA;

		if (startPos[1] > world.GetOceanBaseHeight())
			trace.Flags = trace.Flags | TraceFlags.OCEAN;

		float traceDis = world.TraceMove(trace, null);
		if (traceDis == 1 && !m_bShootFromCamera)
		{
			Print("Cannot spawn entity, cursor is not pointing at surface.", LogLevel.WARNING);
			return;
		}

		//--- Show configuration dialog
		if (!m_bDontShowThisAgain && !Workbench.ScriptDialog("Configure 'Playmode Entity Spawner' plugin", DESCRIPTION, this))
		{
			Print("User-cancelled", LogLevel.NORMAL);
			return;
		}

		if (!m_sPrefab)
		{
			Print("Cannot spawn entity, prefab not defined. Configure it in Plugins > Settings > Entity Spawner.", LogLevel.WARNING);
			return;
		}

		//--- Spawn entity
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		Math3D.AnglesToMatrix(Vector(outDir.VectorToAngles()[0], 0, 0), spawnParams.Transform);
		if (m_bShootFromCamera)
			spawnParams.Transform[3] = startPos;
		else
			spawnParams.Transform[3] = startPos + outDir * traceDis;

		m_Entity = GetGame().SpawnEntityPrefab(Resource.Load(m_sPrefab), world, spawnParams);

		//--- Shoot!
		if (m_bShootFromCamera)
		{
			Physics physics = m_Entity.GetPhysics();
			if (physics)
				physics.SetVelocity(outDir * 0.1);
		}

		if (m_Entity)
			Print(string.Format("Entity @\"%1\" spawned at %2", m_sPrefab.GetPath(), spawnParams.Transform[3]), LogLevel.NORMAL);
		else
			Print(string.Format("Error when spawning entity @\"%1\" at %2", m_sPrefab.GetPath(), spawnParams.Transform[3]), LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	override void Configure()
	{
		Workbench.ScriptDialog("Configure 'Playmode Entity Spawner' plugin", DESCRIPTION, this);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK", true)]
	protected bool ButtonOK()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}
}
#endif // WORKBENCH
