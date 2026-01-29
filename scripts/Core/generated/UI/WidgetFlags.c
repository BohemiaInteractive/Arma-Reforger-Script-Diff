/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

//!Widget flags. See enf::Widget::SetFlags()
enum WidgetFlags
{
	//!When set, widget and his children are visible
	VISIBLE,
	//!Children will be clipped by regions of this widget.
	CLIPCHILDREN,
	//!Inherits WF_CLIPCHILDREN setting from its parent
	INHERIT_CLIPPING,
	//! Clip by rectangle given by some of the previous widgets.
	DO_NOT_CLIP_RECT,
	//! Indicates that a widget is dirty and should be updated
	DIRTY,
	//! Widget will ignore query for widget which pointer is pointing to.
	IGNORE_CURSOR,
	//! Inherits transparency from parent Widget
	INHERIT_COLOR,
	//! UI Widget is not enabled
	DISABLED,
	//! UI Widget will never get focus
	NOFOCUS,
	//! Indicates that a widget position & size is rounded to always be a whole number
	NO_PIXEL_PERFECT,
	//!Widget will be alpha-blended
	BLEND,
	//!Alpha-blending will be additive
	ADDITIVE,
	//!Rendering of image shadow, INTERNAL USE ONLY
	SHADOW,
	//!Image will not be wrapped.
	NOWRAP,
	//!Image will not be bilinear filtered
	NOFILTER,
	//!Image will be used as big, as widget is and will be stretched accordingly.
	STRETCH,
	//!Image will be horizontally flipped. Used by enf::ImageWidget and enf::TextWidget
	FLIPU,
	//! Image will be vertically flipped. Used by enf::ImageWidget and enf::TextWidget
	FLIPV,
	//!UV mapping will be externally set. Usable for special mappings as rotation etc. Used by enf::ImageWidget and enf::TextWidget
	CUSTOMUV,
	//!Text will be centered
	CENTER,
	//!Text will be vertically centered
	VCENTER,
	//!Text will be right aligned
	RALIGN,
	//!Text will be wrapped on horizontal overflow
	WRAP_TEXT,
	//!Text will not be parsed for string IDs
	NO_LOCALIZATION,
}

/*!
\}
*/
