/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! New Feed data structure
class NewsFeedItem
{
	private void NewsFeedItem();
	private void ~NewsFeedItem();

	/*!
	\brief Title of news
	*/
	proto external string Title();
	/*!
	\brief Text of news
	*/
	proto external string Excerpt();
	/*!
	\brief News website URL fragment
	*/
	proto external string Slug();
	/*!
	\brief Category type
	*/
	proto external string Category();
	/*!
	\brief Full website URL
	*/
	proto external string URL();
	/*!
	\brief Date & Time
	*/
	proto external string Date();
	/*!
	\brief Path to downloaded image
	*/
	proto external string Path();
}

/*!
\}
*/
