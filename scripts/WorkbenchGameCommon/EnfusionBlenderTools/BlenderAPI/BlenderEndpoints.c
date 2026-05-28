class RestAPIHelper<JsonApiStruct T > {

	static T Get (BlenderRestAPI connection, string endpoint) {
		string response;
		bool status = connection.Get(endpoint, response);
		
		if (status) 
		{			
			auto struct = new T();
			struct.RegAll();
			struct.ExpandFromRAW(response);
			return struct;
		}
		
		return null;
	}
}

//--------------------------------------------------------------

class GetObjectsResult : JsonApiStruct
{
	ref array<string> content = {};
}

array<string> GetObjects(notnull const BlenderRestAPI connection) 
{
	auto result = RestAPIHelper<GetObjectsResult>.Get(connection, "/get_objects");
	
	if (result) 
	{			
		return result.content;
	}
	
	return null;
}

//--------------------------------------------------------------

class GetTransformResult : JsonApiStruct
{
	vector location;
	vector rotation;
	vector scale;
}

GetTransformResult GetTransform(notnull const BlenderRestAPI connection, string object_name) 
{
	string endpoint = string.Format("/get_transform?name=%1", object_name);
	auto result = RestAPIHelper<GetTransformResult>.Get(connection, endpoint);
	
	if (result) 
	{			
		return result;
	}
	
	return null;
}

//--------------------------------------------------------------

bool OpenBlendFile(notnull const BlenderRestAPI connection, string blend_file) 
{
	string endpoint = string.Format("/open_blend_file?path=%1", blend_file);
	return connection.Get(endpoint);
}

//--------------------------------------------------------------
