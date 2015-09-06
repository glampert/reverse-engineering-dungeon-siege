----------------------------------------------------
--
-- FILE: InstallLanceImporter.ms
-- AUTH: biddle
--
-- Copy this file into the folder:
--
--   gmax/gamepacks/SiegeMax/Scripts/Add-Ons
--
-- to access the importer menu items it defines
-- 
----------------------------------------------------

----------------------------------------------------

PrepareCategoryForHousekeeping "Lance Import Tools"	

-- Interpret files that define the importer rollouts
-- You need to get these importers from Team Elemental

scriptdir = GetDir #scripts
fileIn (scriptdir+"/SNOImport.ms")
fileIn (scriptdir+"/PRSImport.ms")
fileIn (scriptdir+"/ASPImport.ms")

----------------------------------------------------
MacroScript LaunchSNOImporter
ButtonText:"SNO Import"
Category:"Lance Import Tools" 
Tooltip:"Launch SNO Importer" 
(
	On isVisible return true 
	On isEnabled return true
  
	On execute Do  
	(    
		try
		(
			lanceImportSNO()
		)
		catch()	    
		return true
	)
)

----------------------------------------------------
MacroScript LaunchPRSImporter
ButtonText:"PRS Import"
Category:"Lance Import Tools" 
Tooltip:"Launch PRS Importer" 
(
	On isVisible return true 
	On isEnabled return true
  
	On execute Do  
	(    
		try
		(
			lanceImportPRS()
		)
		catch()	    
		return true
	)
)

----------------------------------------------------
MacroScript LaunchASPImporter
ButtonText:"ASP Import"
Category:"Lance Import Tools" 
Tooltip:"Launch ASP Importer" 
(
	On isVisible return true 
	On isEnabled return true
  
	On execute Do  
	(    
		try
		(
			lanceImportASP()
		)
		catch()	    
		return true
	)
)

----------------------------------------------------
fn ExtendQuadMenus = 
(

	fn FindSubMenuItemsByName m searchname = (
	
		for smi = 1 to m.numItems() do
		(
			local sm = m.getItem smi
			local smt = sm.gettitle()
			if smt == searchname then
			(
				return sm
			)
		)
		return undefined
	)
	
	local dvq ,qm, mn, smi, nsm, nmi

	dvq = menuMan.findQuadMenu "Default Viewport Quad" 
	
	-- Delete old quad menu items tied to our context (if they exist)

	qm= dvq.getMenu 4 -- menu 4 is the lower left quad
	
	mn= FindSubMenuItemsByName qm ("Context "+"Lance Import Tools")
	if (mn!= undefined) then qm.RemoveItem mn
	
	-- Now add in the new quad menu item

	mn = menuMan.createMenu ("Context "+"Lance Import Tools") 

	-- Create a sub menu item, so that we can add it
	smi = menuMan.createSubMenuItem "" mn 
	smi.setDisplayFlat true
	qm.AddItem smi -1

	-- Add in a separators
	
	nmi = menuMan.createSeparatorItem()
	mn.AddItem nmi -1
	
	-- Make a sub-menu that calls the three importer launchers
	
	nsm = menuMan.createMenu "Lance Import Tools" 
	nsmi = menuMan.createSubMenuItem "" nsm 
	
	-- Create a sub-sub menu items

	mn.AddItem nsmi -1
	nmi = menuMan.createActionItem "LaunchASPImporter" "Lance Import Tools" 
	nsm.AddItem nmi -1
	nmi = menuMan.createActionItem "LaunchPRSImporter" "Lance Import Tools" 
	nsm.AddItem nmi -1
	nmi = menuMan.createActionItem "LaunchSNOImporter" "Lance Import Tools" 
	nsm.AddItem nmi -1
			
)

----------------------------------------------------
-- Run the quad menu extender we just defined...
ExtendQuadMenus()

