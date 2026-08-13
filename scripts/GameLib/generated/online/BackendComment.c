/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

class BackendComment: Managed
{
	private void BackendComment();
	private void ~BackendComment();

	/*!
	Add a reaction to this comment.
	\note If the current user has already reacted to the comment, their reaction will be replaced.
	*/
	proto external void AddReaction(notnull BackendCallback callback, EBackendCommentReaction reaction);
	//! Remove user's reaction from this comment.
	proto external void RemoveReaction(notnull BackendCallback callback);
	/*!
	Block or unblock the comment.
	\param callback Script callback, where you will receive the result
	\param block If true, this will block the comment, false will unblock
	*/
	proto external void SetBlocked(notnull BackendCallback callback, bool block);
	/*!
	Create a reply to this comment.
	\note Replies cannot be created for comments that are already replies.
	      In such cases, the reply must be created for the top-level comment instead.
	*/
	proto external void Reply(notnull BackendCallback callback, string text);
	/*!
	Delete this comment. The current user must be the comment's author.
	\note Deleted comment will have its text replaced by empty string. All its replies remain visible.
	*/
	proto external void Delete(notnull BackendCallback callback);
	//! Returns object containing data about the author of the comment
	proto external WorkshopUserProfile GetAuthor();
	//! Returns true if this comment was deleted
	proto external bool IsDeleted();
	//! Returns true if the current user is author of this comment
	proto external bool IsAuthor();
	//! Returns who was this comment deleted by. If this comment was not deleted, returns NONE
	proto external EBackendCommentDeletedBy GetDeletedBy();
	/*!
	Returns the comment's thread catalogue, which lists replies to this comment.
	\note Only one instance of the thread catalogue exists. Therefore only a single thread can be viewed at a time.
	      Requesting this catalogue for a different comment clears the previously loaded thread.
	*/
	proto external BackendCommentCatalogue GetThreadCatalogue();
	//! Returns preview of replies for this comment
	proto external void GetRepliesPreview(notnull array<BackendComment> outReplies);
	//! Returns true if this comment is a reply to another comment, false otherwise
	proto external bool IsReply();
	//! Returns user's reaction to this comment
	proto external EBackendCommentReaction GetReaction();
	//! Returns text of the comment
	proto external string GetText();
	//! Returns true is the user has blocked the comment, false otherwise
	proto external bool IsBlocked();
	//! Returns total count of likes
	proto external int GetLikeCount();
	//! Returns total count of dislikes
	proto external int GetDislikeCount();
	//! Returns total count of replies
	proto external int GetRepliesCount();
}

/*!
\}
*/
