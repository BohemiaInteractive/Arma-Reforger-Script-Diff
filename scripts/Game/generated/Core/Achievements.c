/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Core
\{
*/

/*!
Scripted API to access platform's achievements.
Calls to this API may fail if achievements are not properly configured
on used platform or if the platfrom encounters an error (see runtime logs)
*/
sealed class Achievements
{
	private void Achievements();
	private void ~Achievements();

	/*!
	Unlocks the given achievement.
	Fails if game is not running vanilla addons only.
	\return True on success
	*/
	static proto bool UnlockAchievement(AchievementId achievement);
	/*!
	Increments the given stat . Cannot decrement
	Achievements are automatically unlocked if the maximum value is reached.
	Fails if game is not running vanilla addons only.
	\param value Value greater then 0 to be added to tracked stat.
	\return True on success
	*/
	static proto bool IncrementAchievementProgress(AchievementStatId stat, int value);
}

/*!
\}
*/
