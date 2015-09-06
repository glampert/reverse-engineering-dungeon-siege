-- Dungeon Siege SNO Import (Siege Max)
-- Lance ><>
-- 8/20/2002


rollout SNOImport "SNO Import"
(
  local SNOD = 1146048083
  local V7_0 = 7

  struct strHeader  ( mark, version, unk0, numDoors, numSpots, numCorners, numFaces, numTextures, \
                      minBBox, maxBBox, unk1, unk2, unk3, unk4, unk5, unk6, unk7, checkSum )
  struct strSpot    ( xform, name )
  struct strDoor    ( index, xform, numHotSpots, hotSpots )
  struct strCorner  ( pos, nrm, col, txc )
  struct strSurface ( textureName, startCorner, spanCorner, numCorners, faces )

  local snoFileIn
  local dataHeader
  local dataSpots
  local dataDoors
  local dataCorners
  local dataSurfaces

  local myMesh
  local myMat
  local myDoor

  local nnkName

  group "Options"
  (
    spinner spn_Scale "Scale:" range:[0.0001,1000.0,1.0] type:#float scale:0.01
    checkbox chk_Texture "Texture Mesh" checked:true
    checkbox chk_SeeThru "See Thru Mesh" checked:false
    checkbox chk_Debug "Show Debug Info" checked:false
  )
  button btn_ImportNow "Import..." width:100


  fn FlipUp v =
  (
    return ([v.x,-v.z,v.y] as point3)
  )


  fn ReadVector fin =
  (
    local x = ReadFloat fin
    local y = ReadFloat fin
    local z = ReadFloat fin

    return ([x,y,z] as point3)
  )


  fn ReadPosThenRot fin =
  (
    local px = ReadFloat fin
    local py = ReadFloat fin
    local pz = ReadFloat fin
    local r0 = ReadFloat fin
    local r1 = ReadFloat fin
    local r2 = ReadFloat fin
    local r3 = ReadFloat fin
    local r4 = ReadFloat fin
    local r5 = ReadFloat fin
    local r6 = ReadFloat fin
    local r7 = ReadFloat fin
    local r8 = ReadFloat fin

    return (matrix3 [r0,r1,r2] [r3,r4,r5] [r6,r7,r8] [px,py,pz])
  )


  fn ReadRotThenPos fin =
  (
    local r0 = ReadFloat fin
    local r1 = ReadFloat fin
    local r2 = ReadFloat fin
    local r3 = ReadFloat fin
    local r4 = ReadFloat fin
    local r5 = ReadFloat fin
    local r6 = ReadFloat fin
    local r7 = ReadFloat fin
    local r8 = ReadFloat fin
    local px = ReadFloat fin
    local py = ReadFloat fin
    local pz = ReadFloat fin

    return (matrix3 [r0,r1,r2] [r3,r4,r5] [r6,r7,r8] [px,py,pz])
  )


  fn ReadHeader fin =
  (
    local h = strHeader()

    h.mark        = ReadLong fin
    h.version     = ReadLong fin
    h.unk0        = ReadLong fin
    h.numDoors    = ReadLong fin
    h.numSpots    = ReadLong fin
    h.numCorners  = ReadLong fin
    h.numFaces    = ReadLong fin
    h.numTextures = ReadLong fin
    h.minBBox     = ReadVector fin
    h.maxBBox     = ReadVector fin
    h.unk1        = ReadLong fin -- 0
    h.unk2        = ReadFloat fin -- 0
    h.unk3        = ReadLong fin -- 0
    h.unk4        = ReadLong fin -- 26
    h.unk5        = ReadLong fin -- 0
    h.unk6        = ReadLong fin -- 0
    h.unk7        = ReadLong fin -- 0
    h.checkSum    = ReadLong fin #unsigned

    if (h.mark != SNOD) do
    (
      messageBox "file is not a valid siege node"
      return undefined
    )
    if (h.version < V7_0) do
    (
      messageBox "file is not a version 7 siege node"
      return undefined
    )

    return h
  )


  fn ReadDoor fin =
  (
    local i

    local d = ReadLong fin
    local m = ReadPosThenRot fin
    local n = ReadLong fin
    local h = #()
    for i = 1 to n do
      append h (ReadLong fin)

    return (strDoor index:d xform:m numHotSpots:n hotSpots:h)
  )


  fn ReadSpot fin =
  (
    local m = ReadRotThenPos fin
    local n = ReadString fin

    return (strSpot xform:m name:n)
  )


  fn ReadCorner fin =
  (
    local p = ReadVector fin
    local n = ReadVector fin
    local r = ReadByte fin #unsigned
    local b = ReadByte fin #unsigned
    local g = ReadByte fin #unsigned
    local a = ReadByte fin #unsigned
    local u = ReadFloat fin
    local v = ReadFloat fin

    return (strCorner pos:p nrm:n col:#(r,g,b,a) txc:([u,v,0] as point3))
  )


  fn ReadSurface fin =
  (
    local s = strSurface()
    local i

    s.textureName = ReadString fin
    s.startCorner = ReadLong fin
    s.spanCorner  = ReadLong fin
    s.numCorners  = ReadLong fin
    s.faces       = #()
    for i = 1 to (s.numCorners / 3) do
    (
      local a = ReadShort fin
      local b = ReadShort fin
      local c = ReadShort fin
      append s.faces ([a,b,c] as point3)
    )

    return s
  )


  fn ReadSNO fin =
  (
    local i

    if (chk_Debug.checked) do
      format "debug: reading sno...\n"

    -- read header
    dataHeader = ReadHeader fin
    if (dataHeader == undefined) do
      return false

    -- read spots
    dataSpots = #()
    for i = 1 to dataHeader.numSpots do
      append dataSpots (ReadSpot fin)

    -- read doors
    dataDoors = #()
    for i = 1 to dataHeader.numDoors do
      append dataDoors (ReadDoor fin)

    -- read corners
    dataCorners = #()
    for i = 1 to dataHeader.numCorners do
      append dataCorners (ReadCorner fin)

    -- read surfaces
    dataSurfaces = #()
    for i = 1 to dataHeader.numTextures do
      append dataSurfaces (ReadSurface fin)

    return true
  )


  fn MakeDoors =
  (
    local i, j

    if (chk_Debug.checked) do
      format "debug: creating doors...\n"

    myDoor = #()
    for i = 1 to dataHeader.numDoors do
    (
      local doorVert = #()
      local doorTexc = #()
      for j = 1 to dataDoors[i].numHotSpots do
      (
        doorVert[j] = FlipUp dataCorners[dataDoors[i].hotSpots[j]].pos
        doorTexc[j] = dataCorners[dataDoors[i].hotSpots[j]].txc
      )

      local doorFace = #()
      -- create face from data arrays
      myDoor[i] = Mesh \
                  name:     ("Door " + (dataDoors[i].index as string)) \
                  vertices: doorVert \
                  faces:    doorFace \
                  tverts:   doorTexc

      -- set normals for vertices
      for j = 1 to dataDoors[i].numHotSpots do
        setNormal myDoor[i] j (FlipUp dataCorners[dataDoors[i].hotSpots[j]].nrm)
    )
  )


  fn MakeMesh =
  (
    local i, j

    if (chk_Debug.checked) do
      format "debug: creating mesh...\n"

    -- create array of vertices
    local meshVert = #()
    for i = 1 to dataHeader.numCorners do
      meshVert[i] = (FlipUp dataCorners[i].pos) * spn_Scale.value * 1000

    -- create array of texture coordinates
    local meshTexc = #()
    for i = 1 to dataHeader.numCorners do
      meshTexc[i] = dataCorners[i].txc

    -- create array of faces
    local meshFace = #()
    local f = 1
    for i = 1 to dataHeader.numTextures do
    (
      for j = 1 to (dataSurfaces[i].numCorners / 3) do
      (
        local cX = dataSurfaces[i].faces[j].x + dataSurfaces[i].startCorner + 1
        local cY = dataSurfaces[i].faces[j].y + dataSurfaces[i].startCorner + 1
        local cZ = dataSurfaces[i].faces[j].z + dataSurfaces[i].startCorner + 1
        meshFace[f] = [cX, cY, cZ] as point3
        f += 1
      )
    )

    meshMids = #()
    f = 1
    for i = 1 to dataHeader.numTextures do
    (
      for j = 1 to (dataSurfaces[i].numCorners / 3) do
      (
        meshMids[f] = i
        f += 1
      )
    )

    -- create face from data arrays
    myMesh = Mesh \
             name:        nnkName \
             vertices:    meshVert \
             faces:       meshFace \
             materialIDs: meshMids \
             tverts:      meshTexc

    -- set normals for vertices
    for i = 1 to dataHeader.numCorners do
      setNormal myMesh i (FlipUp dataCorners[i].nrm)

    -- build texture faces to match regular faces (tvface == face)
    buildTVFaces myMesh
    for i = 1 to dataHeader.numFaces do
      setTVFace myMesh i meshFace[i]

    -- and miscellaneous stuff
    myMesh.wirecolor = (color 0 0 153)
    if (chk_SeeThru.checked) do
      myMesh[sm].xray = true
  )


  fn MakeTexture n =
  (
    local t = StandardMaterial \
              name:           n \
              shaderByName:   "blinn" \
              adTextureLock:  true \
              adLock:         true \
              showInViewport: true
    t.diffuseMapEnable = true
    t.opacityMapEnable = true
    t.diffuseMap = BitmapTexture name:(n + "_diffuse") \
                   filename:(n + ".raw")
    t.opacityMap = BitmapTexture name:(n + "_opacity") \
                   filename:(n + ".raw") \
                   monoOutput: 1 \
                   alphaSource: 0

    return t
  )


  fn MakeMaterial =
  (
    local i, j

    if (chk_Debug.checked) do
      format "debug: creating material...\n"

    -- create standard texture, or multi-material if needed
    if (dataHeader.numTextures > 1) then
    (
      myMat = MultiMaterial numSubs:dataHeader.numTextures
      for i = 1 to dataHeader.numTextures do
      (
        myMat.names[i] = dataSurfaces[i].textureName
        myMat.materialList[i] = MakeTexture dataSurfaces[i].textureName
      )
    )
    else
    (
      myMat = makeTexture dataSurfaces[1].textureName
    )

    -- add material to first slot in material editor
    setMeditMaterial 1 myMat

    -- assign material to mesh
    myMesh.material = myMat
  )


  fn ImportSNO fin =
  (
    if (chk_Debug.checked) do
      format "debug: importing sno...\n"

    if not (ReadSNO fin) do
      return false

    MakeMesh()
--    MakeDoors()

    if (chk_Texture.checked and dataHeader.numTextures > 0) do
      MakeMaterial()

    -- add siegemax specific things
    select myMesh
    addModifier myMesh (SNOModDef())

    return true
  )


  fn GetOpenFileStream =
  (
    local fname = GetOpenFileName types:"Siege Node Terrain (*.sno)|*.sno|All Files (*.*)|*.*|"
    if (fname == undefined) do
      return false

    -- figure out the node name from the filename
    local tokens = filterString fname "/\\"
    nnkName = (filterString tokens[tokens.count] ".")[1]

    snoFileIn = fopen fname "rb"
    if (snoFileIn == undefined) do
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
      if (ImportSNO snoFileIn) then
        format "SNO import successful.\n"
      else
        messageBox "SNO import failed."
      fclose snoFileIn
    )

    set coordsys oldCoordsys
  )

)


fn lanceImportSNO =
(
  addRollout SNOImport (newRolloutFloater "SNO Import" 200 205)
)
