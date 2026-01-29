#ifdef WORKBENCH
/*
This plugin generates scripted behavior tree node classes.
It parses files, searches for macros in comments and generates the code based on that.
You can adjust parameters in a config file.
If you have more files to generate code from, make sure you add them to the config file.
*/
[WorkbenchPluginAttribute(name: "AI Script Generator", description: "Generates scripted Behavior Tree node classes", wbModules: { "ScriptEditor" })]
class SCR_AiScriptGeneratorPlugin : WorkbenchPlugin
{
	[Attribute(defvalue: "{6741D2D6C8EFBFF9}Configs/AI/AIScriptGeneratorConfig.conf", params: "conf class=SCR_AIScriptGeneratorConfig")]
	ResourceName m_sConfig;

	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		Workbench.ScriptDialog("AI Script Generator Plugin", "", this);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Run")]
	protected void RunButton()
	{
		_print("Plugin started");

		// Open config file
		SCR_AIScriptGeneratorConfig config = SCR_ConfigHelperT<SCR_AIScriptGeneratorConfig>.GetConfigObject(m_sConfig);
		if (!config)
		{
			_print(string.Format("Failed to open config file: %1", m_sConfig), LogLevel.ERROR);
			return;
		}

		int nGeneratedClasses = 0;
		int nGeneratedLines = 0;
		SCR_AiScriptGenerator_Parser parser = new SCR_AiScriptGenerator_Parser();
		SCR_AiScriptGenerator_OutputFormatBuffer bufferSendGoalMessage = new SCR_AiScriptGenerator_OutputFormatBuffer();
		SCR_AiScriptGenerator_OutputFormatBuffer bufferSendInfoMessage = new SCR_AiScriptGenerator_OutputFormatBuffer();
		SCR_AiScriptGenerator_OutputFormatBuffer bufferSendOrder = new SCR_AiScriptGenerator_OutputFormatBuffer();

		// Read and parse all input files
		array<string> fileLines;
		foreach (string inputFilePath : config.m_aInputFiles)
		{
			FileHandle fHandle = FileIO.OpenFile(inputFilePath, FileMode.READ);
			if (!fHandle)
			{
				_print(string.Format("Failed to open input file: %1", inputFilePath), LogLevel.ERROR);
				continue;
			}

			// Read whole file by lines
			fileLines = {};
			string fLine;
			while (fHandle.ReadLine(fLine) != -1)
			{
				fileLines.Insert(fLine);
			}
			fHandle.Close();

			// Parse lines
			parser.ParseLines(fileLines, inputFilePath);
		}

		// Finish parsing
		parser.FindRelatedClasses();
		parser.PrintListing();

		// Run generators for parsed classes
		foreach (SCR_AiScriptGenerator_Class parsedClass : parser.GetParsedClasses())
		{
			foreach (SCR_AiScriptGenerator_ClassGeneratorBase generator : parsedClass.m_aGenerators)
			{
				if (SCR_AiScriptGenerator_SendGoalMessageGenerator.Cast(generator))
					generator.Generate(bufferSendGoalMessage);
				else if (SCR_AiScriptGenerator_SendInfoMessageGenerator.Cast(generator))
					generator.Generate(bufferSendInfoMessage);
				else if (SCR_AiScriptGenerator_SendOrderGenerator.Cast(generator))
					generator.Generate(bufferSendOrder);

				nGeneratedClasses++;
			}
		}

		// Write generated data to files
		WriteLinesToFile(config.m_sSendGoalMessageOutputFile, bufferSendGoalMessage.GetLines());
		WriteLinesToFile(config.m_sSendInfoMessageOutputFile, bufferSendInfoMessage.GetLines());
		WriteLinesToFile(config.m_sSendOrderOutputFile, bufferSendOrder.GetLines());

		// Write some statistics
		nGeneratedLines += bufferSendGoalMessage.GetLines().Count();
		nGeneratedLines += bufferSendInfoMessage.GetLines().Count();
		nGeneratedLines += bufferSendOrder.GetLines().Count();
		_print(string.Format("Generated %1 classes, %2 lines", nGeneratedClasses, nGeneratedLines));
	}

	//------------------------------------------------------------------------------------------------
	protected void WriteLinesToFile(string fileName, array<string> lines)
	{
		FileHandle fHandleOut = FileIO.OpenFile(fileName, FileMode.WRITE);
		if (!fHandleOut)
		{
			_print(string.Format("Error opening output file: %1", fileName));
		}
		else
		{
			foreach (string line : lines)
			{
				fHandleOut.WriteLine(line);
			}
			fHandleOut.Close();
			_print(string.Format("Saved generated scripts to file: %1 (%2 lines)", fileName, lines.Count()));
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] str
	//! \param[in] logLevel
	static void _print(string str, LogLevel logLevel = LogLevel.NORMAL)
	{
		Print(string.Format("[AI Script Generator]: %1", str), logLevel);
	}
}

class SCR_AiScriptGenerator_Parser
{
	const string COMMENT = "//";
	const string CLASS = "class";

	const string MACRO_VARIABLE = "VARIABLE";
	const string NODE_PORT = "NodePort";
	const string NODE_PROPERTY = "NodeProperty";
	const string NODE_PROPERTY_ENUM = "NodePropertyEnum";

	const string MACRO_MESSAGE_CLASS = "MESSAGE_CLASS";
	const string GENERATE_SEND_GOAL_MESSAGE = "GenerateSendGoalMessage";
	const string GENERATE_SEND_INFO_MESSAGE = "GenerateSendInfoMessage";
	const string GENERATE_SEND_ORDER = "GenerateSendOrder";

	// Current state
	protected int m_iCurrentLineId;
	protected string m_sCurrentLine;
	protected string m_sCurrentFile;

	// Current class while parsing
	protected ref SCR_AiScriptGenerator_Class m_CurrentClass;

	protected ref array<ref SCR_AiScriptGenerator_Class> m_aParsedClasses = {};

	//------------------------------------------------------------------------------------------------
	protected void ResetContext(string currentFile)
	{
		m_iCurrentLineId = 0;
		m_sCurrentLine = string.Empty;
		m_sCurrentFile = currentFile;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	array<ref SCR_AiScriptGenerator_Class> GetParsedClasses()
	{
		return m_aParsedClasses;
	}

	//------------------------------------------------------------------------------------------------
	//! Must be called for content of each parsed file
	//! \param[in] lines
	//! \param[in] fileName
	void ParseLines(array<string> lines, string fileName)
	{
		ResetContext(fileName);
		m_CurrentClass = null;

		foreach (int lineId, string line : lines)
		{
			ParseLine(line, lineId);
		}

		// Finish current class, if we are in a class
		if (m_CurrentClass)
			m_aParsedClasses.Insert(m_CurrentClass);
	}

	//------------------------------------------------------------------------------------------------
	//! Must be called after all files have been parsed
	//! Connects related parsed classes together
	void FindRelatedClasses()
	{
		// Iterate all parsed classes and link them to their parents
		foreach (SCR_AiScriptGenerator_Class _class : m_aParsedClasses)
		{
			string parentClassName = _class.m_sParentClassName;
			if (parentClassName.IsEmpty())
				continue;

			// Find a class with this name among classes we've parsed
			foreach (SCR_AiScriptGenerator_Class c : m_aParsedClasses)
			{
				if (c.m_sName == parentClassName)
				{
					_class.m_ParentClass = c;
					break;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Print listing in console
	void PrintListing()
	{
		_print("-----------------------------------------------------------------------------", printContext : false);
		_print("Listing of parsed classes:", printContext : false);
		foreach (SCR_AiScriptGenerator_Class _class : m_aParsedClasses)
		{
			_print(_class.GetListing(), printContext: false);
			foreach (SCR_AiScriptGenerator_Variable _variable : _class.m_aVariables)
			{
				_print("  " + _variable.GetListing(), printContext: false);
			}
			_print("", printContext: false);
		}
		_print("-----------------------------------------------------------------------------", printContext : false);
		_print("", printContext : false);
	}

	//------------------------------------------------------------------------------------------------
	protected void ParseLine(inout string line, int lineId)
	{
		if (!line.Contains(COMMENT))
			return;

		m_iCurrentLineId = lineId;
		m_sCurrentLine = line;

		string comment = ExtractComment(line);
		array<string> macroArgs = {};
		if (ExtractMacro(comment, MACRO_MESSAGE_CLASS, macroArgs))
		{
			// Detected new class

			// If we were already parsing a class, push it to array
			if (m_CurrentClass)
				m_aParsedClasses.Insert(m_CurrentClass);
			m_CurrentClass = null;

			if (!VerifyMacro_MessageClass(macroArgs))
				return;

			string className, parentClassName;
			if (!ExtractClass(line, className, parentClassName))
				return;

			// Create entry for this class
			m_CurrentClass = new SCR_AiScriptGenerator_Class();
			m_CurrentClass.m_sLine = m_sCurrentLine;
			m_CurrentClass.m_sName = className;
			m_CurrentClass.m_sParentClassName = parentClassName;

			if (!macroArgs.IsEmpty())
			{
				switch (macroArgs[0])
				{
					case GENERATE_SEND_GOAL_MESSAGE:
					{
						SCR_AiScriptGenerator_SendGoalMessageGenerator generator = new SCR_AiScriptGenerator_SendGoalMessageGenerator(m_CurrentClass);
						generator.m_sGeneratedClassName = macroArgs[1];
						m_CurrentClass.m_aGenerators.Insert(generator);
						break;
					}
					case GENERATE_SEND_INFO_MESSAGE:
					{
						SCR_AiScriptGenerator_SendInfoMessageGenerator generator = new SCR_AiScriptGenerator_SendInfoMessageGenerator(m_CurrentClass);
						generator.m_sGeneratedClassName = macroArgs[1];
						m_CurrentClass.m_aGenerators.Insert(generator);
						break;
					}
					case GENERATE_SEND_ORDER:
					{
						SCR_AiScriptGenerator_SendOrderGenerator generator = new SCR_AiScriptGenerator_SendOrderGenerator(m_CurrentClass);
						generator.m_sGeneratedClassName = macroArgs[1];
						m_CurrentClass.m_aGenerators.Insert(generator);
						break;
					}
				}
			}
		}
		else if (ExtractMacro(comment, MACRO_VARIABLE, macroArgs))
		{
			// Only makes sense within a class
			if (!m_CurrentClass)
				return;

			if (!VerifyMacro_Variable(macroArgs))
				return;

			string varType, varName;
			if (!ExtractVariable(line, varType, varName))
				return;

			SCR_AiScriptGenerator_Variable v = new SCR_AiScriptGenerator_Variable();
			v.m_sLine = m_sCurrentLine;
			v.m_sName = varName;
			v.m_sType = varType;
			array<int> _a = { 0, 2 };
			foreach (int i : _a)
			{
				if (!macroArgs.IsIndexValid(i+ 1))
					continue;

				string arg = macroArgs[i];

				if (arg == NODE_PORT)
					v.m_sBindPortName = macroArgs[i+ 1];
				else if (arg == NODE_PROPERTY)
					v.m_sBindPropertyName = macroArgs[i+ 1];
				else if (arg == NODE_PROPERTY_ENUM)
				{
					v.m_sBindPropertyName = macroArgs[i+ 1];
					v.m_bBindPropertyIsEnum = true;
				}
			}
			m_CurrentClass.m_aVariables.Insert(v);
		}

	}

	//------------------------------------------------------------------------------------------------
	// Parsing text

	//------------------------------------------------------------------------------------------------
	// Returns content of a comment. Example:
	// "1234; //Something" -> "Something"
	protected string ExtractComment(string str)
	{
		int commentId = str.IndexOf(COMMENT);
		if (commentId == -1)
			return string.Empty;
		int strlen = str.Length();
		int substrStart = commentId + COMMENT.Length();
		int substrLen = strlen - substrStart;
		string commentContent = str.Substring(substrStart, substrLen);
		return commentContent;
	}

	//------------------------------------------------------------------------------------------------
	// Extracts arguments from a macro. Example:
	// "MACRO(1, 2, 3)" -> {"1", "2", "3"}
	protected bool ExtractMacro(string str, string macroName, array<string> outMacroArguments)
	{
		int macroStartId = str.IndexOf(macroName);
		if (macroStartId == -1)
			return false;

		int startBracketSearchId = macroStartId + macroName.Length();
		int openBracketId = str.IndexOfFrom(startBracketSearchId, "(");
		int closeBracketId = str.IndexOfFrom(startBracketSearchId, ")");

		if (openBracketId == -1 || closeBracketId == -1 || openBracketId > closeBracketId)
		{
			_print(string.Format("ExtractMacro: failed to parse macro %1", macroName));
			return false;
		}

		string strMacroArgs = str.Substring(openBracketId + 1, closeBracketId - openBracketId - 1);
		if (strMacroArgs.IsEmpty())
		{
			// No macro arguments
			outMacroArguments.Clear();
			return true;
		}
		if (!strMacroArgs.Contains(","))
		{
			// One macro argument
			strMacroArgs.TrimInPlace();
			outMacroArguments.Clear();
			outMacroArguments.Insert(strMacroArgs);
			return true;
		}
		else
		{
			outMacroArguments.Clear();
			strMacroArgs.Split(",", outMacroArguments, true);
			int nMacroArgs = outMacroArguments.Count();
			for (int i = 0; i < nMacroArgs; i++)
			{
				outMacroArguments[i] = outMacroArguments[i].Trim();
			}
			return true;
		}
	}

	//------------------------------------------------------------------------------------------------
	// Extracts variable type and name. Example:
	// "protected const int m_iSize = 3;" -> "int", "m_iSize"
	protected bool ExtractVariable(string str, out string varType, out string varName)
	{
		/*
		smth smth type VarName;
		smth smth type VarName = whatever();
		smth smth type VarName=whatever();
		*/

		// Remove ; and averything after it
		int idDotComma = str.IndexOf(";");
		if (idDotComma != -1)
			str = str.Substring(0, idDotComma);

		// Remove right side (after '=')
		int idEquals = str.IndexOf("=");
		if (idEquals != -1)
			str = str.Substring(0, idEquals);

		// Split by spaces
		array<string> tokens = {};
		str.Split(" ", tokens, true);

		int nTokens = tokens.Count();
		if (nTokens < 2)
		{
			_print("Failed to parse variable", LogLevel.ERROR);
			return false;
		}

		varType = tokens[nTokens - 2].Trim();
		varName = tokens[nTokens - 1].Trim();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Extracts class name and parent class name. Example:
	// class MyClass : MyParentClass -> "MyClass", "MyParentClass"
	protected bool ExtractClass(string str, out string className, out string parentClassName)
	{
		className = string.Empty;
		parentClassName = string.Empty;

		// Split string by ' '
		array<string> tokens = {};
		str.Split(" ", tokens, true);

		// Find where "class" is
		int idClassToken = tokens.Find(CLASS);
		if (idClassToken == -1)
			return false;

		// Extract class name and parent class name
		for (int i = idClassToken + 1; i < tokens.Count(); i++)
		{
			string s = tokens[i];
			if (s == ":" || s == "extends")			// Ignore ":" or "extends"
				continue;

			if (s.StartsWith("/"))					// Stop if it's start of comment
				break;
			else if (className.IsEmpty())			// Class name
				className = s;
			else if (parentClassName.IsEmpty())
			{
				parentClassName = s;
				break;
			}
		}

		// Success if we at least found class name
		bool success = !className.IsEmpty();

		if (!success)
			_print("Failed to parse class", LogLevel.ERROR);

		return success;
	}

	//------------------------------------------------------------------------------------------------
	// Removes quotes from string. Example:
	// ""123"" -> "123"
	protected string Unquote(string str)
	{
		return str.Substring(1, str.Length() - 2);
	}

	//------------------------------------------------------------------------------------------------
	// Prints message with current parsing context
	protected void _print(string str, LogLevel logLevel = LogLevel.NORMAL, bool printContext = true)
	{
		// Print message
		SCR_AiScriptGeneratorPlugin._print(str, logLevel);

		// Print context
		if (printContext)
		{
			SCR_AiScriptGeneratorPlugin._print(string.Format("  File: %1", m_sCurrentFile), logLevel);
			SCR_AiScriptGeneratorPlugin._print(string.Format("  Line %1: %2", m_iCurrentLineId + 1, m_sCurrentLine), logLevel);
		}
	}

	//------------------------------------------------------------------------------------------------
	// Verification of macros

	//------------------------------------------------------------------------------------------------
	protected bool VerifyMacro_MessageClass(array<string> args)
	{
		if (args.IsEmpty())
			return true;

		if (args.Count() != 2)
		{
			_print(string.Format("Wrong argument count for macro %1", MACRO_MESSAGE_CLASS), logLevel: LogLevel.ERROR);
			return false;
		}

		switch (args[0])
		{
			case GENERATE_SEND_GOAL_MESSAGE:
			case GENERATE_SEND_INFO_MESSAGE:
			case GENERATE_SEND_ORDER:
				return true;
				break;

			default:
				_print(string.Format("Unknown macro argument \"%1\"", args[0]), logLevel: LogLevel.ERROR);
				return false;
		}

		return VerifyMacroArgumentCount(MACRO_MESSAGE_CLASS, args, 1);
	}

	//------------------------------------------------------------------------------------------------
	protected bool VerifyMacro_Variable(array<string> args)
	{
		int nArgs = args.Count();
		if (nArgs != 0 && nArgs != 2 && nArgs != 4)
		{
			_print(string.Format("Wrong argument count for macro: %1", MACRO_VARIABLE), logLevel: LogLevel.ERROR);
			return false;
		}

		if (args.IsIndexValid(0) && args[0] != NODE_PORT && args[0] != NODE_PROPERTY && args[0] != NODE_PROPERTY_ENUM)
		{
			_print(string.Format("Unknown macro argument \"%1\"", args[0]), logLevel: LogLevel.ERROR);
			return false;
		}

		if (args.IsIndexValid(2) && args[2] != NODE_PORT && args[2] != NODE_PROPERTY && args[2] != NODE_PROPERTY_ENUM)
		{
			_print(string.Format("Unknown macro argument \"%1\"", args[0]), logLevel: LogLevel.ERROR);
			return false;
		}

		return VerifyMacroArgumentCount(MACRO_VARIABLE, args, 1);
	}

	//------------------------------------------------------------------------------------------------
	protected bool VerifyMacroArgumentCount(string macroName, array<string> args, int n)
	{
		if (args.Count() < n)
		{
			_print(string.Format("Expected at least %1 arguments for macro: %2", n, macroName), logLevel: LogLevel.ERROR);
			return false;
		}

		return true;
	}
}

//------------------------------------------------------------------------------------------------
// Buffer which handles tabs in generated text
class SCR_AiScriptGenerator_OutputFormatBuffer
{
	protected int m_iIndentCount = 0;
	protected ref array<string> m_aGeneratedLines = {};

	//------------------------------------------------------------------------------------------------
	//! \return
	array<string> GetLines() { return m_aGeneratedLines; }

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] line
	void AddLine(string line)
	{
		if (line.StartsWith("}"))
			m_iIndentCount--;

		string strIndent;
		for (int i = 0; i < m_iIndentCount; i++)
		{
			strIndent = strIndent + "\t";
		}
		m_aGeneratedLines.Insert(strIndent + line);

		// Auto increase/decrease indent count
		if (line.StartsWith("{"))
			m_iIndentCount++;
	}
}

// Base class which defines what to do with parsed data
class SCR_AiScriptGenerator_ClassGeneratorBase
{
	SCR_AiScriptGenerator_Class m_Class; // Class this generator is attached to
	string m_sGeneratedClassName;

	//------------------------------------------------------------------------------------------------
	//! Called to generate text into a context
	//! \param[in] ctx
	void Generate(SCR_AiScriptGenerator_OutputFormatBuffer ctx);

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] ctx
	//! \param[in] extraPortNames
	//! \param[in] variables
	void GenerateGetVariablesIn(SCR_AiScriptGenerator_OutputFormatBuffer ctx, array<string> extraPortNames, array<SCR_AiScriptGenerator_Variable> variables)
	{
		// Port names
		array<string> aPortNames = {};
		aPortNames.Copy(extraPortNames);
		foreach (SCR_AiScriptGenerator_Variable v : variables)
		{
			if (!v.m_sBindPortName.IsEmpty())
				aPortNames.Insert(string.Format("\"%1\"", v.m_sBindPortName));
		}

		ctx.AddLine("protected static ref TStringArray _s_aVarsIn =");
		ctx.AddLine("{");
			int nPorts = aPortNames.Count();
			foreach (int varId, string portName : aPortNames)
			{
				string strPortName = portName;
				if (varId != nPorts-1)
					strPortName = strPortName + ",";
				ctx.AddLine(strPortName);
			}
		ctx.AddLine("};");
		ctx.AddLine("override TStringArray GetVariablesIn() { return _s_aVarsIn; }");
		ctx.AddLine("");
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] ctx
	//! \param[in] variables
	void GenerateVariablesWithAttributes(SCR_AiScriptGenerator_OutputFormatBuffer ctx, array<SCR_AiScriptGenerator_Variable> variables)
	{
		foreach (SCR_AiScriptGenerator_Variable v : variables)
		{
			if (v.m_sBindPropertyName.IsEmpty())
				continue;

			if (!v.m_bBindPropertyIsEnum)
				ctx.AddLine("[Attribute(\"" + "\")]");
			else
				ctx.AddLine(string.Format("[Attribute(\"" + "\", UIWidgets.ComboBox, enumType: %1)]", v.m_sType));

			ctx.AddLine(string.Format("%1 %2;", v.m_sType, v.m_sBindPropertyName));
			ctx.AddLine("");
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] ctx
	//! \param[in] variables
	void GenerateSetMessageVariables(SCR_AiScriptGenerator_OutputFormatBuffer ctx, array<SCR_AiScriptGenerator_Variable> variables)
	{
		ctx.AddLine(string.Format("msg.SetText(m_sText);"));
		ctx.AddLine("");

		foreach (SCR_AiScriptGenerator_Variable v : variables)
		{
			if (!v.m_sBindPortName.IsEmpty() && !v.m_sBindPropertyName.IsEmpty())
			{
				// Set from property or port
				ctx.AddLine(string.Format("if(!GetVariableIn(\"%1\", msg.%2))", v.m_sBindPortName, v.m_sName));
				ctx.AddLine(string.Format("\tmsg.%1 = %2;", v.m_sName, v.m_sBindPropertyName));
				ctx.AddLine("");
			}
			else if (!v.m_sBindPortName.IsEmpty())
			{
				// Set from port
				ctx.AddLine(string.Format("GetVariableIn(\"%1\", msg.%2);", v.m_sBindPortName, v.m_sName));
				ctx.AddLine("");
			}
			else if (!v.m_sBindPropertyName.IsEmpty())
			{
				// Set from property
				ctx.AddLine(string.Format("msg.%1 = %2;", v.m_sName, v.m_sBindPropertyName));
				ctx.AddLine("");
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] ctx
	//! \param[in] variables
	void GenerateGetNodeMiddleText(SCR_AiScriptGenerator_OutputFormatBuffer ctx, array<SCR_AiScriptGenerator_Variable> variables)
	{
		array<SCR_AiScriptGenerator_Variable> properties = {};
		foreach (SCR_AiScriptGenerator_Variable v : variables)
		{
			if (v.m_sBindPropertyName)
				properties.Insert(v);
		}

		if (properties.IsEmpty())
			return;

		ctx.AddLine("override string GetNodeMiddleText()");
		ctx.AddLine("{");
			ctx.AddLine("string s;");
			foreach (SCR_AiScriptGenerator_Variable v : properties)
			{
				if (v.m_bBindPropertyIsEnum)
					ctx.AddLine(string.Format("s = s + string.Format(\"%1: %2\\n\", typename.EnumToString(%3, %4));", v.m_sBindPropertyName, "%1", v.m_sType, v.m_sBindPropertyName));
				else
					ctx.AddLine(string.Format("s = s + string.Format(\"%1: %2\\n\", %3);", v.m_sBindPropertyName, "%1", v.m_sBindPropertyName));
			}
			ctx.AddLine("return s;");
		ctx.AddLine("}");
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] ctx
	void GenerateCommentSeparator(SCR_AiScriptGenerator_OutputFormatBuffer ctx)
	{
		ctx.AddLine("//---------------------------------------------------------------------------------------");
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] _class
	//! \return
	// Returns all variables of a class, including variables of parent classes
	array<SCR_AiScriptGenerator_Variable> GetClassVariables(SCR_AiScriptGenerator_Class _class)
	{
		// Find all parent classes
		array<SCR_AiScriptGenerator_Class> parentClasses = {};
		SCR_AiScriptGenerator_Class parentClass = _class;
		while (parentClass)
		{
			parentClasses.Insert(parentClass);
			parentClass = parentClass.m_ParentClass;
		}

		// Create array of variables, first declared variables first, parent classes first
		array<SCR_AiScriptGenerator_Variable> variables = {};
		for (int classId = parentClasses.Count() - 1; classId >= 0; classId--)
		{
			SCR_AiScriptGenerator_Class c = parentClasses[classId];
			foreach (SCR_AiScriptGenerator_Variable v : c.m_aVariables)
			{
				variables.Insert(v);
			}
		}

		return variables;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_AiScriptGenerator_ClassGeneratorBase(SCR_AiScriptGenerator_Class attachedToClass)
	{
		m_Class = attachedToClass;
	}
}

class SCR_AiScriptGenerator_SendInfoMessageGenerator : SCR_AiScriptGenerator_ClassGeneratorBase
{
	//------------------------------------------------------------------------------------------------
	override void Generate(SCR_AiScriptGenerator_OutputFormatBuffer ctx)
	{
		// Create array of variables of this class and its parent classes
		array<SCR_AiScriptGenerator_Variable> variables = GetClassVariables(m_Class);

		GenerateCommentSeparator(ctx);
		ctx.AddLine(string.Format("// Generated from class: %1", m_Class.m_sName));
		ctx.AddLine(string.Format("class %1 : SCR_AISendMessageGenerated", m_sGeneratedClassName));
		ctx.AddLine("{");

			// Variables with attributes
			GenerateVariablesWithAttributes(ctx, variables);

			// GerVariablesIn
			GenerateGetVariablesIn(ctx, { "SCR_AISendMessageGenerated.PORT_RECEIVER" }, variables);

			// EOnTaskSimulate
			ctx.AddLine("override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)");
			ctx.AddLine("{");
				ctx.AddLine("AIAgent receiver = GetReceiverAgent(owner);");
				ctx.AddLine(string.Format("%1 msg = new %1();", m_Class.m_sName));
				ctx.AddLine("");

				// Set message variables
				GenerateSetMessageVariables(ctx, variables);

				ctx.AddLine(string.Format("if (SendMessage(owner, receiver, msg))"));
				ctx.AddLine("\treturn ENodeResult.SUCCESS;");
				ctx.AddLine("else");
				ctx.AddLine("\treturn ENodeResult.FAIL;");
			ctx.AddLine("}");
			ctx.AddLine("");

			GenerateGetNodeMiddleText(ctx, variables);

			ctx.AddLine("static override bool VisibleInPalette() { return true; }");

		ctx.AddLine("}");
		ctx.AddLine("");
	}
}

class SCR_AiScriptGenerator_SendGoalMessageGenerator : SCR_AiScriptGenerator_ClassGeneratorBase
{
	//------------------------------------------------------------------------------------------------
	override void Generate(SCR_AiScriptGenerator_OutputFormatBuffer ctx)
	{
		// Create array of variables of this class and its parent classes
		array<SCR_AiScriptGenerator_Variable> variables = GetClassVariables(m_Class);

		GenerateCommentSeparator(ctx);
		ctx.AddLine(string.Format("// Generated from class: %1", m_Class.m_sName));
		ctx.AddLine(string.Format("class %1 : SCR_AISendMessageGenerated", m_sGeneratedClassName));
		ctx.AddLine("{");

			// Variables with attributes
			GenerateVariablesWithAttributes(ctx, variables);

			// GetVariablesIn
			GenerateGetVariablesIn(ctx, { "SCR_AISendMessageGenerated.PORT_RECEIVER" }, variables);

			// EOnTaskSimulate
			ctx.AddLine("override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)");
			ctx.AddLine("{");
				ctx.AddLine("AIAgent receiver = GetReceiverAgent(owner);");
				ctx.AddLine(string.Format("%1 msg = new %1();", m_Class.m_sName));
				ctx.AddLine("");

				// Set related activity
				ctx.AddLine(string.Format("msg.m_RelatedGroupActivity = GetRelatedActivity(owner);"));
				ctx.AddLine("");

				// Set message variables
				GenerateSetMessageVariables(ctx, variables);

				// Set related waypoint
				ctx.AddLine("if (msg.m_bIsWaypointRelated)");
				ctx.AddLine("\tmsg.m_RelatedWaypoint = GetRelatedWaypoint(owner);");
				ctx.AddLine("");

				ctx.AddLine(string.Format("if (SendMessage(owner, receiver, msg))"));
				ctx.AddLine("\treturn ENodeResult.SUCCESS;");
				ctx.AddLine("else");
				ctx.AddLine("\treturn ENodeResult.FAIL;");
			ctx.AddLine("}");
			ctx.AddLine("");

			GenerateGetNodeMiddleText(ctx, variables);

			ctx.AddLine("static override bool VisibleInPalette() { return true; }");

		ctx.AddLine("}");
		ctx.AddLine("");
	}
}

class SCR_AiScriptGenerator_SendOrderGenerator : SCR_AiScriptGenerator_ClassGeneratorBase
{
	//------------------------------------------------------------------------------------------------
	override void Generate(SCR_AiScriptGenerator_OutputFormatBuffer ctx)
	{
		// Create array of variables of this class and its parent classes
		array<SCR_AiScriptGenerator_Variable> variables = GetClassVariables(m_Class);

		GenerateCommentSeparator(ctx);
		ctx.AddLine(string.Format("// Generated from class: %1", m_Class.m_sName));
		ctx.AddLine(string.Format("class %1 : SCR_AISendMessageGenerated", m_sGeneratedClassName));
		ctx.AddLine("{");

		// Variables with attributes
		GenerateVariablesWithAttributes(ctx, variables);

		// GetVariablesIn
		GenerateGetVariablesIn(ctx, { "SCR_AISendOrderGenerated.PORT_RECEIVER" }, variables);

		// EOnTaskSimulate
		ctx.AddLine("override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)");
		ctx.AddLine("{");
			ctx.AddLine("AIAgent receiver = GetReceiverAgent(owner);");
			ctx.AddLine(string.Format("%1 msg = new %1();", m_Class.m_sName));
			ctx.AddLine("");

			// Set message variables
			GenerateSetMessageVariables(ctx, variables);

			ctx.AddLine(string.Format("if (SendMessage(owner, receiver, msg))"));
			ctx.AddLine("\treturn ENodeResult.SUCCESS;");
			ctx.AddLine("else");
			ctx.AddLine("\treturn ENodeResult.FAIL;");
		ctx.AddLine("}");
		ctx.AddLine("");

		GenerateGetNodeMiddleText(ctx, variables);

		ctx.AddLine("static override bool VisibleInPalette() { return true; }");

		ctx.AddLine("}");
		ctx.AddLine("");
	}
}

class SCR_AiScriptGenerator_Class
{
	string m_sLine;
	string m_sName;
	string m_sParentClassName;
	SCR_AiScriptGenerator_Class m_ParentClass;
	ref array<ref SCR_AiScriptGenerator_Variable> m_aVariables = {};

	// Defines what to do with this
	ref array<ref SCR_AiScriptGenerator_ClassGeneratorBase> m_aGenerators = {};

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetListing()
	{
		string strParentClassWarning;
		if (!m_ParentClass)
			strParentClassWarning = "(Not found!)";

		return string.Format("Class: %1 : %2%3", m_sName, m_sParentClassName, strParentClassWarning);
	}
}

class SCR_AiScriptGenerator_Variable
{
	string m_sLine;
	string m_sName;
	string m_sType;

	string m_sBindPortName; // Which node port initializes this variable. Can be empty.

	string m_sBindPropertyName; // Which node property initializes this variable. Can be empty.
	bool m_bBindPropertyIsEnum; // True when the property must be an enum

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetListing()
	{
		return string.Format("Variable: %1 %2 -> %3", m_sType, m_sName, m_sBindPortName);
	}
}
#endif // WORKBENCH
