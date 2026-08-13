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
	//! Returns the number of procedural animation controllers available on this component.
	//! \return Number of procanim controllers.
	proto external int GetControllerCount();
	//! Fills the provided array with bone names assigned to the requested procedural animation controller.
	//! \param ctrlIdx Index of the procedural animation controller.
	//! \param outBoneNames Output array filled with bone names.
	//! \return Number of names written to outBoneNames, or -1 when ctrlIdx is invalid.
	proto external int GetBoneNames(int ctrlIdx, array<string> events);
	//! Fills the provided array with bone slot names defined by the requested procedural animation controller.
	//! \param ctrlIdx Index of the procedural animation controller.
	//! \param outBoneSlotNames Output array filled with bone slot names.
	//! \return Number of names written to outBoneSlotNames, or -1 when ctrlIdx is invalid.
	proto external int GetBoneSlotNames(int ctrlIdx, array<string> events);
	//! Finds the index of a bone slot in the requested procedural animation controller.
	//! \param ctrlIdx Index of the procedural animation controller.
	//! \param name Bone slot name to search for.
	//! \return Bone slot index, or -1 when ctrlIdx is invalid or name is empty.
	proto external int GetBoneSlotIndex(int ctrlIdx, string name);
	//! Fills the provided array with signal names defined by the requested procedural animation controller.
	//! \param ctrlIdx Index of the procedural animation controller.
	//! \param signals Output array filled with signal names.
	proto external void GetSignals(int ctrlIdx, out array<string> signals);
	//! Finds the index of a signal in the requested procedural animation controller.
	//! \param ctrlIdx Index of the procedural animation controller.
	//! \param name Signal name to search for.
	//! \return Signal index, or -1 when ctrlIdx is invalid.
	proto external int GetSignalIndex(int ctrlIdx, string name);
}

/*!
\}
*/
