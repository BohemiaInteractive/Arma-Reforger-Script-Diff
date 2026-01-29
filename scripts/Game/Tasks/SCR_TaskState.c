//------------------------------------------------------------------------------------------------
[Obsolete()]
enum SCR_TaskState
{
	OPENED,
	FINISHED,
	PROGRESSED,
	UPDATED,
	CANCELLED,
	REMOVED
};

[Obsolete()]
class SCR_TaskStateHelper
{
	protected static ref ParamEnumArray s_ParamEnumArray = SCR_AttributesHelper.ParamFromDescriptions(SCR_TaskState,
		"Task is available;"+
		"Task was completed successfully;"+
		"Task had progress done;"+
		"Task had an update;"+
		"Task was cancelled;"+
		"Task was removed;"
	);

	//------------------------------------------------------------------------------------------------
	//! \return ParamEnumArray to describe the logic operators.
	static ParamEnumArray GetParamInfo()
	{
		return s_ParamEnumArray;
	}
}
