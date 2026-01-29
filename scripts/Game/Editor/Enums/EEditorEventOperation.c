/** @ingroup Editor_Entities
*/ 

//! Type of event-processing operation
enum EEditorEventOperation
{
	NONE,
	
	INIT,
	REQUEST_OPEN,
	OPEN,
	REQUEST_CLOSE,
	CLOSE,
	MODE_CHANGE,
	MODE_CREATE,
	MODE_DELETE,
	DELETE
};