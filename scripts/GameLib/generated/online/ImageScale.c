/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

sealed class ImageScale
{
	private void ImageScale();
	private void ~ImageScale();

	/*!
	\brief Get Height of the image
	*/
	proto external int Height();
	/*!
	\brief Get Width of the image
	*/
	proto external int Width();
	/*!
	\brief Download image
	*/
	proto external bool Download(BackendCallback callback);
	/*!
	\brief Get file path to the image
	*/
	proto external string Path();
}

/*!
\}
*/
