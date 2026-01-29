enum EBackendCallbackState
{
	EBCS_PENDING,
	EBCS_SUCCESS,
	EBCS_ERROR,
	EBCS_TIMEOUT
}

class StateBackendCallback : BackendCallback
{
	EBackendCallbackState m_eState = EBackendCallbackState.EBCS_PENDING;
	EApiCode m_eLastError = EApiCode.EACODE_ERROR_OK;
	EStringMatchType m_eLastMatchType = EStringMatchType.ESMT_EQUALS;
	
	override void OnError( int code, int restCode, int apiCode )
	{
		super.OnError(code, restCode, apiCode);
		m_eLastError = apiCode;
		m_eState = EBackendCallbackState.EBCS_ERROR;
	}
	
	override void OnSuccess( int code )
	{
		super.OnSuccess(code);
		m_eState = EBackendCallbackState.EBCS_SUCCESS;
	}
	
	override void OnTimeout()
	{
		super.OnTimeout();
		m_eState = EBackendCallbackState.EBCS_TIMEOUT;
	}
}


class UploadSaveCommand: ScrServerCommand
{
	protected ref StateBackendCallback m_callback;
	
	protected ref ScrServerCmdResult m_result = new ScrServerCmdResult(string.Empty, EServerCmdResultType.PENDING);
	
	override event string GetKeyword()
	{
		return "upload";
	}

	override event protected ref ScrServerCmdResult OnChatClientExecution(array<string> argv, int playerId)
	{
		m_result.m_eResultType = EServerCmdResultType.OK;
		m_result.m_sResponse = string.Empty;
		return m_result;
	}
	
	protected override ref ScrServerCmdResult OnChatServerExecution(array<string> argv, int playerId)
	{
		m_result.m_sResponse = string.Empty;
		
		if (argv.Count() < 2)
		{
			m_result.m_eResultType = EServerCmdResultType.PARAMETERS;
			return m_result;
		}

		m_callback = new StateBackendCallback;
		
		
		//TODO pass the callback
		//GetGame().GetSaveManager().UploadToWorkshop(argv[1]);	
		return m_result;
	}
	
	protected override ref ScrServerCmdResult OnUpdate()
	{
		switch(m_callback.m_eState)
		{
			case EBackendCallbackState.EBCS_SUCCESS: m_result.m_eResultType = EServerCmdResultType.OK; break;
			case EBackendCallbackState.EBCS_PENDING: m_result.m_eResultType = EServerCmdResultType.PENDING; break;
			case EBackendCallbackState.EBCS_TIMEOUT: m_result.m_eResultType = EServerCmdResultType.ERR; m_result.m_sResponse = "Timeout"; break;
			default: m_result.m_eResultType = EServerCmdResultType.ERR; break;			
		}
		return m_result;
	}
	
	override event protected int RequiredChatPermission()
	{
		return EPlayerRole.ADMINISTRATOR | EPlayerRole.SESSION_ADMINISTRATOR | EPlayerRole.GAME_MASTER;
	}
	
	override event bool IsServerSide()
	{
		return true;
	}
}
