/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_workshop
\{
*/

/*!
This is interface to track state and control individual asynchronous jobs withing workshop.
State is saved into meta files of the asset which allows persistence between game restarts and continue unfinished jobs.

Jobs which was finished with any result or canceled by user is discarded by the workshop but Script or Native code can store these jobs with strong reference.
This allows to preserve some kind of history with possibility to retry failed or canceled jobs.
*/
sealed class WorkshopJob
{
	private void WorkshopJob();
	//Script is allowed to hold ref to the job to preserve session history of jobs after they finish.
	void ~WorkshopJob();

	//! Returns asset to which this job belongs to.
	proto external WorkshopAssetBase GetAsset();
	/*!
	Returns revision to which this job is currently doing the task.
	\note If job is canceled then return value is swapped with GetPreviousRevision() to revert progress.
	*/
	proto external WorkshopRevisionBase GetRevision();
	/*!
	Returns previous revision for some specific tasks if there was some.
	\note If job is canceled then return value is swapped with GetPreviousRevision() to revert progress.
	*/
	proto external WorkshopRevisionBase GetPreviousRevision();
	/*!
	Returns primary task this job is supposed to do.
	\note This value will not change during entire existence of the job.
	*/
	proto external EWorkshopJobTask GetTask();
	//! Returns in what state the job currently is in.
	proto external EWorkshopJobState GetState();
	/*!
	Return true if job was canceled.
	\note This can return true even if current state is not canceled because some tasks might require
	      to make some reverting operations so it can take time before task is truly finished with canceled state.
	*/
	proto external bool IsCanceled();
	/*!
	Returns primary progress of the jobs process in range 0.0f to 1.0f.
	\warning If Job did not properly begin yet this can return -1.0f due to ongoing calculation.
	*/
	proto external float GetProcessProgress();
	/*!
	Return total amount of data in KB which will be processed by the job.
	\warning If Job did not properly begin yet this can return -1 due to ongoing calculation.
	*/
	proto external int GetProcessTotalKB();
	/*!
	Returns amount of data in KB which was already processed by the job.
	\warning If Job did not properly begin yet this can return -1 due to ongoing calculation.
	*/
	proto external int GetProcessDoneKB();
	//! Returns speed in KB per Second for writing data onto the disk.
	proto external int GetWriteSpeedKBpS();
	/*!
	Returns progress of downloading missing data in range of 0.0f to 1.0f.
	\warning If Job did not properly begin yet this can return -1.0f due to ongoing calculation.
	*/
	proto external float GetDownloadProgress();
	/*!
	Return total amount of missing data in KB which will be downloaded by the job.
	\warning If Job did not properly begin yet this can return -1 due to ongoing calculation.
	*/
	proto external int GetDownloadTotalKB();
	/*!
	Return amount of missing data in KB which was already downloaded by the job.
	\warning If Job did not properly begin yet this can return -1 due to ongoing calculation.
	*/
	proto external int GetDownloadDoneKB();
	//! Returns speed in KB per Second for downloading missing data.
	proto external int GetDownloadSpeedKBpS();
	/*!
	Will pause the job and will not continue until Resume is invoked.
	\warning Original callback which was used to create this will not be invoked.
	         You will have to provide new one (or reuse original) with Resume.
	\note Pause will immediately mark the job as paused and moved out of main processing queue.
	      Upon game restart any previously active/queued or paused job will be recovered in paused state.
	*/
	proto external void Pause();
	/*!
	Will resume the paused job.
	\note Resumed job is always appended at the end of the processing queue.
				You can use SetQueuePriority() right after resume to change the priority of the job.
	*/
	proto external void Resume(notnull BackendCallback callback);
	/*!
	Will change the priority/position in processing queue for the jobs.
	\warning Priority can be changed only for ACTIVE or QUEUED jobs.
	*/
	proto external void SetQueuePriority(int priority);
	/*!
	Returns priority of this job in processing queue.
	*/
	proto external int GetQueuePriority();
	/*!
	Will gracefully cancel job and attempt to revert all changes that were done during active state of the asset.
	job will change to CANCELED state after it finishes reverting.
	- For DOWNLOAD this will revert by deleting all data as there were none before this job.
	- For UPDATE/DOWNGRADE this will revert to previous revision which might take some time until it is fixed.
	\note You can use ForceCancel to immediately cancel the job but with different behavior. Refer to its documentation.
	\warning Original callback which was used to create this will not be invoked and instead will be replaced with one used in this method.
	         You can also reuse the original one used to create this job.
	*/
	proto external void Cancel(notnull BackendCallback callback);
	/*!
	Will forcefully cancel job without attempting to revert any previous changes.
	\warning Canceling some tasks can possibly leave asset in corrupted state and might still require repair to be usable afterwards.
	*/
	proto external void ForceCancel();
	/*!
	Will retry the job after it failed or was canceled.
	\warning Original callback which was used to create this will not be invoked and instead will be replaced with one used in this method.
	         You can also reuse the original one used to create or cancel this job.
	*/
	proto external void Retry(notnull BackendCallback callback);
}

/*!
\}
*/
