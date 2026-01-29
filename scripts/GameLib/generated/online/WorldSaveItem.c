/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

sealed class WorldSaveItem: WorkshopItem
{
	private void WorldSaveItem();
	void ~WorldSaveItem();

	//! Will save all data into files. If some version already exist then it will be deleted.
	proto external void Save(notnull WorldSaveManifest manifest);
	//! Returns time when item was created at.
	proto external int CreatedAt();
	//! It will fill out manifest with data in item.
	proto external void FillManifest(out notnull WorldSaveManifest manifest);
	/*!
	It will publish manifest of World Save onto the workshop.
	\param manifest - manifest which specifies data about item necessary for upload
	\param pCallback - Is script callback where you will receive result/error when request finishes
	*/
	proto external WorldSaveItem UploadWorldSave(notnull WorldSaveManifest manifest, BackendCallback pCallback);
	/*!
	Creates JPEG data from PixelRawData based on given parameters.
	\param pData - raw data of the image
	\param iWidth - width of the image
	\param iHeight - height of the image
	\param iQuality - between 1-100, higher quality results in a bigger image
	\param iChannels - how many channels to use (in case of JPEG its 4 as CMYK)
	\param bPreview - true if the screenshot should be uploaded as preview, false if it should be added to gallery instead
	\return int - 0 on failure, non-0 on success
	*/
	proto external int SaveJPEG(PixelRawData pData, int iWidth, int iHeight, int iQuality, int iChannels, bool bPreview);
	//! Creates and saves new world save item based on the provided manifest and returns pointer to it.
	static proto WorldSaveItem CreateLocalWorldSave(notnull WorldSaveManifest manifest);
}

/*!
\}
*/
