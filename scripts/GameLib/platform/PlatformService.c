class PlatformService
{
	// Native C++ methods

	/*!
	Requests profanity filtering on the provided texts.
	\param[in]	textsToFilter	Array of strings on which profanity filtering is being requested.
	\param			callback			Callback object used to notify result.
	\return			Returns true if the request is submitted.
	*/
	proto native bool FilterProfanityAsync(array<string> textsToFilter, ProfanityFilterRequestCallback callback);
	
	/*!
	Requests privilege from platform. Implemented as queue. When the result for
	one type of privilege is received, all requests of that type receive the result.
	\param[privilege] Requested privilege
	\param[callback] Callback object used to signalize result
	\return Returns if the request was successfully queued
	*/
	[Obsolete("Use SocialComponent.RequestSocialPrivilege() or RequestMultiplayerPrivilege()")]
	proto native bool GetPrivilegeAsync(UserPrivilege privilege, PlatformRequestCallback callback);

	/*!
	Checks for given user privilege without invoking any UI.
	\param[privilege] Requested privilege
	\return Returns true if the privilege is granted
	*/
	[Obsolete("Use SocialComponent.IsPrivilegedTo() or IsMultiplayerAllowed()")]
	proto native bool GetPrivilege(UserPrivilege privilege);

	/*!
	Open native web browser on specified url.
	*/
	proto native void OpenBrowser(string url);

	/*!
	Returns \see PlatformKind for this particular executable.
	*/
	proto native PlatformKind GetLocalPlatformKind();
	
	/*!
	Start platform specific activity
	\param		activity SingleplayerActivity struct.
	\return		Returns true if activity successfully started
	*/
	proto native bool StartSingleplayerActivity(string ObjectID);
	
	/*!
	End platform specific activity
	\param		activity SingleplayerActivity struct.
	\param		outcome Outcome of activity notstarted/inprogress/completed
	\return		Returns true if activity successfully started
	*/
	proto native bool EndSingleplayerActivity(string ObjectID, string Outcome);
	
	/*!
	Change avilability of platform specific activity
	\param activities array of activity ids
	\return Returns true if activity successfully started
	*/
	proto native bool ChangeAvailabilitySingleplayerActivity(array<string> activities, array<string> removeActivities);
}
