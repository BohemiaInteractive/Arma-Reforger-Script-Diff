class AnimExportProfilesRequest: JsonApiStruct
{
	string ExportProfileAbsPath;
	
	void AnimExportProfilesRequest()
	{
		RegV("ExportProfileAbsPath");
	}
}

class AnimExportProfilesResponse: JsonApiStruct
{
	ref TxaExporter m_txaExporter;

	override void OnPack()
	{
		StartArray("ProfileNames");
		int numProfiles = m_txaExporter.GetNumProfiles();
		for (int i = 0; i < numProfiles; i++)
		{
			string pn = m_txaExporter.GetProfileName(i);
			ItemString(pn);
		}
		EndArray();
	}
}


class AnimExportProfiles: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new AnimExportProfilesRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		AnimExportProfilesRequest req = AnimExportProfilesRequest.Cast(request);
		AnimExportProfilesResponse response = new AnimExportProfilesResponse();

		response.m_txaExporter = new TxaExporter();
		response.m_txaExporter.LoadProfiles(req.ExportProfileAbsPath);
		return response;
	}

}


class AnimExportProfileChannelsRequest: JsonApiStruct
{
	string ProfileName;
	
	void AnimExportProfileChannelsRequest()
	{
		RegV("ProfileName");
	}
}

class AnimExportProfileChannelsResponse: JsonApiStruct
{
	ref TxaExporter		m_txaExporter;
	int 							m_profileIdx;

	override void OnPack()
	{
		int nChannels = 0;
		if (m_profileIdx != -1)
		{
			nChannels = m_txaExporter.GetProfileChannelCount(m_profileIdx)
		}
		
		StartObject("Channels");

		for (int channelIdx = 0; channelIdx < nChannels; channelIdx++)
		{
			string name = m_txaExporter.GetProfileChannelName(m_profileIdx, channelIdx);
			StartObject(name);
			string genFn = m_txaExporter.GetProfileChannelGenFn(m_profileIdx, channelIdx);
			if (!genFn.IsEmpty())
			{
				StoreString("GenFn", genFn);
			}
			EndObject();
		}

		EndObject();
	}
}

class AnimExportProfileChannels : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new AnimExportProfileChannelsRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		AnimExportProfileChannelsRequest req = AnimExportProfileChannelsRequest.Cast(request);
		AnimExportProfileChannelsResponse response = new AnimExportProfileChannelsResponse();

		response.m_txaExporter = new TxaExporter();
		response.m_profileIdx = response.m_txaExporter.GetProfileIndex(req.ProfileName);
		return response;
	}
}
