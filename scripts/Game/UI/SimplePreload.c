class SimplePreload
{
	static const int PRELOAD_FRAME_TIME_MS = 100;
	static const int MIN_DURATION_MS = 1000;
	ref LoadingAnim m_LoadingAnim;
	vector m_Position;
	float m_fDuration;
	int m_iMaxDuration;
	
	void SimplePreload() 
	{
		Print("SimplePreload: preload started.", LogLevel.VERBOSE);
	}
	
	void ~SimplePreload() 
	{
		if (m_LoadingAnim)
			m_LoadingAnim.Hide();
		Print("SimplePreload: preload finished (took: " + Math.Round(m_fDuration) + "s).", LogLevel.VERBOSE);
	}
	
	bool Update(float timeSlice)
	{
		m_fDuration += timeSlice;
		
		if (g_Game.IsPreloadFinished() && (m_fDuration >= (MIN_DURATION_MS * 0.001)))
		{
			return true;
		}
		
		if (m_LoadingAnim)
		{
			float progress = Math.Min(m_fDuration / MIN_DURATION_MS * 0.001, 1.0);
			m_LoadingAnim.Update(timeSlice, progress, progress);
		}
		return false;
	}
	
	static SimplePreload Preload(vector position, float radius = 500, int max_duration_ms = 20000)
	{	
		if (g_Game.BeginPreload(g_Game.GetWorld(), position, radius, max_duration_ms / 1000))
		{
			// there is a lot of resoruces to be loaded, show loading anim
			SimplePreload preload = new SimplePreload();
			preload.m_LoadingAnim = g_Game.CreateLoadingAnim(g_Game.GetWorkspace());
			preload.m_LoadingAnim.Show();
			preload.m_iMaxDuration = max_duration_ms;
			preload.m_Position = position;
			return preload;
		}
		
		return null;
	}
};