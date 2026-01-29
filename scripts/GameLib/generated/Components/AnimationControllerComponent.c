/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class AnimationControllerComponentClass: GenericComponentClass
{
}

class AnimationControllerComponent: GenericComponent
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
	//! Binds float variable and returns it's ID
	proto external int BindFloatVariable(string varName);
	proto external void SetFloatVariable(int varId, float value);
	//! Binds bool variable and returns it's ID
	proto external int BindBoolVariable(string varName);
	proto external void SetBoolVariable(int varId, bool value);
}

/*!
\}
*/
