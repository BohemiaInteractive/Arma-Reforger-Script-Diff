/*
===========================================
Do not modify, this script is generated
===========================================
*/

class WorkshopComment
{
	//! Returns ID of this comment
	proto external int GetId();
	//! Returns text of this comment
	proto external string Content();
	//! Edit comment content
	proto external void Update(string sContent, notnull BackendCallback pCallback);
	proto external void Report(EWorkshopReportType eReport, string sDesc, notnull BackendCallback pCallback);
	//! Delete comment online
	proto external void DeleteComment(notnull BackendCallback pCallback);
}
