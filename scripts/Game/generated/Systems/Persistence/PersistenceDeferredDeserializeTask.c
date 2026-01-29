/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Systems_Persistence
\{
*/

sealed class PersistenceDeferredDeserializeTask: Managed
{
	//! Obtained only through PersistenceSystem::AddDeferredDeserializeTask.
	private void PersistenceDeferredDeserializeTask();

	//! Complete the deferred task. Once all tasks are completed the original callback will invoke.
	proto external void Complete();
}

/*!
\}
*/
