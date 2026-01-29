class ActionsManagerComponentClass: BaseActionsManagerComponentClass
{
};

class ActionsManagerComponent : BaseActionsManagerComponent
{
	#ifdef WORKBENCH
	//! List of collision prefixes that will be stripped
	const string IGNORED_PREFIXES[] = 
	{ 
		"v_",
		"w_",
	};
	
	//! Can be used in hitzone generation from bones
	//! to modify ("nicify") the name of a context
	//! \param name Original name that is used for the context point info (bone source)
	//! \param return Modified (nicified) version of string
	override protected string NicifyGeneratedContextName(string name)
	{
		
		// Check collider prefixes and cut them
		foreach (auto prefix : IGNORED_PREFIXES)
		{
			if (name.StartsWith(prefix))
			{
				name.Replace(prefix, "");
				break;
			}
		}	
		
		/*
		
		If this block is uncommented, names will be nicified even more, eg.:
		
		Inputs: w_bone_name
				v_AnOtHerFaNcy_nAmINg
		
		Outputs:
				Bone Name
				Anotherfancy Naming
		
		// Split name into idividual words
		array<string> words = new array<string>();
		
		if (name.Contains("_"))
			name.Split("_", words, true);
		else
			words.Insert(name);
		
		// Go through each word in the list
		// Cut the first letter, make it big
		// Append the rest as lowercase
		int wordsCount = words.Count();
		for (int i = 0; i < wordsCount; i++)
		{
			string word = words[i];
			int len = word.Length();
			if (len > 0)
			{
				string start = word.Get(0);
				start.ToUpper();
				string rem = word.Substring(1, len-1);
				rem.ToLower();
				
				words[i] = start + rem;
			}
		}
		
		// Reconstruct the string
		if (wordsCount > 0)
		{
			// Clear the out name
			name = "";
			
			// Append all other names
			for (int i = 0; i < wordsCount; i++)
			{
				
				//! Append space for every entry except last one
				if (i != wordsCount-1)
					name += words[i] + " ";
				else
					name += words[i];
			}
		}
		*/
		
		return name;
	}
	#endif
};