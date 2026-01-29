/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

/*!
SizeLayoutWidget class
*/
sealed class SizeLayoutWidget: Widget
{
	proto external void EnableWidthOverride(bool enabled);
	proto external void EnableHeightOverride(bool enabled);
	proto external void EnableMinDesiredWidth(bool enabled);
	proto external void EnableMinDesiredHeight(bool enabled);
	proto external void EnableMaxDesiredWidth(bool enabled);
	proto external void EnableMaxDesiredHeight(bool enabled);
	proto external void EnableAspectRatio(bool enabled);
	proto external void SetWidthOverride(float value);
	proto external void SetHeightOverride(float value);
	proto external void SetMinDesiredWidth(float value);
	proto external void SetMinDesiredHeight(float value);
	proto external void SetMaxDesiredWidth(float value);
	proto external void SetMaxDesiredHeight(float value);
	proto external void SetAspectRatio(float value);
	proto external float GetWidthOverride();
	proto external float GetHeightOverride();
	proto external float GetMinDesiredWidth();
	proto external float GetMinDesiredHeight();
	proto external float GetMaxDesiredWidth();
	proto external float GetMaxDesiredHeight();
	proto external float GetAspectRatio();
	proto external bool IsWidthOverride();
	proto external bool IsHeightOverride();
	proto external bool IsMinDesiredWidth();
	proto external bool IsMinDesiredHeight();
	proto external bool IsMaxDesiredWidth();
	proto external bool IsMaxDesiredHeight();
	proto external bool IsAspectRatio();
	/*
	Returns true if during the last update the child's widget had bigger desired width than this widget's width
	Note: the child widget must not be set to Stretch otherwise it's desired size is ignored
	*/
	proto external bool IsChildDesiredWidthClamped();
	/*
	Returns true if during the last update the child's widget had bigger desired height than this widget's height
	Note: the child widget must not be set to Stretch otherwise it's desired size is ignored
	*/
	proto external bool IsChildDesiredHeightClamped();
}

/*!
\}
*/
