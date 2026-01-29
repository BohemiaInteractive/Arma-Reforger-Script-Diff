/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

sealed class ImageWidget: Widget
{
	/*!
	Switches to another image, if it's loaded
	\param num Number of image.
	\return True when there is such image, false otherwise.
	*/
	proto external bool SetImage(int num);
	//!Returns active image
	proto external int GetImage();
	/*!
	Loads image form image set. When image of this 'num' is already loaded, then is replaced
	by new one.
	\param num Number of image (0...7)
	\param resource ResourceName of image file
	\param imageName Name of an image in the image set
	\param noCache Do not cache the texture
	\return True when image is loaded, false otherwise
	*/
	proto external bool LoadImageFromSet(int num, ResourceName resource, string imageName, bool noCache = false);
	proto external void SetImageTexture(int image, notnull RTTextureWidget texture);
	/*!
	Loads texture as mask used for alpha masking.
	\param resource Resource name of the texture to load
	\return True if texture was successfully loaded, false otherwise
	*/
	proto external bool LoadMaskTexture(ResourceName resource);
	/*!
	Loads image form image set as mask used for alpha masking.
	\param resource ResourceName of image file
	\param imageName Name of an image in the image set
	\return True when image is loaded, false otherwise
	*/
	proto external bool LoadMaskFromSet(ResourceName resource, string imageName);
	/*!
	When WF_CUSTOMUV is set, we can supply our own UV coords instead of computed ones.
	\param uv
	Pointer to array of at least 8 floats (4 corners * 2 floats for u/v)
	*/
	proto external void SetUV(float uv[4][2]);
	/*!
	Sets rotation of widget around defined center
	\param angle Angle in degrees
	*/
	proto external void SetRotation(float angle);
	//! Returns rotation of widget in degrees
	proto external float GetRotation();
	//! Sets pivot around which the widget will rotate
	proto external void SetPivot(float x, float y);
	/*!
	Progress determines which alpha values are opaque using the mask. For progress x,
	pixels with alpha in mask < x will be opaque and alpha in mask > x will be transparent.
	For smooth transition see GetMaskTransitionWidth.
	\return Progress from 0 to 1 representing alpha range which is visible in the mask
	*/
	proto external float GetMaskProgress();
	/*!
	See GetMaskProgress for explanation.
	\param value Expected in range [0; 1]
	*/
	proto external void SetMaskProgress(float value);
	/*!
	Transition width 0.1 and progress 0.2 mean that values in mask lower than progress will be opaque.
	Values higher than (progress + width) will be transparent and values between progress and (progress + width)
	will be smoothly transitioned.
	\return Width of the alpha mask transition representing softness of the transition.
	*/
	proto external float GetMaskTransitionWidth();
	/*!
	See GetMaskTransitionWidth for transition width explanation.
	\param value Expected in range [0; 1]
	*/
	proto external void SetMaskTransitionWidth(float value);
	proto external ImageMaskMode GetMaskMode();
	proto external void SetMaskMode(ImageMaskMode maskMode);
	/*!
	Valid only if the image mask is in range mode
	\return Range of the the mask to show/hide
	*/
	proto external float GetMaskRange();
	//! \see GetMaskRange
	proto external void SetMaskRange(float range);
	proto external float GetSaturation();
	proto external void SetSaturation(float saturation);
	/*!
	Loads image. When image of this 'num' is already loaded, then is replaced
	by new one.
	\param num Number of image (0...7)
	\param resource ResourceName of image file
	\param noCache Do not cache the texture
	\param fromLocalStorage Prevent checking of ResourceName against Resource Database
	\return True when image is loaded, false otherwise
	*/
	proto bool LoadImageTexture(int num, ResourceName resource, bool noCache = false, bool fromLocalStorage = false);
	/*!
	Copy image from screenshot texture data into widget texture. When image of this 'num' is already loaded,
	then is replaced by new one.
	\param num Number of image (0...7)
	\return True when image is copyied, false otherwise
	*/
	proto bool CopyImageTexture(int num, ScreenshotTextureData texData);
	/*!
	get texture data from image
	\param num						Number of image (0...7)
	\param scriptCallback
	*/
	proto bool GetTextureRawData(int num, GetTextureRawDataCallback scriptCallback);
	//! Returns offset of the image shadow. Z coordinate is not used (always 0)
	proto external vector GetShadowOffset();
	proto external void SetShadowOffset(float x, float y);
	proto ref Color GetShadowColor();
	proto external void SetShadowColor(notnull Color color);
	//!Returns size of image
	proto void GetImageSize(int image, out int x, out int y);
	//! Returns user defined size used in "Size to content" scenarios
	proto external vector GetSize();
	//! Sets user defined size used in "Size to content" scenarios
	proto external void SetSize(float width, float height);
}

/*!
\}
*/
