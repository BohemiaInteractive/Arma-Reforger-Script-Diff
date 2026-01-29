enum TxaErrCode
{
	TXAERR_OK = 0,
	TXAERR_WARNING = 1,
	TXAERR_ERROR = 2,
};

class AnimExportTrackExportRequest : JsonApiStruct
{
	string	SourceFile;
	string 	TargetFile;
	string	Profile;
	int		NumChannels;
	int		NumKeyframes;
	int		Fps;
	ref array<float> 	KeyData = {};
	ref array<float>	DiffData = {};

	void AnimExportTrackExportRequest()
	{
		RegV("SourceFile");
		RegV("TargetFile");
		RegV("Profile");
		RegV("NumChannels");
		RegV("NumKeyframes");
		RegV("Fps");
		RegV("KeyData");
		RegV("DiffData");
	}
}

class AnimExportTrackExportResponse: JsonApiStruct
{
	string	Result;
	string  ErrorMessage;
	void AnimExportTrackExportResponse()
	{
		RegV("Result");
		RegV("ErrorMessage");
	}
}

class AnimExportTrackExport : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new AnimExportTrackExportRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		AnimExportTrackExportRequest req = AnimExportTrackExportRequest.Cast(request);
		AnimExportTrackExportResponse response = new AnimExportTrackExportResponse();

		TxaExporter txaExporter = new TxaExporter();	
		int profileIdx = txaExporter.GetProfileIndex(req.Profile);
		if (profileIdx == -1)
		{
			response.Result = "ERROR";
			response.ErrorMessage = "Missing export profile";
			return response;
		}
		
		TxaErrCode err = txaExporter.TrackReset(profileIdx, req.NumKeyframes, req.Fps, req.SourceFile, req.TargetFile);
		if (err == TxaErrCode.TXAERR_ERROR)
		{
			response.Result = "ERROR";
			response.ErrorMessage = txaExporter.ErrMsg();
			return response;
		}
		
		err = txaExporter.TrackSetChannels(req.NumChannels, req.NumKeyframes, req.KeyData, req.DiffData);
		if (err == TxaErrCode.TXAERR_ERROR)
		{
			response.Result = "ERROR";
			response.ErrorMessage = txaExporter.ErrMsg();
			return response;
		}
		
		err = txaExporter.TrackExport();
		if (err == TxaErrCode.TXAERR_ERROR)
		{
			response.Result = "ERROR";
			response.ErrorMessage = txaExporter.ErrMsg();
			return response;
		}
		
		response.Result = "OK";
		response.ErrorMessage = "";
		return response;
	}
}
