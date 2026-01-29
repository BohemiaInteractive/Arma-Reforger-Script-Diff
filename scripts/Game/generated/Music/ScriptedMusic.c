/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Music
\{
*/

class ScriptedMusic: Music
{
	//! Gets the sound event name
	proto external string GetName();
	//! Sets the name of the SoundEvent that will be played on evaluation/playing of the music.
	proto external void SetName(string name);

	// callbacks

	//! Evaluation on whether the sound should play (upon automatic evaluation of music manager)
	event bool ShouldPlay();
	//! Initialize called from MusicManager::EOnInit
	event void Init();
	//! Update called from MusicManager::EOnFrame
	event void Update(float dt);
	//! Delete called from MusicManager::OnDelete
	event void OnDelete();
	//! Callback that is called upon starting to play this music
	event void OnPlay();
	//! Callback that is called when this music has stopped playing
	event void OnStop();
}

/*!
\}
*/
