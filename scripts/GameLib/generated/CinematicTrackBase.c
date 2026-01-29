/*
===========================================
Do not modify, this script is generated
===========================================
*/

class CinematicTrackBase: ScriptAndConfig
{
	proto external string GetTrackName();
	proto external string GetSceneName();

	// callbacks

	event void OnInit(World world);
	event void OnFinish();
	event void OnApply(float time);
}
