/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_bans
\{
*/

sealed class ServerBanManagerApi
{
	private void ServerBanManagerApi();
	private void ~ServerBanManagerApi();

	/*!
	Set the server to manage bans on. Must be called before any ban operation
	or ban listing when managing bans from a game client. The operation will also
	verify that the calling player has the required permission to manage bans on
	that server.
	\note Not needed when called directly from a dedicated server.
	*/
	static proto void SetServer(ServerInfo pServer);
	//! Returns the catalogue used to list and page through this server's ban entries.
	static proto ServerBanEntryCatalogue GetServerBans();
	/*!
	Request creation of a new ban. Fill in the manifest with the target, scope action, reason and duration before calling.

	The resulting BanEntry is stored inside the callback and deleted alongside it.
	Retrieve via BackendCallback.GetResultData() cast to BanEntry.

	API codes delivered to the callback:
	 - OnSuccess - ban was created. Retrieve the created BanEntry via BackendCallback.GetResultData() cast to BanEntry.
	 - OnError:
	   - EApiCode::EACODE_ERROR_DUPLICATE_BAN - a ban for this target already exists in the specified ban list.
	     The existing BanEntry is also available via BackendCallback.GetResultData() cast to BanEntry.

	\code
		ref BackendCallback m_CreateBanCallback;

		void CreateBan(ServerBanManifest manifest)
		{
			m_CreateBanCallback = new BackendCallback();
			m_CreateBanCallback.SetOnSuccess(OnCreateBanSuccess);
			m_CreateBanCallback.SetOnError(OnCreateBanError);
			ServerBanManagerApi.RequestCreateBan(m_CreateBanCallback, manifest);
		}

		void OnCreateBanSuccess(BackendCallback callback)
		{
			BanEntry ban = BanEntry.Cast(callback.GetResultData());
			// use the newly created ban entry
		}

		void OnCreateBanError(BackendCallback callback)
		{
			if (callback.GetApiCode() == EApiCode.EACODE_ERROR_DUPLICATE_BAN)
			{
				BanEntry existing = BanEntry.Cast(callback.GetResultData());
				// a ban for this target already exists - existing holds the duplicate entry
			}
		}
	\endcode
	*/
	static proto void RequestCreateBan(notnull BackendCallback pCallback, notnull ServerBanManifest pManifest);
	/*!
	Request removal of one or more bans. Pass the BanEntry objects obtained from a previous listing.
	Each ban is processed independently; a single failure does not prevent others from being removed.
	Results are stored inside the BackendCallback used for the request which will be deleted alongside the callback.
	Retrieve them via BackendCallback.GetResultData() which must be then casted into BanRemoveResult type.
	*/
	static proto void RequestRemoveBan(notnull BackendCallback pCallback, notnull array<BanEntry> bans);
	/*!
	Returns the default ban list configured for this server, or null if none is configured.
	Use when issuing automatic bans where no specific ban list is selected by the caller.
	*/
	static proto BanList GetDefaultBanList();
	//! Fills the output array with all ban lists the server is registered with (owned + shared).
	static proto void GetAllBanLists(out array<BanList> listsOut);
	/*!
	Returns true if the connected player has an active ban on the SERVER system that covers
	"eAction" (direct match or GLOBAL action scope). Returns false if the player is not in
	the cache (not connected or cache not yet populated).
	*/
	static proto bool IsPlayerActionBanned(UUID identityId, EBanScopeAction eAction);
}

/*!
\}
*/
