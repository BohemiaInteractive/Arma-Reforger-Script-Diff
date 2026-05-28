/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_bimage
\{
*/

sealed class BImage: Managed
{
	private void BImage();
	private void ~BImage();

	/*!
	Download the image. Scale and GUID must be set.
	\note If the requested scale or bigger is already downloaded (comparing equal aspect ratios),
	      the downloaded scale will be used instead.
	*/
	proto external void Download(notnull BackendCallback pCallback);
	/*!
	Sets texture data in your ImageWidget by calling \ref ImageWidget.SetImageTexture.
	When image of this 'iNum' is already loaded, then is replaced by new one.
	The image must be ready to use - \ref BImage.IsReady
	\param iNum Number of image (0...7)
	\param pWidget Widget to set the texture in
	*/
	proto external void SetImageTexture(int iNum, notnull ImageWidget pWidget);
	//! Set fit method to download the image with.
	proto external void SetFitMethod(EBImageFitMethod eMethod);
	//! Returns currently set fit method
	proto external EBImageFitMethod GetFitMethod();
	/*!
	Set scale of this image. If the new scale is already downloaded, the image is ready to use ( \ref BImage.IsReady ) and you can
	update your widget by calling \ref BImage.SetImageTexture. Otherwise \ref BImage.Download must be called and you can update your widget after the download.
	*/
	proto external void SetScale(int iWidth, int iHeight);
	//! Returns width of the image
	proto external int GetWidth();
	//! Returns height of the image
	proto external int GetHeight();
	//! Whether this image is downloaded and ready to use
	proto external bool IsReady();
}

/*!
\}
*/
