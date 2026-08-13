modded class SCR_Timer
{
	//! Get a WorldTimestamp of the current world time
	static WorldTimestamp GetCurrentWorldTime(World world = null)
	{
		Game game = GetGame();
		if (!game)
			return null;
		
		if (!world)
			world = GetGame().GetWorld();
		
		if (!world)
			return null;
		
		return world.GetTimestamp();
	}
	
	//! Convert seconds to milliseconds
	static int ToMilliseconds(float seconds)
	{
		return (seconds * 1000);
	}
	
	//! Convert milliseconds to seconds
	static float ToSeconds(int milliseconds)
	{
		return (float)milliseconds / 1000;
	}
}