/*!
Localization key status.
*/
enum EStringTableStatus
{
	DEVELOPMENT_PENDING, ///< Developers are working on the text
	DEVELOPMENT_DONE, ///< Developers finished the text and it's ready to be proofread
	PROOFREADING_PENDING, ///< Proofreaders are working on the text
	PROOFREADING_DONE, ///< Proofreaders processed the text and it's ready to be translated
	TRANSLATION_PENDING, ///< Translators are working on the text
	TRANSLATION_DONE ///< Text is fully translated and ready for release or another round of editing
};