/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_workshop
\{
*/

sealed enum WorkshopInitState
{
	//! Workshop is initializing data and any systems or UI elements should wait until ready
	INIT,
	//! Workshop init failed and retry will be attempted - semi-permanent state to indicate issues with INIT until success
	INIT_RETRY,
	//! Workshop is ready and can be used by other systems and UI - permanent state until application restart
	READY,
	//! Workshop is disabled and any systems or UI elements should be also disabled - permanent state until application restart
	DISABLED,
}

/*!
\}
*/
