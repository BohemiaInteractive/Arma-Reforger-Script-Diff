void RespawnRequestDelegate(SCR_SpawnRequestComponent requestComponent);
typedef func RespawnRequestDelegate;
typedef ScriptInvokerBase<RespawnRequestDelegate> OnRespawnRequestInvoker;

void RespawnResponseDelegate(SCR_SpawnRequestComponent requestComponent, SCR_ESpawnResult response);
typedef func RespawnResponseDelegate;
typedef ScriptInvokerBase<RespawnResponseDelegate> OnRespawnResponseInvoker;

/// on server this can be called after a reponse is received, because RPC is done in one step.
/// bear that in mind
void CanRespawnRequestDelegate(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data);
typedef func CanRespawnRequestDelegate;
typedef ScriptInvokerBase<CanRespawnRequestDelegate> OnCanRespawnRequestInvoker;

void CanRespawnResponseDelegate(SCR_SpawnRequestComponent requestComponent, SCR_ESpawnResult response, SCR_SpawnData data);
typedef func CanRespawnResponseDelegate;
typedef ScriptInvokerBase<CanRespawnResponseDelegate> OnCanRespawnResponseInvoker;

void RespawnReadyDelegate();
typedef func RespawnReadyDelegate;
typedef ScriptInvokerBase<RespawnReadyDelegate> OnRespawnReadyInvoker;