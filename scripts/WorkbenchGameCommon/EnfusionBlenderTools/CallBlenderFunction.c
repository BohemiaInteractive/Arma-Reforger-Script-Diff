#ifdef WORKBENCH


// returns hex string in #ffffffff format
static string ColorToHexString(Color color)
{
    const string hexChars = "0123456789abcdef";
	
	int value = color.PackToInt();
	
    string hexString = "#";
        
    for (int i = 7; i >= 0; i--) {
        int hexDigit = (value >> i * 4) & 0xF;
        hexString += hexChars[hexDigit];
    }

    return hexString;
};

// returns color from given hex string
static Color HexStringToColor(string hexString)
{
	const string hexChars = "0123456789abcdef";
	
	int lookupTable[128] = {0}; // create hex digit lookup table from ascii table
	
	for (int i = "0".ToAscii(); i <= "9".ToAscii(); i++)
	{
		lookupTable[i] = i - "0".ToAscii();
	}
	
	for (int i = "a".ToAscii(); i <= "f".ToAscii(); i++)
	{
		lookupTable[i] = 10 + (i - "a".ToAscii());
	}
	
	int packedInt = 0;
	
	hexString.ToLower();
	hexString = hexString.Substring(1, 8);
	
	for (int i = 0; i < 8; i++)
	{
		int c = hexString[i].ToAscii();
		packedInt = (packedInt << 4) | lookupTable[c];
	}
	
	return Color.FromInt(packedInt);
}

class BlenderOperatorDescription : JsonApiStruct
{
	string moduleName;
	string blIDName;
	
	ref map<string, int> intParams = new map<string, int>;
	ref map<string, float> floatParams = new map<string, float>;
	ref map<string, bool> boolParams = new map<string, bool>;
	ref map<string, Color> colorParams = new map<string, Color>;
	ref map<string, string> stringParams = new map<string, string>;
	
	void BlenderOperatorDescription(string _moduleName = "", string _blIDName = "")
	{
		moduleName = _moduleName;
		blIDName = _blIDName;
	}
	
	void AddParam(string paramName, int value) 	{ intParams.Insert(paramName, value); 		}
	void AddParam(string paramName, float value) 	{ floatParams.Insert(paramName, value); 	}
	void AddParam(string paramName, bool value) 	{ boolParams.Insert(paramName, value); 	}
	void AddParam(string paramName, Color value) 	{ colorParams.Insert(paramName, value); 	}
	void AddParam(string paramName, string value) 	{ stringParams.Insert(paramName, value); 	}
	
	override void OnPack()
	{
		StoreString("module_name", moduleName);
		StoreString("bl_idname", blIDName);
		
		StartObject("parameters");
		{
			foreach(string paramName, int value : intParams)
			{
				StoreInteger(paramName, value);
			}
			foreach(string paramName, float value : floatParams)
			{
				StoreFloat(paramName, value);
			}
			foreach(string paramName, bool value : boolParams)
			{
				StoreBoolean(paramName, value);
			}
			foreach(string paramName, Color value : colorParams)
			{				
				StoreString(paramName, ColorToHexString(value));
			}
			foreach(string paramName, string value : stringParams)
			{
				StoreString(paramName, value);
			}
		}
		EndObject();
	}
}

static void StartBlenderWithOperator(BlenderOperatorDescription operatorDescription, bool runInBakcground = false)
{
	string pathToExecutable;
	if (!EBTConfigPlugin.GetDefaultBlenderPath(pathToExecutable))
		return;
	
	operatorDescription.Pack();
	
	string runInBackgroundCommand = "";
	if (runInBakcground)
		runInBackgroundCommand = " \"-b\"";
	
	string jsonString = operatorDescription.AsString();
	
	jsonString.Replace("\"", "\\*");
	jsonString.Replace("*", "\"");
	
	string cmd = string.Format("\"%1\" %2 --python-expr \"import bpy; bpy.ops.ebt.call_blender_function(description = '%3', check_signature = True)\"", pathToExecutable, runInBackgroundCommand, jsonString);
	if (EBTConfigPlugin.CopyToClipboard())
		System.ExportToClipboard(cmd);
	Workbench.RunProcess(cmd);
}

static void StartBlenderWithServer(bool runInBackground = true)
{
	if (!runInBackground)
	{
		BlenderOperatorDescription operatorDescription = new BlenderOperatorDescription("core");
		operatorDescription.blIDName = "ebt.http_server";
		
		StartBlenderWithOperator(operatorDescription, false);
		return;
	}
	
	string pathToExecutable;
	if (!EBTConfigPlugin.GetDefaultBlenderPath(pathToExecutable))
		return;
	
	string cmd = string.Format("\"%1\" --background --python-expr \"import EnfusionBlenderTools.core.server as s; s.EBT_HTTP_Server.start(background_mode=True); s.EBT_HTTP_Server.background_loop()\"", pathToExecutable); 
	Workbench.RunProcess(cmd);
	
}

#endif