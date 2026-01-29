class SCR_SpawnPreload : SimplePreload
{
	static const int MAX_DURATION_MS = 10000;

	static SCR_SpawnPreload PreloadSpawnPosition(vector position, float radius = 500)
	{
		if (g_Game.BeginPreload(g_Game.GetWorld(), position, radius, MAX_DURATION_MS / 1000))
		{
			SCR_SpawnPreload preload = new SCR_SpawnPreload();
			preload.m_iMaxDuration = MAX_DURATION_MS;
			preload.m_Position = position;

			return preload;
		}

		return null;
	}

	void SCR_SpawnPreload()
	{
		Print("SpawnPreload: preload started.", LogLevel.VERBOSE);
	}

	void ~SCR_SpawnPreload()
	{
		Print("SpawnPreload: preload finished (took: " + Math.Round(m_fDuration) + "s).", LogLevel.VERBOSE);
	}
};