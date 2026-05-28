/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Plugins_Persistence_System
\{
*/

sealed class PersistenceDeferredDeserializeTask: Managed
{
	//! Obtained only through System::AddDeferredDeserializeTask.
	private void PersistenceDeferredDeserializeTask();

	//! Complete the deferred task. Once all tasks are completed the original callback will invoke.
	proto external void Complete();
}

/*!
\}
*/
