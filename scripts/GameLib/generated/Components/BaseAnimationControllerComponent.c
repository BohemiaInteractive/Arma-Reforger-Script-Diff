/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class BaseAnimationControllerComponent: GenericComponent
{
	//! Rebinding of entity is necessary in case of changed/modified underlying MeshObject
	proto external void RebindEntity(IEntity owner);
	//! Binds anim command and returns it's ID
	proto external int BindCommand(string commandName);
	proto external void CallCommand(int cmdID, int intParam, float floatParam);
	proto external void CallCommand4I(int cmdID, int intParam1, int intParam2, int intParam3, int intParam4, float floatParam);
	//! Binds integer variable and returns it's ID
	proto external int BindIntVariable(string varName);
	proto external void SetIntVariable(int varId, int value);
	proto external int GetIntVariable(int varId);
	//! Binds float variable and returns it's ID
	proto external int BindFloatVariable(string varName);
	proto external void SetFloatVariable(int varId, float value);
	proto external float GetFloatVariable(int varId);
	//! Binds bool variable and returns it's ID
	proto external int BindBoolVariable(string varName);
	proto external void SetBoolVariable(int varId, bool value);
	proto external bool GetBoolVariable(int varId);
	// --- script bindings: attachments ---
	proto external int BindAttachment(string attachmentName);
	proto external int BindAttCommand(int attachmentName, string commandName);
	proto external void CallAttCommand(int attachmentName, int cmdID, int intParam, float floatParam);
	proto external void CallAttCommand4I(int attachmentName, int cmdID, int intParam1, int intParam2, int intParam3, int intParam4, float floatParam);
	proto external int BindAttIntVariable(int attachmentName, string varName);
	proto external void SetAttIntVariable(int attachmentName, int varId, int value);
	proto external int GetAttIntVariable(int attachmentName, int varId);
	proto external int BindAttFloatVariable(int attachmentName, string varName);
	proto external void SetAttFloatVariable(int attachmentName, int varId, float value);
	proto external float GetAttFloatVariable(int attachmentName, int varId);
	proto external int BindAttBoolVariable(int attachmentName, string varName);
	proto external void SetAttBoolVariable(int attachmentName, int varId, float value);
	proto external bool GetAttBoolVariable(int attachmentName, int varId);
	// --- script bindings: output of last animation update ---
	proto external int BindTag(string tagName);
	proto external bool IsTag(int tagId);
	proto external bool IsSecondaryTag(int tagId);
	proto external bool IsTagOrSecondaryTag(int tagId);
	proto external int BindEvent(string eventName);
	proto external bool IsEvent(int eventId);
	//! Get event completion if eventId is present.
	proto bool GetEventCompletion(int eventId, float timeSlice, out float completion);
	//! Get event user data if eventId is present.
	proto bool GetEventUserData(int eventId, out string eventUserString, out int eventUserInt);
}

/*!
\}
*/
