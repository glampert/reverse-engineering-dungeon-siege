----------------------------------------------------------------------------------------
--
-- Scripted Modifier Plugin for ASPECTS
--
----------------------------------------------------------------------------------------

GLOBAL gASPPopUp

plugin modifier ASPModDef 
name:"Siege Max ASP" 
classID:#(0x4dac926f, 0x9e5623ef)
extends:Mesh_Select replaceUI:false version:1 
category:"Siege Max"
( 

	local tHeadBits = #{} 
	local tHandBits = #{}
	local tFeetBits = #{}
	
	local STENCIL_ALPHA_BIT		=  1
	local DISABLE_LIGHTING_BIT	=  2
	local VERTEX_ALPHA_BIT		= 16
	
	parameters ASPDATA
	(
		RenderFlags			type:#integer	animatable:false	
		HeadFaceList		type:#intTab	animatable:false tabsizevariable:true	
		HandFaceList		type:#intTab	animatable:false tabsizevariable:true
		FeetFaceList		type:#intTab	animatable:false tabsizevariable:true
		StitchVertList		type:#intTab	animatable:false tabsizevariable:true
		StitchTagList		type:#stringTab	animatable:false tabsizevariable:true
		
	)
	
	rollout ModActions "Siege Max ASP"
	(
		button OpenASPToolkit "Open ASP Tool Kit"
		
		on OpenASPToolkit pressed do ( gASPPopUp() )
	)
		
	fn RebuildInternals =
	(
		this.tHeadBits = #{}
		for i in this.HeadFaceList do append this.tHeadBits i
		
		this.tHandBits = #{}
		for i in this.HandFaceList do append this.tHandBits i
		
		this.tFeetBits = #{}
		for i in this.FeetFaceList do append this.tFeetBits i						
	)
		
	on create do 
	(		
		
		--print "Creating!"
		
		this.tHeadBits = #{}
		this.tHandBits = #{}
		this.tFeetBits = #{}
	
		this.RenderFlags = 0
			
		this.HeadFaceList = #()			
		this.HandFaceList = #()		
		this.FeetFaceList = #()
		this.StitchTagList = #()
		this.StitchVertList = #()
			
		try 
		(
			local oldmod = $selection[1].modifiers["Siege Max ASP"]
			
			if (classof oldmod) == (classof this) then
			(		
				-- format "Pulling new ASPDATA out of another ASP MOD\n"
				
				this.RenderFlags = oldmod.RenderFlags 
				this.HeadFaceList = oldmod.HeadFaceList
				this.HandFaceList = oldmod.HandFaceList 
				this.FeetFaceList = oldmod.FeetFaceList 
				this.StitchTagList = oldmod.StitchTagList 
				this.StitchVertList = oldmod.StitchVertList				
				this.RebuildInternals()
			)
		)
		catch
		(
			format "Encountered an error while pulling ASPDATA!\n"
		)
		
	)
		
	on update do 
	(	
		this.RebuildInternals()
	)
	
	on load do 
	(
		this.RebuildInternals()
	)
		
) 
