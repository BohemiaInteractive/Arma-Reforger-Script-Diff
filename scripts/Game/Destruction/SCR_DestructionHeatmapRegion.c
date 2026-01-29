class SCR_DestructionHeatmapRegion: DestructionHeatmapRegion
{
	event override bool MergeHeatmapEntries(notnull DestructionHeatmapEntry mainEntry, notnull DestructionHeatmapEntry newEntry)
	{
		//too big already, we dont merge
		if(mainEntry.GetWeight() > 50)
			return false;
		
		vector mins1 = mainEntry.GetMins();
		vector maxs1 = mainEntry.GetMaxs();
		
		vector mins2 = newEntry.GetMins();
		vector maxs2 = newEntry.GetMaxs();
		
		bool intersects = Math3D.IntersectionBoxBox(mins1, maxs1, mins2, maxs2);
	
		if(!intersects)
			return false;
		
		vector newMins = SCR_Math3D.Min(mins1, mins2);
		vector newMaxs = SCR_Math3D.Max(maxs1, maxs2);
		
		int newWeight = mainEntry.GetWeight() + newEntry.GetWeight();
		
		mainEntry.SetBoundingBox(newMins, newMaxs);
		mainEntry.SetWeight(newWeight);
		
		return true;
	}
}
	
