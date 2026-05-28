// // The BlenderRestAPI class provides a way to communicate with Blender from Workbench using a REST API. 
// // See BlenderEndpoints.c for info about available endpoints. 
//
// // Example: 
//
// // Create a connection to Blender. 
// // This will handle launching the Blender process and starting the HTTP server. 
// // If a port is not specified, a random port will be used in the range (50000-65000). 
// // Multiple connections can exist at a time. 
// auto connection = BlenderRestAPI.Create();
//
// // Load the specified .blend file. 
// auto open_blend_result = OpenBlendFile(connection, "W:/my_blender_file.blend");
//
// if (!open_blend_result) 
// {
// 	Print("Unable to open blend file", LogLevel.ERROR);
// 	return;
// }
//	
// // Get a list of the objects in the scene 
// array<string> objects = GetObjects(connection);
//
// foreach (string object : objects) { 
// 	auto transform = GetTransform(connection, object);
// 	Print(transform.location);
// 	Print(transform.rotation);
// 	Print(transform.scale);
// }
// connection.Destroy();
// return;



enum BlenderRestAPIState  {
	WAITING = 0,
	SUCCESS = 1,
	ERROR = 2,
};

class BlenderRestAPI
{		
	private void OnRestSuccessCallback(RestCallback cb)
	{   
		m_RestState = BlenderRestAPIState.SUCCESS;
		m_Response = cb.GetData();
	}
	
	private void OnRestErrorCallback(RestCallback cb)
	{   
		m_RestState = BlenderRestAPIState.ERROR;
	}
	
	
	// HTTP API
	
	bool Get(string endpoint, out string data = "")
	{ 
		this.m_Response = "";
		m_RestState = BlenderRestAPIState.WAITING;
		
		m_RestContext.GET(m_RestCallback, endpoint);
		
		while (m_RestState == BlenderRestAPIState.WAITING) {
			Sleep(10);
		}
		
		data = m_Response;
		return m_RestState == BlenderRestAPIState.SUCCESS;
	}
	
	// TODO POST, maybe others? 
	
	
	static BlenderRestAPI Create(int port = -1) 
	{
		string path;
		
		if (!EBTConfigPlugin.GetDefaultBlenderPath(path)) {
			Print("Failed to get Blender path. Make sure you have Blender setup using EBT: Config", LogLevel.ERROR);	
			return null;
		}
		
		// See https://en.wikipedia.org/wiki/Ephemeral_port
		if (port == -1) {
			port = 50000 + Math.RandomInt(0, 15000);
		}
		
		string address = string.Format("http://localhost:%1", port);
	
		string python_command = string.Format("import bpy; bpy.ops.ebt.start_http_server(port=%1)", port);
		string cmd = string.Format("\"%1\" --background --python-expr \"%2\"", path, python_command);
		
		Print(string.Format("Running Blender Instance on %1", address));
			
		auto blender_process = Workbench.RunProcess(cmd);
		
		if (!blender_process || !Workbench.IsRunningProcess(blender_process))
		{
			Print("Unable begin Blender process", LogLevel.ERROR);
			return null;
		}
		
		auto rest_context = GetGame().GetRestApi().GetContext(address);
		
		if (!rest_context)
		{
			Print("Unable to obtain REST context for " + address, LogLevel.ERROR);
			return null;
		}
		
		auto instance = BlenderRestAPI();
		instance.m_BlenderProcess = blender_process;
		instance.m_RestContext = rest_context;
		instance.m_RestContext.SetTimeout(200);
		instance.m_RestCallback = RestCallback();
		instance.m_RestCallback.SetOnSuccess(instance.OnRestSuccessCallback);
		instance.m_RestCallback.SetOnError(instance.OnRestErrorCallback);	
		
		return instance;
	}
	
	void Destroy() {
		if (Workbench.IsRunningProcess(m_BlenderProcess)) {
			Workbench.KillProcess(m_BlenderProcess);
		}
	}
	
	private ProcessHandle    m_BlenderProcess;
	private RestContext      m_RestContext;		
	private ref RestCallback m_RestCallback;
	
	// Response Handling
	private string              m_Response = "";	
	private BlenderRestAPIState m_RestState = BlenderRestAPIState.WAITING; 
}


