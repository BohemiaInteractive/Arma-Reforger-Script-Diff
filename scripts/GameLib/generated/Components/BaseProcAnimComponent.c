/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class BaseProcAnimComponentClass: GenericComponentClass
{
}

class BaseProcAnimComponent: GenericComponent
{
	proto external int GetControllerCount();
	proto external int GetBoneNames(int ctrlIdx, array<string> events);
	proto external int GetBoneSlotNames(int ctrlIdx, array<string> events);
	proto external int GetBoneSlotIndex(int ctrlIdx, string name);
	proto external void GetSignals(int ctrlIdx, out array<string> signals);
	proto external int GetSignalIndex(int ctrlIdx, string name);
}

/*!
\}
*/
