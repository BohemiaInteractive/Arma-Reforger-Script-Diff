/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

sealed class BackendImage
{
	private void BackendImage();
	private void ~BackendImage();

	/*!
	\brief Get the list of available scales
	*/
	proto external int GetScales(out notnull array<ImageScale> scales);
	/*!
	\brief	Get a downloaded scale by its width
			If no suitable scale is downloaded, it return the best downloaded candidate
	*/
	proto external ImageScale GetScale(int width);
	/*!
	\brief Get a max available scale
	*/
	proto external ImageScale GetMaxScale();
	/*!
	\brief	Get a downloaded scale by its width
			If no suitable scale is downloaded, it return the best downloaded candidate
	*/
	proto external ImageScale GetLocalScale(int width);
}

/*!
\}
*/
