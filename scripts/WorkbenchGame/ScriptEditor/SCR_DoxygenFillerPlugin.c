#ifdef WORKBENCH
[WorkbenchPluginAttribute(
	name: "Doxygen Filler",
	description: "Create/Format Doxygen documentation skeleton for configured methods (default public methods only)",
	shortcut: "Ctrl+Alt+Shift+D",
	wbModules: { "ScriptEditor" },
	awesomeFontCode: 0xF518)]
// TODO: multiline method declarations
// TODO: addon-wide file process
class SCR_DoxygenFillerPlugin : WorkbenchPlugin
{
	/*
		Doxygen
	*/

	[Attribute("1", category: "Doxygen", desc: "Add Doxygen skeleton to public methods (neither protected nor private)")]
	protected bool m_bDoxygenPublicMethods;

	[Attribute("0", category: "Doxygen", desc: "Add Doxygen skeleton to protected methods")]
	protected bool m_bDoxygenProtectedMethods;

	[Attribute("0", category: "Doxygen", desc: "Add Doxygen skeleton to private methods")]
	protected bool m_bDoxygenPrivateMethods;

	[Attribute("0", category: "Doxygen", desc: "Add Doxygen skeleton to overridden methods")]
	protected bool m_bDoxygenOverriddenMethods;

	[Attribute("1", category: "Doxygen", desc: "Add Doxygen skeleton to static methods")]
	protected bool m_bDoxygenStaticMethods;

	[Attribute("0", category: "Doxygen", desc: "Add Doxygen skeleton to Obsolete methods")]
	protected bool m_bDoxygenObsoleteMethods;

	[Attribute("MethodPrefix", category: "Doxygen", desc: "Only document params and return value (without description) for methods starting with these prefixes (e.g Set*, Get*, On*, Is* etc, case-sensitive)")]
	protected ref array<string> m_aPartialDoxygenPrefixes;

	/*
		Other
	*/

	[Attribute("1", category: "Other", desc: "Replace Doxygen blocks with multiple one line comments (" + DOXYGEN_LINE_START + ")")]
	protected bool m_bConvertDoxygenFormatting;

	[Attribute("1", category: "Other", desc: "Add missing separators to undocumented methods")]
	protected bool m_bAddMissingSeparators;

	[Attribute("1", category: "Other", desc: "Add \"" + CONSTRUCTOR_COMMENT + "\" comment to constructor")]
	protected bool m_bAddConstructorNormalComment;

	[Attribute("1", category: "Other", desc: "Add \"" + DESTRUCTOR_COMMENT + "\" comment to destructor")]
	protected bool m_bAddDestructorNormalComment;

	/*
		Debug
	*/

	[Attribute(defvalue: "0", category: "Debug", desc: "Output method parsing errors in the log console")]
	protected bool m_bOutputMethodParsingErrors;

	protected static const string METHOD_SEPARATOR = SCR_StringHelper.DOUBLE_SLASH + "------------------------------------------------------------------------------------------------";
	protected static const string DOXYGEN_LINE_START = SCR_StringHelper.DOUBLE_SLASH + "!"; // the space is optional... unfortunately
	protected static const string GENERATED_SCRIPT_WARNING = "Do not modify, this script is generated"; // must be the exact line, tabs included if any
	protected static const string CONSTRUCTOR_COMMENT = SCR_StringHelper.DOUBLE_SLASH + " constructor";
	protected static const string DESTRUCTOR_COMMENT = SCR_StringHelper.DOUBLE_SLASH + " destructor";
	protected static const string COMMENT_BLOCK_START = "/" + "*";
	protected static const string COMMENT_BLOCK_END = "*" + "/";
	protected static const ref array<string> DOXYGEN_BLOCK_STARTS = { COMMENT_BLOCK_START + "*", COMMENT_BLOCK_START + "!" };
	protected static const string BASE_PARAM = "\\param";
	protected static const string FAULTY_PARAM = "\\param ";

	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		ProcessCurrentFile();
	}

	//------------------------------------------------------------------------------------------------
	protected bool ProcessAddon()
	{
		// TODO
	}

	//------------------------------------------------------------------------------------------------
	protected void Init()
	{
		if (m_aPartialDoxygenPrefixes.IsEmpty())
			m_aPartialDoxygenPrefixes = { "Get", "Set", "On", "Is" };

		SCR_ArrayHelperT<string>.RemoveDuplicates(m_aPartialDoxygenPrefixes);
	}

	//------------------------------------------------------------------------------------------------
	//! Process the file currently opened in the Script Editor (focused tab)
	//! \return true in case of success, false otherwise
	protected bool ProcessCurrentFile()
	{
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		if (!scriptEditor)
			return false;

		string filePath;
		if (!scriptEditor.GetCurrentFile(filePath))
		{
			Print("Could not get current file", LogLevel.ERROR);
			return false;
		}

		if (filePath == __FILE__)
		{
			Print("Cannot add Doxygen documentation to the Doxygen Filler plugin itself - " + filePath, LogLevel.NORMAL);
			return false;
		}

		string absoluteFilePath;
		if (!Workbench.GetAbsolutePath(filePath, absoluteFilePath, true))
		{
			Print("Could not get absolute file path - " + filePath, LogLevel.WARNING);
			return false;
		}

		Print("Processing " + absoluteFilePath, LogLevel.NORMAL);

		bool result = ProcessFile(absoluteFilePath);
		if (result)
			Print("Successfully processed " + absoluteFilePath, LogLevel.NORMAL);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Process the file and add Doxygen-formatted method documentation
	//! \param[in] filePath ideally an absolute file path
	//! \return true on success, false otherwise
	protected bool ProcessFile(string filePath)
	{
		array<string> lines = SCR_FileIOHelper.ReadFileContent(filePath);
		if (lines.Contains(GENERATED_SCRIPT_WARNING))
		{
			Print("This file is generated - skipping - " + filePath, LogLevel.WARNING);
			return false;
		}

		array<ref Tuple2<int, string>> commentsToAdd = {};
		array<ref Tuple2<int, string>> commentsToReplace = {}; // Doxygen blocks to Doxygen lines

		int resetValues = 0; // 0 = don't reset, 1 = don't reset line offset, 2 = don't reset separator, 3 = reset everything
		bool hasSeparator;
		string currentDoxygenComment;
		bool isInDoxygenCommentBlock;
		bool isInCommentBlock;
		string trimmedLine;
		int commentBlockIndex;
		string currentClassName;
		int lineOffset;
		bool isObsolete;
		foreach (int lineId, string line : lines)
		{
			if (resetValues > 0)
			{
				if (resetValues != 2)
					hasSeparator = false;

				currentDoxygenComment = string.Empty;
				if (resetValues != 1)
					lineOffset = 0;

				resetValues = 0;
			}

			commentBlockIndex = 0;
			while (true) // meh
			{
				if (!isInCommentBlock)
				{
					commentBlockIndex = line.IndexOfFrom(commentBlockIndex, COMMENT_BLOCK_START);
					if (commentBlockIndex < 0)
						break;

					isInCommentBlock = true;
					isInDoxygenCommentBlock = SCR_StringHelper.IndexOfFrom(line, commentBlockIndex, DOXYGEN_BLOCK_STARTS) == commentBlockIndex;
				}

				commentBlockIndex = line.IndexOfFrom(commentBlockIndex, COMMENT_BLOCK_END);
				if (commentBlockIndex < 0)
					break;

				isInCommentBlock = false;
				isInDoxygenCommentBlock = false;
			}

			trimmedLine = line.Trim(); // 8191 chars limit

			if (isInCommentBlock)
			{
				if (isInDoxygenCommentBlock)
				{
					if (currentDoxygenComment) // already has comment
						currentDoxygenComment += "\n" + trimmedLine;
					else
						currentDoxygenComment += trimmedLine;
				}
				else
				{
					resetValues = 1;
				}

				continue;
			}
			else // related to comment blocks
			{
				if (currentDoxygenComment && trimmedLine.Contains(COMMENT_BLOCK_END))
				{
					if (trimmedLine == COMMENT_BLOCK_END)
						currentDoxygenComment += "\n" + trimmedLine;
					else
					if (trimmedLine.StartsWith(COMMENT_BLOCK_END))
						currentDoxygenComment += "\n" + COMMENT_BLOCK_END;
					else
						currentDoxygenComment += "\n" + trimmedLine.Substring(0, trimmedLine.IndexOfFrom(1, COMMENT_BLOCK_END) - 1);

					continue;
				}
			}

			// not in comment block? go ahead

			if (!trimmedLine)
			{
				resetValues = 3;
				continue;
			}

			if (trimmedLine.StartsWith(SCR_StringHelper.DOUBLE_SLASH))
			{
				if (trimmedLine == METHOD_SEPARATOR)
				{
					hasSeparator = true;
					resetValues = 2;
					continue;
				}
				else if (trimmedLine == CONSTRUCTOR_COMMENT || trimmedLine == DESTRUCTOR_COMMENT || trimmedLine.StartsWith(DOXYGEN_LINE_START))
				{
					if (currentDoxygenComment)
						currentDoxygenComment += "\n" + trimmedLine;
					else
						currentDoxygenComment = trimmedLine;

					continue;
				}
				else
				{
					// allow normal comments between Doxygen doc and method line
					lineOffset--;
					continue;
				}
			}

			if (!m_bDoxygenObsoleteMethods && trimmedLine.StartsWith("[Obsolete(")) // )
			{
				isObsolete = true;
				continue;
			}

			if (trimmedLine.StartsWith("["))	// [Attribute()]
			{
				lineOffset--;
				continue;
			}

			if (trimmedLine.StartsWith("class "))
			{
				string classname = GetClassname(trimmedLine);
				if (classname)
				{
					currentClassName = classname;
					resetValues = 3;
					continue;
				}
			}
			else
			if (trimmedLine.StartsWith("enum "))
			{
				resetValues = 3;
				continue;
			}

			if (!currentClassName) // not in class? keep seeking
				continue;

			SCR_DoxygenFillerPlugin_Method method = GetMethodObject(line, lineId);
			if (!method)
			{
				resetValues = 3;
				continue;
			}

			if (isObsolete)
			{
				isObsolete = false;
				resetValues = 3;
				continue;
			}

			if (m_bConvertDoxygenFormatting && SCR_StringHelper.StartsWithAny(currentDoxygenComment, DOXYGEN_BLOCK_STARTS))
			{
				currentDoxygenComment = ConvertDoxygenBlockToSingleLines(currentDoxygenComment, method);
				if (currentDoxygenComment)
				{
					currentDoxygenComment = AddIndent(currentDoxygenComment);
					array<string> doxyBlockLines = {};
					currentDoxygenComment.Split("\n", doxyBlockLines, false);
					int doxyBlockLinesCount = doxyBlockLines.Count();
					if (doxyBlockLinesCount > 0)
					{
						if (!hasSeparator && doxyBlockLines[0].Trim() == DOXYGEN_LINE_START)
							currentDoxygenComment = SCR_StringHelper.ReplaceTimes(currentDoxygenComment, DOXYGEN_LINE_START + "\n", METHOD_SEPARATOR + "\n", 1);

						commentsToReplace.Insert(new Tuple2<int, string>(lineId - doxyBlockLines.Count(), currentDoxygenComment));
					}
				}
			}

			if (!currentDoxygenComment)
			{
				int addedLines;
				string doxygenDocumentation = GetDoxygenDocumentation(method, !hasSeparator && m_bAddMissingSeparators, currentClassName);
				if (doxygenDocumentation)
				{
					doxygenDocumentation = AddIndent(doxygenDocumentation);
					addedLines = SCR_StringHelper.CountOccurrences(doxygenDocumentation, "\n") + 1;

					Print("+Doxygen " + method.m_sName + "() method (previously on L" + (lineId + 1) + ")", LogLevel.NORMAL);
					commentsToAdd.Insert(new Tuple2<int, string>(lineId + lineOffset, doxygenDocumentation));
				}
			}

			resetValues = 3;
		}

		int addCount = commentsToAdd.Count();
		int replaceCount = commentsToReplace.Count();
		if (addCount < 1 && (!m_bConvertDoxygenFormatting || replaceCount < 1))
		{
			if (m_bConvertDoxygenFormatting)
				Print("No documentable methods nor convertible comments were found", LogLevel.NORMAL);
			else
				Print("No documentable methods were found", LogLevel.NORMAL);

			return false;
		}

		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		bool useScriptEditor = scriptEditor != null; // staying in script editor for now

		// replace first, -then- change lines
		// otherwise line number is skewed

		if (useScriptEditor)
		{
			if (!scriptEditor.SetOpenedResource(filePath))
			{
				Print("Cannot open resource in Script Editor - " + filePath, LogLevel.WARNING);
				return false;
			}

			if (replaceCount > 0)
			{
				Print("Converting " + replaceCount + " comments...", LogLevel.NORMAL);

				array<string> commentLines = {};
				foreach (Tuple2<int, string> commentToReplace : commentsToReplace)
				{
					commentToReplace.param2.Split("\n", commentLines, false);
					foreach (int i, string line : commentLines)
					{
						if (scriptEditor.SetOpenedResource(filePath))
							scriptEditor.SetLineText(commentLines[i], commentToReplace.param1 + i);
					}
				}
			}

			if (addCount > 0)
			{
				Print("Documenting " + addCount + " methods...", LogLevel.NORMAL);

				Tuple2<int, string> commentInfo;
				for (int i = commentsToAdd.Count() - 1; i > -1; --i)
				{
					commentInfo = commentsToAdd[i];
					if (commentInfo.param1 > 0)
					{
						string trimmedPreviousLine = lines[commentInfo.param1 - 1].Trim();
						if (trimmedPreviousLine && trimmedPreviousLine != "{" && !trimmedPreviousLine.StartsWith(SCR_StringHelper.DOUBLE_SLASH)) // }
							commentInfo.param2 = "\n" + commentInfo.param2;
					}

					if (scriptEditor.SetOpenedResource(filePath)) // forces the edited file to be focused
						scriptEditor.InsertLine(commentInfo.param2, commentInfo.param1);
				}
			}
		}
		else // using FileIO (SCR_FileIOHelper)
		{
			if (replaceCount > 0)
			{
				// TODO: replacement
			}

			if (addCount > 0)
			{
				Tuple2<int, string> commentInfo;
				for (int i = commentsToAdd.Count() - 1; i > -1; --i)
				{
					commentInfo = commentsToAdd[i];
					if (commentInfo.param1 > 0)
					{
						string previousLine = lines[commentInfo.param1 - 1].Trim();
						if (previousLine != METHOD_SEPARATOR && !SCR_StringHelper.IsEmptyOrWhiteSpace(previousLine))
							commentInfo.param2 = "\n" + commentInfo.param2;
					}

					lines.InsertAt(commentInfo.param2, commentInfo.param1);
				}
			}

			if (lines[lines.Count() - 1] != string.Empty)
				lines.Insert(string.Empty);

			SCR_FileIOHelper.WriteFileContent(filePath, lines);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the name of the class declared on this line
	//! Works with inherited classes, does not work with interfering comments e.g "class /* */ myClass"
	//! \param[in] classLine a line starting with "class " (trim it before providing if needed)
	//! \return found classname
	protected string GetClassname(string classLine)
	{
		if (!classLine)
			return string.Empty;

		classLine.Replace("\t", " ");
		if (!classLine.StartsWith("class "))
			return string.Empty;

		array<string> parts = {};
		classLine.Split(" ", parts, true);

		if (parts[0] != "class")
			return string.Empty;

		string classname = parts[1];
		if (classname.EndsWith(":"))
			classname = classname.Substring(0, classname.Length() - 1).Trim();

		return classname;
	}

	//------------------------------------------------------------------------------------------------
	//! Get a method object IF public/protected/private/static/override settings match
	//! \param[in] line method signature with all arguments - starting with ONE tab only
	//! \param[in] lineNumber 0-based line number , for debug print
	//! \return method object with details
	protected SCR_DoxygenFillerPlugin_Method GetMethodObject(string line, int lineNumber)
	{
		if (!line)
			return null;

		if (line.StartsWith("\treturn"))
			return null;

		if (line.Contains(" = new ")) // safety against ref ScriptInvoker m_Invoker = new ScriptInvoker();
			return null;

		line.Replace("\n", string.Empty); // in case multiple lines are provided - beware of comments for now

		int index = line.IndexOf(SCR_StringHelper.DOUBLE_SLASH);
		if (index == 0)
			return null;

		if (index > -1)
			line = line.Substring(0, index);

		bool isMethod = line.StartsWith("\t") &&	// by method indentation level
			!line.StartsWith("\t\t") &&				// meh, but still
			line.Contains("(") &&
			line.Contains(")");

		if (!isMethod)
			return null;

		string trimmedLine = line.Trim();
		if (trimmedLine.StartsWith(SCR_StringHelper.DOUBLE_SLASH))
			return null;

		trimmedLine.Replace(SCR_StringHelper.TAB, SCR_StringHelper.SPACE);
		trimmedLine.Replace(", ", ",");
		trimmedLine = SCR_StringHelper.ReplaceRecursive(trimmedLine, "  ", " ");

		index = trimmedLine.IndexOf("("); // ) // has to be > -1
		array<string> rawSignatureAndParameters = {
			trimmedLine.Substring(0, index),
			trimmedLine.Substring(index + 1, trimmedLine.Length() - index - 1),
		};

		array<string> protectionReturnValueAndName = {};
		rawSignatureAndParameters[0].Split(" ", protectionReturnValueAndName, true);

		string tempString;
		for (int i = protectionReturnValueAndName.Count() - 2; i >= 0; --i) // e.g array<ref SomeClass>
		{
			tempString = protectionReturnValueAndName[i];
			if (tempString.EndsWith("ref"))
			{
				protectionReturnValueAndName[i] = tempString + " " + protectionReturnValueAndName[i + 1];
				protectionReturnValueAndName.RemoveOrdered(i + 1);
			}
		}

		bool isProtected = protectionReturnValueAndName.RemoveItemOrdered("protected");
		if (!m_bDoxygenProtectedMethods && isProtected)
			return null;

		bool isPrivate = !isProtected && protectionReturnValueAndName.RemoveItemOrdered("private");
		if (!m_bDoxygenPrivateMethods && isPrivate)
			return null;

		bool isPublic = !isProtected && !isPrivate;
		if (!m_bDoxygenPublicMethods && isPublic)
			return null;

		bool isOverride = protectionReturnValueAndName.RemoveItemOrdered("override");
		if (!m_bDoxygenOverriddenMethods && isOverride)
			return null;

		bool isStatic = protectionReturnValueAndName.RemoveItemOrdered("static");
		if (!m_bDoxygenStaticMethods && isStatic)
			return null;

		bool isSealed = protectionReturnValueAndName.RemoveItemOrdered("sealed");
		bool isEvent = protectionReturnValueAndName.RemoveItemOrdered("event");

		if (protectionReturnValueAndName.Count() != 2)
		{
			if (m_bOutputMethodParsingErrors)
				Print("Too many remaining method keywords line " + (lineNumber + 1) + ": " + SCR_StringHelper.Join(" | ", protectionReturnValueAndName), LogLevel.WARNING);

			return null;
		}

		SCR_DoxygenFillerPlugin_Method method = new SCR_DoxygenFillerPlugin_Method();
		method.m_sReturnType = protectionReturnValueAndName[0];
		method.m_sName = protectionReturnValueAndName[1];
		method.m_bOverride = isOverride;
		method.m_bIsStatic = isStatic;
		method.m_bIsSealed = isSealed;
		method.m_bIsEvent = isEvent;

		if (isProtected)
			method.m_iProtection = 1;
		else
			method.m_iProtection = 2;

		// parameters

		tempString = rawSignatureAndParameters[1]; // variable reuse
		if (tempString.StartsWith(")")) // no parameters, return now
			return method;

		tempString = tempString.Substring(0, tempString.LastIndexOf(")"));

		array<ref array<string>> paramStrings = GetParamsModifierTypeAndNameFromSig(tempString);

		SCR_DoxygenFillerPlugin_MethodParameter parameter;
		foreach (array<string> modifiersTypeAndParamName : paramStrings)
		{
			parameter = new SCR_DoxygenFillerPlugin_MethodParameter();

			if (modifiersTypeAndParamName.RemoveItemOrdered("out"))
				parameter.m_iInOut = 1;
			else if (modifiersTypeAndParamName.RemoveItemOrdered("inout"))
				parameter.m_iInOut = 2;

			if (modifiersTypeAndParamName.RemoveItemOrdered("notnull"))
				parameter.m_bNotNull = true;

			int count = modifiersTypeAndParamName.Count();
			if (count != 2 && count != 4)
			{
				if (m_bOutputMethodParsingErrors)
					Print("Cannot process parameter: " + SCR_StringHelper.Join(" | ", modifiersTypeAndParamName), LogLevel.WARNING);

				continue;
			}

			parameter.m_sType = modifiersTypeAndParamName[0];
			parameter.m_sName = modifiersTypeAndParamName[1];

			if (count != 2)
			{
				if (modifiersTypeAndParamName[2] == "=") // "=" "defValue"
					parameter.m_sDefaultValue = modifiersTypeAndParamName[3];
				else
				if (m_bOutputMethodParsingErrors)
					Print("Cannot process parameter: " + SCR_StringHelper.Join(" | ", modifiersTypeAndParamName), LogLevel.WARNING);
			}

			method.m_aParameters.Insert(parameter);
		}

		return method;
	}

	//------------------------------------------------------------------------------------------------
	//! Get parameter bits for each parameters
	//! \param[in] paramsString format "type paramName" (out, inout, notnull etc supported)
	//! \return array of arrays format { "modifier", "type", "paramName" }
	protected array<ref array<string>> GetParamsModifierTypeAndNameFromSig(string paramsString)
	{
		// remove comma-space
		paramsString.Replace(", ", ",");

		bool inBrackets;
		int chevronLevel;
		string currC;
		string buffer;

		array<ref array<string>> result = {};
		array<string> subResult = {};
		for (int i, count = paramsString.Length(); i < count; i++)
		{
			currC = paramsString[i];

			if (inBrackets)
			{
				if (currC == "]")
					inBrackets = false;

				continue;
			}

			if (currC == "[")
			{
				inBrackets = true;
				continue;
			}

			if (chevronLevel == 0)
			{
				if (currC == ",")
				{
					if (buffer)
					{
						subResult.Insert(buffer); // dropping currC
						buffer = string.Empty;
					}

					result.Insert(subResult);
					subResult = {};
					continue;
				}

				if (currC == " ")
				{
					if (buffer)
					{
						subResult.Insert(buffer); // dropping currC
						buffer = string.Empty;
					}

					continue;
				}
			}

			if (currC == "<")
				chevronLevel++;
			else if (currC == ">")
				chevronLevel--;

			buffer += currC;
		}

		if (buffer)
			subResult.Insert(buffer);

		if (!subResult.IsEmpty())
			result.Insert(subResult);

		// re-add comma-space
		foreach (array<string> subArray : result)
		{
			foreach (int i, string value : subArray)
			{
				value.Replace(",", ", ");
				subArray[i] = value;
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected string GetDoxygenDocumentation(notnull SCR_DoxygenFillerPlugin_Method method, bool addSeparator = false, string classname = string.Empty)
	{
		string generatedDoxygenDocumentation;

		bool isConstrOrDestr;
		if (classname && method.m_sReturnType == "void")
		{
			if (m_bAddDestructorNormalComment && method.m_sName == "~" + classname)
			{
				generatedDoxygenDocumentation = DESTRUCTOR_COMMENT;
				isConstrOrDestr = true;
			}
			else
			if (m_bAddConstructorNormalComment && method.m_sName == classname)
			{
				generatedDoxygenDocumentation = CONSTRUCTOR_COMMENT;
				isConstrOrDestr = true;
			}
		}

		if (!isConstrOrDestr)
		{
			if (!SCR_StringHelper.StartsWithAny(method.m_sName, m_aPartialDoxygenPrefixes))
				generatedDoxygenDocumentation += DOXYGEN_LINE_START;
		}

		foreach (SCR_DoxygenFillerPlugin_MethodParameter parameter : method.m_aParameters)
		{
			if (generatedDoxygenDocumentation)
				generatedDoxygenDocumentation += "\n";

			generatedDoxygenDocumentation += DOXYGEN_LINE_START + " \\param";
			switch (parameter.m_iInOut)
			{
				case 0: generatedDoxygenDocumentation += "[in]"; break; // meh - but \param alone is for old/unclear params
				case 1: generatedDoxygenDocumentation += "[out]"; break;
				case 2: generatedDoxygenDocumentation += "[in,out]"; break;
			}

			generatedDoxygenDocumentation += " " + parameter.m_sName;
		}

		if (method.m_sReturnType != "void")
		{
			if (generatedDoxygenDocumentation)
				generatedDoxygenDocumentation += "\n";

			generatedDoxygenDocumentation += DOXYGEN_LINE_START + " \\return";
		}

		if (addSeparator)
		{
			if (generatedDoxygenDocumentation)
				generatedDoxygenDocumentation = METHOD_SEPARATOR + "\n" + generatedDoxygenDocumentation;
			else
				generatedDoxygenDocumentation = METHOD_SEPARATOR;
		}

		return generatedDoxygenDocumentation;
	}

	//------------------------------------------------------------------------------------------------
	//! Convert a Doxygen comment block (indented or not) into multiple single lines (without indent)
	//! The result has the exact same number of lines as the input
	//! \param[in] doxygenBlock Doxygen comment block (between /** */ or /*! */)
	//! \param[in,out] method the method object, used to set proper [in], [out], [in,out] in case of existing params
	//! \return Doxygen comment as multiple single //! lines or empty string if nothing is provided or the block is empty
	protected string ConvertDoxygenBlockToSingleLines(string doxygenBlock, inout SCR_DoxygenFillerPlugin_Method method)
	{
		doxygenBlock.TrimInPlace();

		if (!doxygenBlock)
			return string.Empty;

//		doxygenBlock.Replace("\\param ", "\\param[in] ");

		int length = doxygenBlock.Length();
		if (length < 5)
			return string.Empty;
		else if (length == 5) // / * * * /
			return DOXYGEN_LINE_START;

		if (!SCR_StringHelper.StartsWithAny(doxygenBlock, DOXYGEN_BLOCK_STARTS))
			return string.Empty;

		array<string> lines = SCR_StringHelper.GetLines(doxygenBlock, false, true);

		int count = lines.Count();
		if (count == 0)
			return string.Empty; // huh?
		else
		if (count == 1)
			return lines[0].Substring(3, lines[0].Length() - 5);

		string line = lines[0];
		length = line.Length();
		if (length == 3)
			lines[0] = string.Empty;			// keep the line count
		else
			lines[0] = line.Substring(3, length - 3);

		line = lines[count - 1]; // last line
		length = line.Length();
		if (length == 2)
			lines[count - 1] = string.Empty;	// keep the line count
		else
			lines[count - 1] = line.Substring(0, length - 2).Trim();

		foreach (int i, string line2 : lines)
		{
			if (line2)
			{
				if (method)
				{
					SCR_DoxygenFillerPlugin_MethodParameter param;
					for (int j = method.m_aParameters.Count() - 1; j >= 0; --j)
					{
						param = method.m_aParameters[j];
						if (line2.StartsWith(FAULTY_PARAM + param.m_sName))
						{
							string suffix;
							switch (param.m_iInOut)
							{
								case 0: suffix = "[in]"; break;
								case 1: suffix = "[out]"; break;
								case 2: suffix = "[in,out]"; break;
							}

							line2.Replace(FAULTY_PARAM + param.m_sName, BASE_PARAM + suffix + " " + param.m_sName);
							break;
						}
					}
				}

				lines[i] = DOXYGEN_LINE_START + " " + line2;
			}
			else
			{
				lines[i] = DOXYGEN_LINE_START;
			}
		}

		return SCR_StringHelper.Join("\n", lines, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Indent text with the provided indent character(s)
	//! \param[in] input
	//! \param[in] indentLevel min 1, max int.MAX
	//! \param[in] indent character(s) to be used (for -one- indent level)
	//! \return indented text
	protected static string AddIndent(string input, int indentLevel = 1, string indent = "\t")
	{
		if (!input)
			return string.Empty;

		if (indentLevel < 1)
			indentLevel = 1;

		indent = SCR_StringHelper.PadLeft("", indentLevel, indent);
		input.Replace("\n", "\n" + indent);
		return indent + input;
	}

	//------------------------------------------------------------------------------------------------
	protected override void Configure()
	{
		Init();
		Workbench.ScriptDialog("Configure Doxygen plugin", "Add Doxygen skeleton where wanted and needed.", this);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK", true)]
	protected bool ButtonOK()
	{
		return true;
	}
}

//class SCR_DoxygenFillerPlugin_Class
//{
//	string m_sName;
//	string m_sParent;
//
//	ref array<ref SCR_DoxygenFillerPlugin_Variable> m_aVariables = {};
//	ref array<ref SCR_DoxygenFillerPlugin_Method> m_aMethods = {};
//}
//
//class SCR_DoxygenFillerPlugin_Variable
//{
//	bool m_bIsStatic;
//	bool m_bIsConstant;
//	int m_iProtection; //!< private = 0, protected = 1, public = 2
//	string m_sRef;
//	string m_sType;
//	string m_sName;
//	string m_sStartValue;
//}

class SCR_DoxygenFillerPlugin_Method
{
	int m_iProtection; //!< private = 0, protected = 1, public = 2
	bool m_bIsStatic;
	bool m_bIsSealed;		// unused atm
	bool m_bIsEvent;		// unused atm
	bool m_bOverride;
	string m_sReturnType;
	string m_sName;
	ref array<ref SCR_DoxygenFillerPlugin_MethodParameter> m_aParameters = {};
}

class SCR_DoxygenFillerPlugin_MethodParameter
{
	int m_iInOut; //!< 0 = in, 1 = out, 2 = inout
	bool m_bNotNull;		// unused atm
	string m_sType;			// unused atm
	string m_sName;
	string m_sDefaultValue;	// unused atm
}
#endif
