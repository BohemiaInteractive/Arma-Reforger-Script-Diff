/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_workshop
\{
*/

sealed class WorkshopAssetTag
{
	private void WorkshopAssetTag();
	private void ~WorkshopAssetTag();

	/*!
	Returns name of the asset tag.
	\note this can serve also as a key for tags which are prepared by us
	      and should be localized. Custom tags made by community should
	      use this value because they cannot be localized.
	*/
	proto external string GetName();
}

/*!
\}
*/
