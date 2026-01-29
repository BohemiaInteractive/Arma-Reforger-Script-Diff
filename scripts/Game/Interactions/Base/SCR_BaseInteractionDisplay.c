//------------------------------------------------------------------------------------------------
//! Reason for why an action ended. Used in SCR_BaseInteractionDisplay and derived classes.
enum ActionFinishReason
{
	FINISHED,
	INTERRUPTED

	// FINISHED: Action was sucessfuly finished, i.e. duration was fulfilled
	// INTERRUPTED: Action was interrupted, i.e. user stopped performing the action midway
};

//------------------------------------------------------------------------------------------------
class ActionsTuple
{
	//! Array of actions
	ref array<BaseUserAction> param1;
	//! Array of whether actions above can be performed or not (true=can)
	ref array<bool> param2;
	
	void Init()
	{
		param1 = new array<BaseUserAction>();
		param2 = new array<bool>();
	}
};

//------------------------------------------------------------------------------------------------
//! Data container.
class ActionDisplayData
{
	IEntity pUser;
	ref ActionsTuple pActionsData;
	BaseUserAction pSelectedAction;
	UserActionContext pCurrentContext;
};

//------------------------------------------------------------------------------------------------
//! Base class for displaying interactions in the UI
class SCR_BaseInteractionDisplay : SCR_InfoDisplayExtended
{
	//! Override to handle what should happen when an action is performed. Optional method.
	//! \param pUser The entity which started performing this action
	//! \param pPerformedAction The action that was performed
	void OnActionStart( IEntity pUser, BaseUserAction pPerformedAction );
	
	//! Override to handle what should happen when action is being "ticked". Optional method.
	//! \param pUser The entity which started performing this action
	//! \param pPerformedAction The action that is being performed
	//! \param fProgress The progress of the action in seconds
	//! \param fDuration The duration of the action in seconds
	void OnActionProgress( IEntity pUser, BaseUserAction pPerformedAction, float fProgress, float fDuration );

	//! Override to handle what should happen when an action ends. Optional method.
	//! \param pUser The entity which stopped performing this action
	//! \param pFinishedAction The action that was performed
	//! \param eFinishReason The reason for why this action ended
	void OnActionFinish( IEntity pUser, BaseUserAction pFinishedAction, ActionFinishReason eFinishReason );
	
	//! Override to handle what should happen when display is supposed to show when it was previously closed.
	void ShowDisplay();
	
	//! Override to handle what should happen when display is supposed to hide when it was previously shown.
	void HideDisplay();

	//! Data provided by the interaction handler.
	void SetDisplayData(ActionDisplayData data);
};
