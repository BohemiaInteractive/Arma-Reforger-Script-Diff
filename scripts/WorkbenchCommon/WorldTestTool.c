[WorkbenchPluginAttribute("WorldTest Plugin", "Open Map and run game mode")]
class WorldTestPlugin: WorkbenchPlugin
{
	
	override void RunCommandline()
	{
		ResourceManager rb = Workbench.GetModule(ResourceManager);
		string path;
		rb.GetCmdLine("-world", path);
		
		string wait;
		rb.GetCmdLine("-wait", wait); // if wait param is specified then plugin waits provided amount of seconds before exiting back to edit mode. If value is <0 then plugin will wait for the game to switch back using Game().RequestClose().
		int waitTime = 1000;
		if (wait)
		{
			waitTime = wait.ToInt();
		}
		
		if (TestMap(path, waitTime))
		{
			Workbench.Exit(0);
		}
		else
		{
			Workbench.Exit(-1);
		}
	}
	
	bool WaitForGameMode(WorldEditor we, int timeout = 120000)
	{
		while (we.GetApi() && !we.GetApi().IsGameMode())
		{
			Sleep(50);
			timeout -= 50;
			if (timeout < 0)
				return false;
		}

		if (we.GetApi() == null)
			return false;
		return true;
	}
	
	bool TestMap(string path, int waitTime)
	{
		bool success = false;
		Print("WorldTestPlugin: opening map "+ path, LogLevel.VERBOSE);

		Workbench.OpenModule(WorldEditor);
		WorldEditor we = Workbench.GetModule(WorldEditor);
		
		if (we.SetOpenedResource(path))
		{
			Sleep(300);
			Print("WorldTestPlugin: running game mode", LogLevel.VERBOSE);
			
			we.SwitchToGameMode(false, true);
			WaitForGameMode(we);
			
			if (waitTime < 0)
			{
				while (we.GetApi().IsGameMode())
				{
					Sleep(1000);
				}
			}
			else
			{
				Sleep(waitTime);
				we.SwitchToEditMode();
			}
			Print("WorldTestPlugin: closing game mode", LogLevel.VERBOSE);
			success = true;
			Sleep(100);
			
			we.Close();
		}
		
		if (success)
		{
			Print("WorldTestPlugin: test successful!", LogLevel.VERBOSE);
		}
		else
		{
			Print("WorldTestPlugin: test failed!", LogLevel.ERROR);
		}
		
		return success;
	}
}

