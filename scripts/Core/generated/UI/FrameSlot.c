/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

class FrameSlot
{
	private void FrameSlot();
	private void ~FrameSlot();

	static proto void GetOffsets(notnull Widget widget, out float left, out float top, out float right, out float bottom);
	static proto float GetPosX(notnull Widget widget);
	static proto vector GetPos(notnull Widget widget);
	static proto float GetPosY(notnull Widget widget);
	static proto vector GetSize(notnull Widget widget);
	static proto float GetSizeX(notnull Widget widget);
	static proto float GetSizeY(notnull Widget widget);
	static proto vector GetAnchorMin(notnull Widget widget);
	static proto vector GetAnchorMax(notnull Widget widget);
	static proto vector GetAlignment(notnull Widget widget);
	static proto bool IsSizeToContent(notnull Widget widget);
	static proto void SetOffsets(notnull Widget widget, float left, float top, float right, float bottom);
	static proto void SetPos(notnull Widget widget, float x, float y);
	static proto void SetPosX(notnull Widget widget, float x);
	static proto void SetPosY(notnull Widget widget, float y);
	static proto void SetSize(notnull Widget widget, float x, float y);
	static proto void SetSizeX(notnull Widget widget, float x);
	static proto void SetSizeY(notnull Widget widget, float y);
	//! Same as calling following two: SetAnchorMin(w, x, y); SetAnchorMax(w, x, y);
	static proto void SetAnchor(notnull Widget widget, float x, float y);
	static proto void SetAnchorMin(notnull Widget widget, float x, float y);
	static proto void SetAnchorMax(notnull Widget widget, float x, float y);
	static proto void SetAlignment(notnull Widget widget, float x, float y);
	static proto void SetSizeToContent(notnull Widget widget, bool isSizeToContent);
	/*!
	Moves given widget by given amount.
	\param widget Widget to move, must have a FrameSlot
	\param xOffset Offset added to widget's current x coordinate
	\param yOffset Offset added to widget's current y coordinate
	*/
	static proto void Move(notnull Widget widget, float xOffset, float yOffset);
}

/*!
\}
*/
