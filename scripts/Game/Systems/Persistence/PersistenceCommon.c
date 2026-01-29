//! Create/Update/Delete callback delegate
void PersistenceStatusDelegate(EPersistenceStatusCode statusCode, Managed context = null);
typedef func PersistenceStatusDelegate;

//! Read callback delegate
void PersistenceResultDelegate(EPersistenceStatusCode statusCode, Managed result, bool isLast, Managed context = null);
typedef func PersistenceResultDelegate;

//! WhenAvailable callback delegate
void PersistenceWhenAvailableTaskDelegate(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context = null);
typedef func PersistenceWhenAvailableTaskDelegate;
