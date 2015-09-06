-- Dungeon Siege PRS Import (Siege Max)
-- Lance ><>
-- 8/22/2002
-- modified by Xaa and ghastley


rollout PRSImport "PRS Import"
(
  local ANIM = 1296649793
  local NOTE = 1163153230
  local TRCR = 1380143700
  local RKEY = 1497713490
  local KLST = 1414745163
  local AEND = 1145980225
  local INFO = 1330007625
  local V3_0 = 3
  local V4_0 = 4 -- introduced with siege max
  local V4_1 = 260
  local v5_0 = 5     -- DS LoA

  local tokenList =
  #(
    1279739202, -- BEGL Begin Loop
    1279544901, -- ENDL End Loop
    1313097292, -- LFDN Left Foot Down
    1347765836, -- LFUP Left Foot Up
    1313097298, -- RFDN Right Foot Down
    1347765842, -- RFUP Right Foot Up
    827868755,  -- SFX1 Special Effect 1
    844645971,  -- SFX2 Special Effect 2
    861423187,  -- SFX3 Special Effect 3
    878200403,  -- SFX4 Special Effect 4
    1196905282, -- BSWG Begin Swing/Cast
    1196905285, -- ESWG End Swing/Cast
    1163020614, -- FIRE Fire/Strike with Weapon
    1145128260, -- DEAD Die/Explode
    1096045633, -- ATTA Attach Ammo
    1162103112, -- HIDE Hide Mesh
    1464813651  -- SHOW Show Mesh
  )

  struct strNOTE ( time, token )
  struct strTRCR ( pos )
  struct strKeyList ( numRotKeys, numPosKeys, rotKeys, posKeys )
  struct strRotKey ( time, rot )
  struct strPosKey ( time, pos )

  local prsFileIn
  local aspFileIn
  local dataNOTE
  local dataTRCR
  local dataRKEY
  local dataKLST

  local numBones
  local numNotes
  local numTracers
  local animLength
  local boneNames


  group "Options"
  (
    spinner spn_Scale "Scale:" range:[0.0001,1000.0,1.0] type:#float scale:0.01
    spinner spn_Fps   "FPS:"   range:[1,1000,12] type:#integer scale:1
    checkbox chk_Debug "Show Debug Info" checked:false
  )
  button btn_ImportNow "Import..." width:100


  fn FlipUp v =
  (
    return ([v.x,-v.z,v.y] as point3)
  )


  fn FlipUpQuat q =
  (
    return (quat q.x -q.z q.y q.w)
  )


  fn ReadVector fin =
  (
    local x = ReadFloat fin
    local y = ReadFloat fin
    local z = ReadFloat fin

    return ([x,y,z] as point3)
  )


  fn ReadQuat fin =
  (
    local x = ReadFloat fin
    local y = ReadFloat fin
    local z = ReadFloat fin
    local w = ReadFloat fin

    return (quat x y z w)
  )


  fn DotQuat q1 q2 =
  (
    return ((q1.x * q2.x) + (q1.y * q2.y) + (q1.z * q2.z) + (q1.w * q2.w))
  )


  fn QuatApply v q =
  (
    local m = q as matrix3
    local x = dot m.row1 v
    local y = dot m.row2 v
    local z = dot m.row3 v

    return ([x,y,z] as point3)
  )


  fn ReadKeyList fin =
  (
    local nrk = ReadLong fin
    local npk = ReadLong fin

    local rk = #()
    for i = 1 to nrk do
    (
      local t = ReadFloat fin
      local r = ReadQuat fin
      rk[i] = strRotKey time:t rot:r
    )

    local pk = #()
    for i = 1 to npk do
    (
      local t = ReadFloat fin
      local p = ReadVector fin
      pk[i] = strPosKey time:t pos:p
    )

    return (strKeyList numRotKeys:nrk numPosKeys:npk rotKeys:rk posKeys:pk)
  )


  fn ReadANIM fin =
  (
    if (chk_Debug.checked) do
      format "debug: reading ANIM section...\n"
    local version = ReadLong fin

    local sizeTextField = ReadLong fin
    numBones = ReadLong fin
    animLength = ReadFloat fin
    local rootTravel = ReadVector fin
    local unkrot1 = ReadQuat fin
    local unkrot2 = ReadQuat fin
    local unk = ReadFloat fin

    boneNames = #()
    while (sizeTextField > 0) do
    (
      append boneNames (ReadString fin)
      local size = boneNames[boneNames.count].count + 1
      local padding = mod (4 - (mod size 4)) 4
      for i = 1 to padding do
        ReadByte fin -- throw away null padding
      sizeTextField -= size + padding
    )

    dataKLST = #()
  )


  fn ReadNOTE fin =
  (
    if (chk_Debug.checked) do
      format "debug: reading NOTE section...\n"
    local version = ReadLong fin

    numNotes = ReadLong fin

    dataNOTE = #()
    for i = 1 to numNotes do
    (
      local t = ReadFloat fin
      local k = ReadLong fin
      dataNOTE[i] = strNOTE time:t token:k
    )
  )


  fn ReadTRCR fin =
  (
    if (chk_Debug.checked) do
      format "debug: reading TRCR section...\n"
    local version = ReadLong fin

    numTracers = ReadLong fin

    messageBox "oh no, tracers!"
    -- ???
  )


  fn ReadRKEY fin =
  (
    if (chk_Debug.checked) do
      format "debug: reading RKEY section...\n"
    local version = ReadLong fin

    dataRKEY = ReadKeyList fin
  )


  fn ReadKLST fin =
  (
    if (chk_Debug.checked) do
      format "debug: reading KLST section...\n"
    local version = ReadLong fin

    local i = ReadLong fin
    local o = ReadLong fin -- text offset

    dataKLST[i + 1] = ReadKeyList fin
  )


  fn ReadAEND fin =
  (
    if (chk_Debug.checked) do
      format "debug: reading AEND section...\n"

    -- INFO section
    if (chk_Debug.checked) do
      format "debug: reading INFO section...\n"

    if (INFO != ReadLong fin) do
      messageBox "error: INFO expected marker"

    local numInfoEntries = ReadLong fin
    for i = 1 to numInfoEntries do
    (
      local str = ReadString fin
      if (chk_Debug.checked) do
        format "debug: info [%]\n" str
    )
  )


  fn ReadPRS fin =
  (
    while true do
    (
      mark = ReadLong fin
           if (mark == ANIM) then (ReadANIM fin)
      else if (mark == NOTE) then (ReadNOTE fin)
      else if (mark == TRCR) then (ReadTRCR fin)
      else if (mark == RKEY) then (ReadRKEY fin)
      else if (mark == KLST) then (ReadKLST fin)
      else if (mark == AEND) then
      (
        ReadAEND fin
        return true
      ) else
        return false
    )

    return false
  )


  fn MakeKeyframes =
  (
    local i, j

    if (chk_Debug.checked) do
      format "debug: applying animation...\n"

    -- gather bone nodes from specified names
    local bone = #()
    for i = 1 to numBones do
      bone[i] = getNodeByName boneNames[i] exact:true

    -- set global animation properties
    frameRate = spn_Fps.value
    animationRange = (interval 0 (spn_Fps.value * animLength))
    print animLength

    -- create/reset root marker
    try
      ResetRootPosMarker()
    catch
      messageBox "error while creating root marker"

    -- apply position keyframes to root marker
	-- this fails if no RKEY section in PRS, so check
    if (chk_Debug.checked) do
      format "debug: Root animation...\n"
	if (dataRKEY == undefined) then
	(
	    if (chk_Debug.checked) do
			format "debug: no root keys...\n"
	)
	else
    (
      -- create a new linear controller
      local c = linear_position()
      $ROOT.position.controller = c

      for j = 1 to dataRKEY.numPosKeys do
      (
        -- make new position key
        local f = dataRKEY.posKeys[j].time * spn_Fps.value * animLength
        local k = addNewKey c f

        -- set position
        k.value = (FlipUp dataRKEY.posKeys[j].pos) * spn_Scale.value * 1000
      )
    )

    -- apply rotation keyframes to root marker
    -- (nope, we don't do this, it breaks things)

    -- apply position keyframes for bones
    for i = 1 to numBones do
    (
      -- create a new linear controller
      local c = linear_position()
      bone[i].position.controller = c

      for j = 1 to dataKLST[i].numPosKeys do
      (
        -- make new position key
        local f = dataKLST[i].posKeys[j].time * spn_Fps.value * animLength
        local k = addNewKey c f

        -- set position
        k.value = (FlipUp dataKLST[i].posKeys[j].pos) * spn_Scale.value * 1000
        if (i == 1) do
          k.value += at time f $ROOT.position
      )
    )

    -- apply rotation keyframes for bones
    for i = 1 to numBones do
    (
      -- create a new linear controller
      local c = linear_rotation()
      bone[i].rotation.controller = c

      -- grips must be pre-rotated by root marker rotation
      local preRot = false
      if (bone[i].name == "weapon_grip" or bone[i].name == "shield_grip") do
        preRot = true

      local lastRot = (quat 0 0 0 1)
      for j = 1 to dataKLST[i].numRotKeys do
      (
        -- make new rotation key
        local f = dataKLST[i].rotKeys[j].time * spn_Fps.value * animLength
        local k = addNewKey c f

        -- compute rotation in controller coordinates
        local thisRot = FlipUpQuat dataKLST[i].rotKeys[j].rot

        -- avoid any unnecessary pirouettes in interpolation
        if ((DotQuat thisRot lastRot) < 0) do
          thisRot *= -1
        lastRot = thisRot

        -- set rotation
        k.value = thisRot
        if (preRot) do
          k.value = thisRot * (at time f $ROOT.rotation)

--        if (i == 1) do
--          k.value = (at time f $ROOT.rotation) * k.value
      )
    )
  )


  fn _AddCriticalEventToNoteTrack timeval fourcc trk =
  (
    critpointtrackname = "CriticalEvents"

    if (numnotetracks $ == 0) then
      for t = 1 to dsglb_CriticalEvents.count do
        addNoteTrack $ (notetrack dsglb_CriticalEventNames[t][1])
    else 
      for t = (numnotetracks $)+1 to dsglb_CriticalEvents.count do
        addNoteTrack $ (notetrack dsglb_CriticalEventNames[t][1])

    nt = getNoteTrack $ trk

    nki =  getnotekeyindex nt timeval
    if (nki == undefined) then
      nk = addNewNoteKey nt.keys timeval
    else
      nk = nt.keys[nki]

    nk.value = fourcc
    nk.selected = true
  )


  fn MakeNotes =
  (
    if (chk_Debug.checked) do
      format "debug: making notes...\n"

    local i
    for i = 1 to numNotes do
    (
      local n = findItem tokenList dataNOTE[i].token
      if (n == 0) then
        messageBox "error: unknown type of note encountered"
      else
        _AddCriticalEventToNoteTrack (dataNOTE[i].time * spn_Fps.value * animLength) dsglb_CriticalEventFourCC[n] n
    )
  )


  fn ImportPRS fin =
  (
    if (chk_Debug.checked) do
      format "debug: importing prs...\n"

    if not (ReadPRS fin) do
      return false

    MakeKeyframes()
    MakeNotes()

    return true
  )


  fn GetOpenFileStream =
  (
    local fname = GetOpenFileName types:"Aspect Animation (*.prs)|*.prs|All Files (*.*)|*.*|"
    if (fname == undefined) do
      return false

    prsFileIn = fopen fname "rb"
    if (prsFileIn == undefined) do
    (
      messageBox "Could not open file for binary reading!"
      return false
    )

    return true
  )


  on btn_ImportNow pressed do
  (
    local oldCoordsys

    oldCoordsys = set coordsys world

    if (GetOpenFileStream()) do
    (
      if (ImportPRS prsFileIn) then
        format "PRS import successful.\n"
      else
        messageBox "PRS import failed."
      fclose prsFileIn
    )

    set coordsys oldCoordsys
  )

)


fn lanceImportPRS =
(
  addRollout PRSImport (newRolloutFloater "PRS Import" 200 185)
)
