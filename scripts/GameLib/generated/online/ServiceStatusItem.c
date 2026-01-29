/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! Service status item
class ServiceStatusItem
{
	private void ServiceStatusItem();
	private void ~ServiceStatusItem();

	/*!
	\brief Name of service
	*/
	proto external string Name();
	/*!
	\brief Status of service
	*/
	proto external string Status();
	/*!
	\brief Message related to service
	*/
	proto external string Message();
	/*!
	\brief Is there any message related to service
	*/
	proto external bool HasMessage();
}

/*!
\}
*/
