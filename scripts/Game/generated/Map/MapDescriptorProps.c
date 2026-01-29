/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Map
\{
*/

class MapDescriptorProps
{
	// todo: Api to set sizes and colors
	// todo: WB props

	/*!
	\brief De/Active (local properties)
	*/
	proto external void Activate( bool enable );
	proto external void Setup( bool enable );
	proto external bool IsVisible();
	proto external void SetVisible( bool enable );
	proto external bool IsIconVisible();
	proto external void SetIconVisible( bool enable );
	proto void SetFont(ResourceName fontPath);
	proto external bool IsTextVisible();
	proto external void SetTextVisible( bool enable );
	proto external void SetTextOffset(vector offset);
	proto external void SetTextOffsetX(float x);
	proto external void SetTextOffsetY(float y);
	proto external void SetIconTextAlignH(float h);
	proto external void SetIconTextAlignV(float v);
	proto external void SetAlignWithParent(bool b);
	proto external void SetTextBold();
	proto external void SetTextItalic();
	/*!
	\brief Set icon scale
	*/
	proto external void SetIconSize( float size, float minScale, float maxScale );
	/*!
	\brief Get icon scale
	*/
	proto external float GetIconSize();
	/*!
	\brief Set text/ description scale
	*/
	proto external void SetTextSize( float size, float minScale, float maxScale );
	/*!
	\brief Get text/ description scale
	*/
	proto external float GetTextSize();
	/*!
	\brief Set tesselation level detail 1 - 255
	*/
	proto external void SetDetail( int iDetail );
	/*!
	\brief Get tesselation level detail
	*/
	proto external int GetDetail();
	/*!
	\brief Get current image def
	*/
	proto external string GetImageDef();
	/*!
	\brief Set image def
	*/
	proto external void SetImageDef( string name );
	/*!
	\brief Set text angle
	*/
	proto external void SetTextAngle(float angle);
	/*!
	\brief Get text angle
	*/
	proto external float GetTextAngle();
	/*!
	\brief Set group type
	*/
	proto external void SetGroupType(int type);
	/*!
	\brief Get group type
	*/
	proto external int GetGroupType();
	/*!
	\brief Set group scale
	*/
	proto external void SetGroupScale(float scale);
	/*!
	\brief Get group scale
	*/
	proto external float GetGroupScale();
	/*!
	\brief Set display priority
	*/
	proto external void SetPriority(int priority);
	/*!
	\brief Get display priority
	*/
	proto external float GetPriority();
	/*!
	\brief Set Front color
	*/
	proto external void SetFrontColor( notnull Color color );
	/*!
	\brief Get Front color
	*/
	proto ref Color GetFrontColor();
	/*!
	\brief Set Background color
	*/
	proto external void SetBackgroundColor( notnull Color color );
	/*!
	\brief Get Background color
	*/
	proto ref Color GetBackgroundColor();
	/*!
	\brief Set Outline color
	*/
	proto external void SetOutlineColor( notnull Color color );
	/*!
	\brief Get Outline color
	*/
	proto ref Color GetOutlineColor();
	/*!
	\brief Set Text color
	*/
	proto external void SetTextColor( notnull Color color );
	/*!
	\brief Get Text color
	*/
	proto ref Color GetTextColor();
}

/*!
\}
*/
