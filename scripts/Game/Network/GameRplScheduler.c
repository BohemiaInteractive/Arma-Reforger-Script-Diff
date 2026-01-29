class GameRplSchedulerInsertionCtx: RplSchedulerInsertionCtx
{
	RplIdentity OnlyRelevantTo;
	bool CanBeStreamed;
	bool OwnerStreamOut;
};