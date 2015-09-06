DELETE_OLD_MACROS = true

if (DELETE_OLD_MACROS) then
(
  SiegeMaxMacroscriptFiles = GetFiles (GetDir #UI + "/macroscripts/Siege Max Tools-*.mcr")

  for i = 1 to SiegeMaxMacroscriptFiles.count do
    deletefile SiegeMaxMacroscriptFiles[i]
)


---------------------------------------------------
MacroScript LaunchPreview
ButtonText:"Launch Previewer"
Category:"Siege Max Tools"
Tooltip:"Launch Previewer"
(
  fn IsSelectable =
  (
    try
    (
      if ($selection.count != 1) do return false
      if $selection[1] == undefined do return false
      if dsIsDummyObject $selection[1] do return false

      if not (canConvertTo $selection[1] Editable_Mesh) do return false

      local nm = dsLowerCase $selection[1].name
      if (nm.count > 2 and (nm[1] == "t") and nm[2] == "_" ) do return dsDoesNodeViewerExist()
      if (nm.count > 2 and (nm[1] == "m") and nm[2] == "_" ) do return dsDoesAnimViewerExist()

      if ($skinmesh != undefined) and (dsLowerCase ($selection[1].name) != "skinmesh") do return false

      nm = dsLowerCase (getfilenamefile maxfilename)
      if (nm.count > 2 and (nm[1] == "t") and nm[2] == "_" ) do return dsDoesNodeViewerExist()
      if (nm.count > 2 and (nm[1] == "a" or nm[1] == "m") and nm[2] == "_" ) do return dsDoesAnimViewerExist()
    )
    catch ()

    return false
  )

  on isVisible return true
  on isEnabled return IsSelectable()

  on Execute do
  (
    try
    (
      if ($.modifiers.count != 0) do
      (
        modPanel.setCurrentObject $.modifiers[1]
        subobjectLevel = 0
      )

      local oldtime = slidertime
      slidertime = 0  

      if ($.material == undefined) then
      (
        messageBox "The selected object has no material assigned to it!\n\nObjects must be texture mapped before you can export them" title:"Ooops!"
        dsPreviewSelected()
      )
      else if ((classOf $.material == MultiMaterial and classOf $.material[1].diffuseMap == BitmapTexture and not doesFileExist $.material[1].diffuseMap.fileName) or
               (classOf $.material == StandardMaterial and classOf $.material.diffuseMap == BitmapTexture and not doesFileExist $.material.diffuseMap.fileName)) then
      (
        messageBox "The selected object is assigned to a material with a diffuse map that points to a non-existant bitmap." title:"Ooops!"
        local mat = $.material
        $.material = undefined
        dsPreviewSelected()
        $.material = mat
      )
      else
      (
        dsPreviewSelected()
      )

      slidertime = oldtime
    )
    catch ()
  )
)


----------------------------------------------------
MacroScript ExportSno
ButtonText:"Export Siege Node"
Category:"Siege Max Tools"
Tooltip:"Export Siege Node File (SNO)"
(
  fn IsSelectable =
  (
    try
    (
      if ($selection.count != 1) do return false
      if ($selection[1] == undefined) do return false
      if (dsIsDummyObject $selection[1]) do return false

      if not (SelectedHasASNOModifier()) do return false

      if ($selection[1].material == undefined) do return false

      nm = dsLowerCase $selection[1].name
      if (nm.count > 2 and nm[1] == "t" and nm[2] == "_" ) do return true
    )
    catch ()

    return false
  )

  on isVisible return true
  on isEnabled return IsSelectable()

  on Execute do
  (
    try
    (
      if ($.modifiers.count != 0) do
      (
        modPanel.setCurrentObject $.modifiers[1]
        subobjectLevel = 0
      )

      local oldtime = slidertime
      slidertime = 0

      if ($.material == undefined) then
      (
        messageBox "The selected object has no material assigned to it!\n\nObjects must be texture mapped before you can export them" title:"Ooops!"
      )
      else
      (
        dsExportSelected()
      )

      slidertime = oldtime
    )
    catch ()
  )
)


----------------------------------------------------
MacroScript ImportMesh
ButtonText:"Import Mesh"
Category:"Siege Max Tools"
Tooltip:"Import Mesh File (ASP)"
(
  on isVisible return true
  on isEnabled return true

  on Execute do
  (
    try
      lanceImportASP()
    catch ()
  )
)


----------------------------------------------------
MacroScript ImportSno
ButtonText:"Import Siege Node"
Category:"Siege Max Tools" 
Tooltip:"Import Siege Node File (SNO)"
(
  on isVisible return true
  on isEnabled return true

  on Execute do
  (
    try
      lanceImportSNO()
    catch ()
  )
)


----------------------------------------------------
MacroScript ImportAnim
ButtonText:"Import Animation"
Category:"Siege Max Tools"
Tooltip:"Import Animation File (PRS)"
(
  fn IsSelectable =
  (
    try
    (
      if ($selection.count != 1) do return false
      if ($selection[1] == undefined) do return false
      if (dsIsDummyObject $selection[1]) do return false
      if ($selection[1].modifiers["Siege Max ASP"] == undefined) do return false
      if ($selection[1].modifiers["Skin"] == undefined) do return false

      nm = dsLowerCase $selection[1].name
      if (nm == "skinmesh") or (nm.count > 2 and nm[1] == "m" and nm[2] == "_" ) do return true
    )
    catch ()

    return false
  )

  on isVisible return true
  on isEnabled return IsSelectable()

  on Execute do
  (
    try
      lanceImportPRS()
    catch ()
  )
)


----------------------------------------------------
MacroScript ExportAnim
ButtonText:"Export Animation"
Category:"Siege Max Tools" 
Tooltip:"Export Animation File (PRS)" 
(
  fn IsSelectable= 
  (
    try (
      if ($selection.count != 1) then return false
      if $selection[1] == undefined then return false
      if dsIsDummyObject $selection[1] then return false
      
      nm = dsLowerCase $selection[1].name    
      
      --if (nm == "skinmesh") or (nm.count > 2 and nm[1] == "m" and nm[2] == "_" ) then
      
      if (nm == "skinmesh") then
      (
        nm = dsLowerCase (getfilenamefile maxfilename)
        if (nm.count > 2 and nm[1] == "a" and nm[2] == "_" ) then return true
      )
      
    )
    catch ()
    return false
  )
  
    on isVisible return true
    on isEnabled return IsSelectable()

  on Execute do
  (
    try
    (
      if $.modifiers.count != 0 then 
      (
        modPanel.setCurrentObject $.modifiers[1]
        subobjectLevel = 0
      )
      local oldtime = slidertime
      slidertime = 0
      if ($.material == undefined) then 
      (
        MessageBox "The selected object has no material assigned to it!\n\nObjects must be texture mapped before you can export them" title:"Ooops!"
      )
      else
      (
        dsExportSelected()
      )

      slidertime = oldtime
    )
    catch()
  )
)

----------------------------------------------------
MacroScript ExportMesh
ButtonText:"Export Mesh"
Category:"Siege Max Tools" 
Tooltip:"Export Mesh File (ASP)" 
(
  fn IsSelectable= 
  (
    try (
      if ($selection.count != 1) then return false
      if $selection[1] == undefined then return false
      if dsIsDummyObject $selection[1] then return false
      
      if ($selection[1].material == undefined) then return false      
      
      if not (canConvertTo $selection[1] Editable_Mesh) then return false

      nm = dsLowerCase $selection[1].name
      if (nm.count > 2 and nm[1] == "m" and nm[2] == "_" ) then return true
      
      if (nm == "skinmesh" ) then 
      (
        nm = dsLowerCase (getfilenamefile maxfilename)
        if (nm.count > 2 and nm[1] == "m" and nm[2] == "_" ) then return true
      )
    )
    catch ()
    return false
  )
  
    on isVisible return true
    on isEnabled return IsSelectable()

  on Execute do
  (
    try
    (
      if $.modifiers.count != 0 then 
      (
        modPanel.setCurrentObject $.modifiers[1]
        subobjectLevel = 0
      )
      local oldtime = slidertime
      slidertime = 0
      
      if ($.material == undefined) then 
      (
        MessageBox "The selected object has no material assigned to it!\n\nObjects must be texture mapped before you can export them" title:"Ooops!"
      )
      else
      (
        dsExportSelected()
      )

      slidertime = oldtime
    )
    catch()
  )
)

----------------------------------------------------
MacroScript AddRefEquipment
ButtonText:"Add Reference Equipment"
Category:"Siege Max Tools" 
Tooltip:"Add reference equipment" 
(
  fn IsSelectable = 
  (
    try (
      -- $$$ Need to make sure the grips are associated with this skinmesh!
      
      if ($selection.count != 1) then return false
      if $selection[1] == undefined then return false
      if dsIsDummyObject $selection[1] then return false
      
      if not (canConvertTo $selection[1] Editable_Mesh) then return false
      
      if (aspmoddef == undefined) or ((classof aspmoddef) != modifier) then return false
      if $selection[1].modifiers["Siege Max ASP"] == undefined then return false
      
      nm = dsLowerCase (getfilenamefile maxfilename)
      if (nm.count <= 2 or nm[1] != "a" or nm[2] != "_" ) then return false
      
      if $weapon_grip == undefined then return false
      if $shield_grip == undefined then return false
      if $grip_reference_*.count == 1 then return false
      return true
    )
    catch()
    return false
  )
  
    on isVisible return true 
    on isEnabled return IsSelectable()

  on Execute do
  (
    try (
      UnEquipEverything false 
      
      EquipWeaponUsingStance $selection[1] false 
      EquipShieldUsingStance $selection[1] false
    )
    catch()
  )
  
)

----------------------------------------------------
MacroScript RemoveRefEquipment
ButtonText:"Remove Reference Equipment"
Category:"Siege Max Tools" 
Tooltip:"Remove reference equipment" 
(
  fn IsSelectable= 
  (
    try (
      -- $$$ Need to make sure the grips are associated with this skinmesh!
      if ($selection.count != 1) then return false
      if $selection[1] == undefined then return false
      if dsIsDummyObject $selection[1] then return false
      
      if not (canConvertTo $selection[1] Editable_Mesh) then return false
      
      if (aspmoddef == undefined) or ((classof aspmoddef) != modifier) then return false
      if $selection[1].modifiers["Siege Max ASP"] == undefined then return false
          
      nm = dsLowerCase (getfilenamefile maxfilename)
      if (nm.count <= 2 or nm[1] != "a" or nm[2] != "_" ) then return false
      
      if $weapon_grip == undefined then return false
      if $shield_grip == undefined then return false
      if ($grip_reference_*).count == 0 then return false
      return true
    )
    catch()
    return false
  )
  
    on isVisible return true
    on isEnabled return IsSelectable()

  on Execute do
  (
    try (
      UnEquipEverything false 
    )
    catch()
  )
  
)

----------------------------------------------------
MacroScript ShowTracers
ButtonText:"Show Tracers"
Category:"Siege Max Tools" 
Tooltip:"Add tracer boxes to the scene" 
(
  fn IsSelectable = 
  (
    try (
      -- $$$ Need to make sure the grips are associated with this skinmesh!
      if ($selection.count != 1) then return false
      if $selection[1] == undefined then return false
      if dsIsDummyObject $selection[1] then return false
      
      if not (canConvertTo $selection[1] Editable_Mesh) then return false
      
      if (aspmoddef == undefined) or ((classof aspmoddef) != modifier) then return false
      if $selection[1].modifiers["Siege Max ASP"] == undefined then return false
      
      nm = dsLowerCase (getfilenamefile maxfilename)
      if (nm.count < 2 or nm[1] != "a" or nm[2] != "_" ) then return false
      
      return ($__tracerbox__*).count == 0
    )
    catch()
    return false
  )
  
    on isVisible return true
    on isEnabled return IsSelectable()

  on Execute do
  (
    try (
      dsglb_tracers = #()
      dscb_CalculateTracers()
      for s in dsglb_tracers do
      (
        b = box  pos:s[2] name:"__tracerbox__"
        in coordsys local b.rotation = s[3  ]
      )
      dsglb_tracers = #()
    )
    catch()
  )
  
)

----------------------------------------------------
MacroScript RemoveTracers
ButtonText:"Remove Tracers"
Category:"Siege Max Tools" 
Tooltip:"Delete tracer boxes from the scene" 
(
    on isVisible return true
    on isEnabled return ($__tracerbox__*).count > 0

  on Execute do
  (
    delete $__tracerbox__*
  )
  
)

----------------------------------------------------
MacroScript AddLoopEvents
ButtonText:"Add Loop Events"
Category:"Siege Max Tools" 
Tooltip:"Add Begin/End events to looping anim" 
(
  fn IsSelectable = 
  (
    try (
      if ($selection.count != 1) then return false
      if $selection[1] == undefined then return false
      if dsIsDummyObject $selection[1] then return false
      
      local nm = dsLowerCase $selection[1].name
      if (nm != "skinmesh") and (nm.count < 2 or nm[1] != "m" or nm[2] != "_" ) then
      (
        return false
      )
      
      nm = dsLowerCase (getfilenamefile maxfilename)
      if (nm.count < 2 or nm[1] != "a" or nm[2] != "_" ) then return false
            
      return not (HasLoopEventAtAnimEnd $selection[1])
    )
    catch()
    return false
  )
  
    on isVisible return true
    on isEnabled return IsSelectable()

  on Execute do
  (
    try (
      AddBegEndEvents()
    )
    catch()
  )
  
)

----------------------------------------------------
MacroScript RemoveLoopEvents
ButtonText:"Remove Loop Events"
Category:"Siege Max Tools" 
Tooltip:"Remove Begin/End events from anim" 
(
  fn IsSelectable = 
  (
    try (
      if ($selection.count != 1) then return false
      if $selection[1] == undefined then return false
      if dsIsDummyObject $selection[1] then return false
      
      local nm = dsLowerCase $selection[1].name
      if (nm != "skinmesh") and (nm.count < 2 or nm[1] != "m" or nm[2] != "_" ) then
      (
        return false
      )
      
      nm = dsLowerCase (getfilenamefile maxfilename)
      if (nm.count < 2 or nm[1] != "a" or nm[2] != "_" ) then return false
      
      return HasLoopEventAtAnimEnd $selection[1]
    )
    catch()
    return false
  )
  
    on isVisible return true
    on isEnabled return IsSelectable()

  on Execute do
  (
    try (
      DeleteBegEndEvents()
    )
    catch()
  )
  
)

----------------------------------------------------
MacroScript OpenAnimEventTool
ButtonText:"Open Anim Event Tool"
Category:"Siege Max Tools" 
Tooltip:"Open the anim event editing tool" 
(
  fn IsSelectable = 
  (
    try (
      if ($selection.count != 1) then return false
      if $selection[1] == undefined then return false
      if dsIsDummyObject $selection[1] then return false
      
      local nm = dsLowerCase $selection[1].name
      if (nm != "skinmesh") and (nm.count < 2 or nm[1] != "m" or nm[2] != "_" ) then
      (
        return false
      )
        
      nm = dsLowerCase (getfilenamefile maxfilename)
      if (nm.count > 2 and nm[1] == "a" and nm[2] == "_" ) then return true
    )
    catch()
    return false
  )
  
    on isVisible return true
    on isEnabled return IsSelectable()

  on Execute do
  (
    try (      
      if (critpointfloat != undefined) then closerolloutfloater critpointfloat
      critpointfloat = newRolloutFloater ("Events")  220 582 
      addrollout CriticalEventsFloater critpointfloat 
    )
    catch()
  )
  
)
----------------------------------------------------
MacroScript ListAnimEvents
ButtonText:"List Anim Events"
Category:"Siege Max Tools" 
Tooltip:"Show dialog with current anim event list" 
(
  fn IsSelectable = 
  (
    try (
      if ($selection.count != 1) then return false
      if $selection[1] == undefined then return false
      if dsIsDummyObject $selection[1] then return false
      
      local nm = dsLowerCase $selection[1].name
      if (nm != "skinmesh") and (nm.count < 2 or nm[1] != "m" or nm[2] != "_" ) then
      (
        return false
      )
      
      if (aspmoddef == undefined) or ((classof aspmoddef) != Modifier) then return false
      if ($selection[1].Modifiers["Siege Max ASP"]) == undefined then return false  
    
      nm = dsLowerCase (getfilenamefile maxfilename)
      if (nm.count > 2 and nm[1] == "a" and nm[2] == "_" ) then return true
    )
    catch()
    return false
  )
  
    on isVisible return true
    on isEnabled return IsSelectable()

  on Execute do
  (
    try (
      ListCriticalEvents() 
    )
    catch()
  )
  
)

----------------------------------------------------
MacroScript AddRootPosMarker
ButtonText:"Add/Reset Root Marker"
Category:"Siege Max Tools" 
Tooltip:"Adds or resets the root position marker" 
(
  fn IsSelectable = 
  (
    try (
      if ($selection.count != 1) then return false
      if $selection[1] == undefined then return false
      if dsIsDummyObject $selection[1] then return false
      
      local nm = dsLowerCase $selection[1].name
      if (nm != "skinmesh") and (nm.count < 2 or nm[1] != "m" or nm[2] != "_" ) then
      (
        return false
      )
      
      local itm = selection[1]
      local dummyroot = execute ("$'dummyroot_"+nm+"'")
      
      if (dummyroot != undefined) then (
        return false
      )
      
      if (nm.count > 2 and nm[1] == "m" and nm[2] == "_" ) then return true
      nm = dsLowerCase (getfilenamefile maxfilename)
      if (nm.count > 2 and (nm[1] == "a" or nm[1] == "m") and nm[2] == "_" ) then return true
    )
    catch()
    return false
  )
  
    on isVisible return true
    on isEnabled return IsSelectable()

  on Execute do
  (
    try
    (
      ResetRootPosMarker()
    )
    catch
    (
      format "Unable to load new ROOT object from Reference_Scenes\GPG_RefObjs.gmax"
    )
  )
  
)

----------------------------------------------------
MacroScript AnimateRootPosMarker
ButtonText:"Animate Root Marker"
Category:"Siege Max Tools" 
Tooltip:"Set first and last keys for root position marker" 
(
  fn IsSelectable = 
  (
    try (
      if ($selection.count != 1) then return false
      if $selection[1] == undefined then return false
      if dsIsDummyObject $selection[1] then return false
      
      local nm = dsLowerCase $selection[1].name
      if (nm != "skinmesh") and (nm.count < 2 or nm[1] != "m" or nm[2] != "_" ) then
      (
        return false
      )
      
      nm = dsLowerCase (getfilenamefile maxfilename)
      if (nm.count > 2 and nm[1] == "a" and nm[2] == "_" ) then return true
    )
    catch()
    return false
  )
  
    on isVisible return true
    on isEnabled return IsSelectable()

  on Execute do
  (
    AddFirstLastRootPosMarkerKeys $
  )
  
)

----------------------------------------------------
MacroScript OpenItemToolkit
ButtonText:"Open Item Tool Kit"
Category:"Siege Max Tools" 
Tooltip:"Open Item Tool Kit" 
(
  fn IsSelectable = 
  (
    try (
      if ($selection.count != 1) then return false
      if $selection[1] == undefined then return false
      if dsIsDummyObject $selection[1] then return false
            
      if not (canConvertTo $selection[1] Editable_Mesh) then return false

      nm = dsLowerCase $selection[1].name
      if (nm.count > 2 and nm[1] == "m" and nm[2] == "_" ) then return true
      
      if (nm == "skinmesh" ) then 
      (
        nm = dsLowerCase (getfilenamefile maxfilename)
        if (nm.count > 2 and nm[1] == "m" and nm[2] == "_" ) then return true
      )
    )
    catch()
    return false
  )
  
    on isVisible return IsSelectable()
    on isEnabled return true

  on Execute do
  (
    OpenHelpTools()
  )
  
)

----------------------------------------------------
MacroScript OpenASPToolkit
ButtonText:"Open Aspect Tool Kit"
Category:"Siege Max Tools" 
Tooltip:"Open Aspect Tool Kit" 
(
  fn SelectedIsAnASPorPRS = 
  (
    try
    (    
      if (aspmoddef == undefined) or ((classof aspmoddef) != modifier) then return false
      if (snomoddef == undefined) or ((classof snomoddef) != modifier) then return false
      
      if ($selection.count != 1) then return false
      if $selection[1] == undefined then return false
      if dsIsDummyObject $selection[1] then return false
      
      nm = dsLowerCase $selection[1].name 
      if (nm.count > 2 and (nm[1] == "m") and nm[2] == "_" ) then return true  
      if (nm != "skinmesh" ) then return false
      
      nm = dsLowerCase (getfilenamefile maxfilename)
      if (nm.count > 2 and (nm[1] == "m") and nm[2] == "_" ) then return true  
    )
    catch ()
    return false
  )

  On isVisible return SelectedIsAnASPorPRS() 
  On isEnabled return true
  
  On execute Do  
  (    
    if SelectedHasASNOModifier() then 
    (
      MessageBox("Can't open ASP Tool Kit with a SNO modifier on stack")
      return false
    )

    if ($selection[1].modifiers["Siege Max ASP"]) == undefined then 
    (
      try 
      (
        AddModifier $selection[1] (ASPModDef())
      )
      catch( return false )
    )

    gASPPopUp()
    
    return true

  )
  
)

----------------------------------------------------
MacroScript OpenNeckStitchToolkit
ButtonText:"Open Neck Stitch Tool Kit"
Category:"Siege Max Tools" 
Tooltip:"Open Neck Stitch Tool Kit" 
(
  fn SelectedIsAnASP = 
  (
    try
    (    
      if (aspmoddef == undefined) or ((classof aspmoddef) != Modifier) then return false
      if (snomoddef == undefined) or ((classof snomoddef) != Modifier) then return false
      
      if ($selection.count != 1) then return false
      if $selection[1] == undefined then return false
      if dsIsDummyObject $selection[1] then return false
      
      local nm = dsLowerCase $selection[1].name 
      if (nm.count > 2 and (nm[1] == "m") and nm[2] == "_" ) then return true  
      
      if (nm != "skinmesh" ) then return false
      nm = dsLowerCase (getfilenamefile maxfilename)
      if (nm.count > 2 and nm[1] == "m" and nm[2] == "_" ) then return true  
    )
    catch ()
    return false
  )

  On isVisible return SelectedIsAnASP() 
  On isEnabled return true
    
  On execute Do  
  (    
    if SelectedHasASNOModifier() then 
    (
      MessageBox("Can't open Neck Stitch Tool Kit with a SNO modifier on stack")
      return false
    )
    
    if ($selection[1].Modifiers["Siege Max ASP"]) == undefined then 
    (
      max modify mode
      
      try 
      (
        AddModifier $selection[1] (ASPModDef())              
      )
      catch( return false )
      
      -- Switch over to modify mode, so we can adjust the ASP parameters
    )
    
    OpenNeckToolkit()
    
    return true
  )
  
)

----------------------------------------------------
MacroScript OpenSNOToolkit
ButtonText:"Open Siege Node Tool Kit"
Category:"Siege Max Tools" 
Tooltip:"Open Siege Node Tool Kit" 
(
  fn SelectedIsASNO = 
  (
    try
    (
      if (snomoddef == undefined) or ((classof snomoddef) != Modifier) then return false
      
      if ($selection.count != 1) then return false
      if $selection[1] == undefined then return false
      if dsIsDummyObject $selection[1] then return false  
      
      local nm = dsLowerCase $selection[1].name
      if (nm.count > 2 and (nm[1] == "t") and nm[2] == "_" ) then return true  
    )
    catch ()
    return false
  )

  On isVisible do
  (
    if ( SelectedIsASNO() ) then
    (
      return true
    )
    else
    (
      return false
    )
  )    
  
  On isEnabled return true;
  
  On execute Do  
  (    
    if $selection[1].modifiers["Siege Max ASP"] != undefined then 
    (
      MessageBox("Can't open Siege Node Tool Kit with an ASP modifier on stack")
      return false
    )

    if not SelectedHasASNOModifier() then 
    (
      max modify mode
      
      try 
      (
        AddModifier $selection[1] (SNOModDef())              
      )
      catch( return false )  
    )
    
    gSNOPopUp()
    
    return true
  )
  
)

----------------------------------------------------
MacroScript SelectAllSNOs
ButtonText:"Select All Siege Nodes"
Category:"Siege Max Tools" 
Tooltip:"Select All Siege Nodes" 
(
  fn HasASNOmod n = 
  (
    try
    (
      if ($selection[1].modifiers["Siege Max SNO"]) != undefined then return true
    )
    catch ()
    return false
  )

  On isEnabled return true;
  On isEnabled return true;
  
  On execute Do  
  (    
    if (snomoddef == undefined) or ((classof snomoddef) != Modifier) then return false
    
    snonodes = for n in $T_* where (HasASNOmod n) collect n
    
    select snonodes
    
    return true
  )
  
)

----------------------------------------------------
MacroScript SelectAllNonSNOs
ButtonText:"Select All Without Siege Nodes"
Category:"Siege Max Tools" 
Tooltip:"Select All Without Siege Nodes" 
(
  fn HasASNOmod n = 
  (
    try
    (
      if ($selection[1].modifiers["Siege Max SNO"]) != undefined then return true
    )
    catch ()
    return false
  )

  On isEnabled return true;
  On isEnabled return true;
  
  On execute Do  
  (    
    if (snomoddef == undefined) or ((classof snomoddef) != Modifier) then return false
    
    snonodes = for n in $T_* where (not HasASNOmod n) collect n
    
    select snonodes
    
    return true
  )
  
)

----------------------------------------------------
MacroScript OpenRefDropperToolkit
ButtonText:"Open RefObj Dropper"
Category:"Siege Max Tools" 
Tooltip:"Open Reference Object Dropper" 
(
  On isVisible return true 
  On isEnabled return true
    
  On execute Do  
  (    
    try
    (
      OpenRefObjDropper()
    )
    catch()
    
    return true
  )
  
)

----------------------------------------------------
MacroScript NamingKeyLookup
ButtonText:"Check Siege Max Name"
Category:"Siege Max Tools" 
Tooltip:"Check Siege Max Name" 
(
  On isVisible return true 
  On isEnabled return $selection.count == 1
    
  On execute Do  
  (    
    try
    (
      local lookname = dsLowerCase $selection[1].name
      local isskinmesh = false
      
      if lookname == "skinmesh" then
      (
        lookname = dolowercase (getfilenamefile maxfilename)
        isskinmesh = true
      )
              
      if lookname[1] == "m" or lookname[1] == "t" or lookname[1] == "a" then
      (
        addr = dsBuildContentLocation lookname
      )
      else
      (
        addr = undefined
      )
      
      local msg = stringstream ""
      if (addr == undefined) then
      (
        format "The selected object:\n\n\t%\n\nhas an invalid export name\n\nYou will not be able export it!" lookname to:msg
      )
      else
      (
        local lookdesc
        
        if lookname[1] == "m" then
        (
          type = "aspect mesh"
          addr = addr + ".ASP"
        )
        else if lookname[1] == "t" then
        (
          type = "terrain node"
          addr = addr + ".SNO"
        )
        else if lookname[1] == "a" then
        (
          type = "animation"
          addr = addr + ".PRS"
        )    
        
        if (isskinmesh) then
        (
          lookdesc = lookname + "   (objects named \"SkinMesh\" use filename)"
        )
        else
        (
          lookdesc = lookname
        )
        
        format "The selected %:\n\n\t%\n\ncan be exported as:\n\n\t%" type lookdesc addr to:msg 
      )
      MessageBox msg title:"Naming Key Lookup"
    )
    catch()
    
    return true
  )
  
)


----------------------------------------------------
menuMan.loadMenuFile "../gamepacks/siegemax/ui/SiegeMaxQuadMenus.mnu"