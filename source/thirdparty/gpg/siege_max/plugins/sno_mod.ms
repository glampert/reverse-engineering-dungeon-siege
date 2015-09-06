----------------------------------------------------------------------------------------
--
-- Scripted Modifier Plugin for SIEGE NODES
--
-- 8/19/02: fixed a problem with using ign/lok/nrd in sno toolkit -lance
--
----------------------------------------------------------------------------------------

global gSNOPopUp

plugin modifier SNOModDef 
name:"Siege Max SNO" 
classID:#(0x7f621ebb, 0x6850842e)
extends:Mesh_Select replaceUI:false version:1 
category:"Siege Max"
( 
	parameters SNODATA
	(
		DoorEdgeList		type:#intTab	animatable:false tabsizevariable:true
		DoorLastEdges		type:#intTab	animatable:false tabsizevariable:true
		DoorVertList		type:#intTab	animatable:false tabsizevariable:true
		DoorLastVerts		type:#intTab	animatable:false tabsizevariable:true	
		DoorDirectionList	type:#point3Tab	animatable:false tabsizevariable:true

		FloorFaceList		type:#intTab	animatable:false tabsizevariable:true
		
		WaterFaceList		type:#intTab	animatable:false tabsizevariable:true
		
		IgnoredFaceList		type:#intTab	animatable:false tabsizevariable:true
		
		LockedNormVertList	type:#intTab	animatable:false tabsizevariable:true

		DoNotRoundVertList	type:#intTab	animatable:false tabsizevariable:true	
	)
	
	local tDoorList = #()
	local tFloorBits = #{} 
	local tWaterBits = #{}
	local tIgnoredBits = #{}
	local tLockedNormBits = #{}
	local tDoNotRoundBits = #{}
	local tSelectedDoor = undefined

	struct SiegeDoor (verts,edges)
	
	rollout ModActions "Siege Max SNO"
	(
		button OpenSNOToolkit "Open SNO Tool Kit"
		
		on OpenSNOToolkit pressed do ( gSNOPopUp() )
	)
		
		
	fn RebuildInternals =
	(
		tFloorBits = #{}
		for ffl in FloorFaceList do append tFloorBits ffl
		
		tWaterBits = #{}
		for wfl in WaterFaceList do append tWaterBits wfl
		
		tIgnoredBits = #{}
		for ifl in IgnoredFaceList do append tIgnoredBits ifl
		
		tLockedNormBits = #{}
		for iln in LockedNormVertList do append tIgnoredBits iln 

		tDoNotRoundBits = #{}
		for drn in DoNotRoundVertList do append tDoNotRoundBits drn 
		
		tDoorList = #()

		for i = 1 to DoorLastEdges.count do
		(
			local vmin = if i > 1 then (DoorLastVerts[i-1]+1) else 1
			local vmax = DoorLastVerts[i]
			local doorverts = for v = vmin to vmax collect DoorVertList[v]
			
			local emin = if i > 1 then (DoorLastEdges[i-1]+1) else 1
			local emax = DoorLastEdges[i]
			local dooredges = for e = emin to emax collect DoorEdgeList[e]
			
			append tDoorList (SiegeDoor doorverts dooredges)
		)
				
	)

	on create do 
	(		
		tFloorBits = #{}
		tWaterBits = #{}
		tIgnoredBits = #{}
		tLockedNormBits = #{}
   		tDoNotRoundBits = #{}
		
		tDoorList = #()
		tSelectedDoor = undefined
		
		DoorEdgeList = #()
		DoorLastEdges = #()
		DoorVertList = #()
		DoorLastVerts = #()
		
		DoorDirectionList = #()
		
		FloorFaceList = #()			
		WaterFaceList = #()		
		IgnoredFaceList = #()
		LockedNormVertList = #()	
		DoNotRoundVertList = #()
		
		--try 
		(
			local oldmod = $selection[1].modifiers["Siege Max SNO"]

			if (classof oldmod) == (classof this) then
			(		
				-- format "Pulling new SNODATA out of another SNO MOD\n"
				
				DoorEdgeList = oldmod.DoorEdgeList 
				DoorLastEdges = oldmod.DoorLastEdges 
				DoorVertList = oldmod.DoorVertList 
				DoorLastVerts = oldmod.DoorLastVerts 
				DoorDirectionList = oldmod.DoorDirectionList 
				
				FloorFaceList = oldmod.FloorFaceList 		
				WaterFaceList = oldmod.WaterFaceList 				
				IgnoredFaceList = oldmod.IgnoredFaceList 				
				LockedNormVertList = oldmod.LockedNormVertList 				
				DoNotRoundVertList = oldmod.DoNotRoundVertList 				
				
				RebuildInternals()
			)
		)
		--catch
		--(
		--	format "Encountered an error while pulling SNODATA!\n"
		--)
	)
		
	on update do 
	(	
		--format "Updating SNOMODDEF version %\n" version
		RebuildInternals()
	)
	
	on load do 
	(
		--print "Loading SNO"
		RebuildInternals()
	)
	
) 

