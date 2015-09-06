-- Dungeon Siege ASP Import (Siege Max)
-- Lance ><>
-- 8/23/2002
-- updates Jan 2004 by Ghastley (DS LoA etc)


rollout ASPImport "ASP Import"
(
  local BMSH = 1213418818
  local BONH = 1213091650
  local BSUB = 1112888130
  local BSMM = 1296913218
  local BVTX = 1481922114
  local BCRN = 1314014018
  local WCRN = 1314014039
  local BVMP = 1347245634
  local BTRI = 1230132290
  local BVWL = 1280792130
  local STCH = 1212372051
  local RPOS = 1397706834
  local BBOX = 1481589314
  local BEND = 1145980226
  local INFO = 1330007625
  local V1_2 = 513
  local V1_3 = 769   -- introduced WCRN section
  local V2_0 = 2     -- introduced STCH section
  local V2_1 = 258   -- introduced BBOX section
  local V2_2 = 514
  local V2_3 = 770
  local V2_4 = 1026
  local V2_5 = 1282
  local V4_0 = 4     -- introduced with Siege Max
  local V4_1 = 260
  local v5_0 = 5     -- DS LoA
  local NECK = 1262699854
  local WRST = 1414746711
  local ANKL = 1280003649


  struct strBoneInfo ( name, parent, flags )
  struct strSubMesh ( numSubTextures, numVertices, numCorners, numFaces, numStitches, \
                      dataBSMM, dataBVTX, dataBCRN, dataWCRN, dataBVMP, dataBTRI, dataBVWL, dataSTCH )
  struct strBSMM ( textureIndex, faceSpan )
  struct strBCRN ( pos, nrm, col, txc )
  struct strWCRN ( pos, weight, bone, nrm, col, txc )
  struct strBVMP ( count, cornerIndex )
  struct strBTRI ( cornerStart, cornerSpan, cornerIndex )
  struct strBVWL ( weight, bone )
  struct strSTCH ( token, vertexCount, vertexIndex )
  struct strRPOS ( rot, pos )


  local aspFileIn
  local nnkName
  local numBones
  local numTextures
  local numVerticesTotal
  local numSubMeshes
  local renderFlags
  local curSM

  local textureName
  local boneInfo
  local subMesh
  local dataRPOS

  local myMesh
  local myMat
  local myBone
  local mySkin


  group "Options"
  (
    spinner spn_Scale "Scale:" range:[0.0001,1000.0,1.0] type:#float scale:0.01
    checkbox chk_Texture "Texture Mesh" checked:true
    checkbox chk_Bone "Bone Mesh" checked:true
    checkbox chk_Skin "Skin Mesh" checked:true
    checkbox chk_BoneBall "Bones As Balls" checked:false
    checkbox chk_SeeThru "See Thru Mesh" checked:false
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


  fn VersionOf v =
  (
    case of
    (
      (v == V1_2): return 12
      (v == V1_3): return 13
      (v == V2_0): return 20
      (v == V2_1): return 21
      (v == V2_2): return 22
      (v == V2_3): return 23
      (v == V2_4): return 24
      (v == V2_5): return 25
      (v == V4_0): return 40
      (v == V4_1): return 41
      (v == V5_0): return 50
      default:     return 0
    )
  )


  fn ReadQuat fin =
  (
    local x = ReadFloat fin
    local y = ReadFloat fin
    local z = ReadFloat fin
    local w = ReadFloat fin

    return (quat x y z w)
  )


  fn ReadVector fin =
  (
    local x = ReadFloat fin
    local y = ReadFloat fin
    local z = ReadFloat fin

    return ([x,y,z] as point3)
  )


  fn ReadCoord fin =
  (
    local u = ReadFloat fin
    local v = ReadFloat fin

    return ([u,v,0] as point3)
  )


  fn ReadFourBB fin =
  (
    local a = ReadByte fin #unsigned
    local b = ReadByte fin #unsigned
    local c = ReadByte fin #unsigned
    local d = ReadByte fin #unsigned

    return #(a,b,c,d)
  )

   fn ReadFourBB2 fin =
  (
    local a = ReadByte fin +1 -- #unsigned
    local b = ReadByte fin +1 -- #unsigned
    local c = ReadByte fin +1 -- #unsigned
    local d = ReadByte fin +1 -- #unsigned

    return #(a,b,c,d)
  )


  fn ReadFourFF fin =
  (
    local a = ReadFloat fin
    local b = ReadFloat fin
    local c = ReadFloat fin
    local d = ReadFloat fin

    return #(a,b,c,d)
  )


  fn ReadBMSH fin =
  (
    if (chk_Debug.checked) do
      format "debug: reading BMSH section...\n"
    local version = ReadLong fin

    local sizeTextField = ReadLong fin
    numBones = ReadLong fin
    numTextures = ReadLong fin
    numVerticesTotal = ReadLong fin
    numSubMeshes = ReadLong fin
    renderFlags = ReadLong fin

    -- read text field
    local token = #()
    while (sizeTextField > 0) do
    (
      append token (ReadString fin)
      local size = token[token.count].count + 1
      local padding = mod (4 - (mod size 4)) 4
      for i = 1 to padding do
        ReadByte fin -- throw away null padding
      sizeTextField -= size + padding
    )

    -- split tokens in textures names and bone names
    textureName = #()
    if (numTextures > 0) do
      for i = 1 to numTextures do
        textureName[i] = token[i]
    boneInfo = #()
    if (numBones > 0) do
      for i = 1 to numBones do
        boneInfo[i] = strBoneInfo name:token[i + numTextures]

    subMesh = #()
  )


  fn ReadBONH fin =
  (
    if (chk_Debug.checked) do
      format "debug: reading BONH section...\n"
    local version = ReadLong fin

    for i = 1 to numBones do
    (
      local j = ReadLong fin
      local p = ReadLong fin
      local f = ReadLong fin
      boneInfo[j + 1].parent = p + 1
      boneInfo[j + 1].flags = f
    )
  )


  fn ReadBSUB fin =
  (
    if (chk_Debug.checked) do
      format "debug: reading BSUB section...\n"
    local version = ReadLong fin

    curSM = (ReadLong fin)
	if ((VersionOf version) > 40) then
      curSM += 1 -- 0 based index -> 1 based index
    else
      curSM += 2 -- -1 based index -> 1 based index
    subMesh[curSM] = strSubMesh()

    if (numTextures != ReadLong fin) do
      if (chk_Debug.checked) do
        messageBox "error: BSUB numTextures don't match"

    subMesh[curSM].numVertices = ReadLong fin
    subMesh[curSM].numCorners = ReadLong fin
    subMesh[curSM].numFaces = ReadLong fin
  )


  fn ReadBSMM fin =
  (
    if (chk_Debug.checked) do
      format "debug: reading BSMM section...\n"
    local version = ReadLong fin

    subMesh[curSM].numSubTextures = ReadLong fin

    subMesh[curSM].dataBSMM = #()
    for i = 1 to subMesh[curSM].numSubTextures do
    (
      subMesh[curSM].dataBSMM[i] = strBSMM()
      subMesh[curSM].dataBSMM[i].textureIndex = (ReadLong fin) + 1
      subMesh[curSM].dataBSMM[i].faceSpan = ReadLong fin
    )
  )


  fn ReadBVTX fin =
  (
    if (chk_Debug.checked) do
      format "debug: reading BVTX section...\n"
    local version = ReadLong fin

    if (subMesh[curSM].numVertices != ReadLong fin) do
      if (chk_Debug.checked) do
        messageBox "error: BVTX numVertices don't match"

    subMesh[curSM].dataBVTX = #()
    for i = 1 to subMesh[curSM].numVertices do
      subMesh[curSM].dataBVTX[i] = ReadVector fin
  )


  fn ReadBCRN fin =
  (
    if (chk_Debug.checked) do
      format "debug: reading BCRN section...\n"
    local version = ReadLong fin

    if (subMesh[curSM].numCorners != ReadLong fin) do
      if (chk_Debug.checked) do
        messageBox "error: BCRN numCorners don't match"

    subMesh[curSM].dataBCRN = #()
    for i = 1 to subMesh[curSM].numCorners do
    (
      local bvtxIndex = ReadLong fin
      local p = subMesh[curSM].dataBVTX[bvtxIndex + 1]
      local n = ReadVector fin
      local c = ReadFourBB fin
      local unk0 = ReadLong fin
      local t = ReadCoord fin
      subMesh[curSM].dataBCRN[i] = strBCRN pos:p nrm:n col:c txc:t
    )
  )


  fn ReadWCRN fin =
  (
    if (chk_Debug.checked) do
      format "debug: reading WCRN section...\n"
    local version = ReadLong fin

    if (subMesh[curSM].numCorners != ReadLong fin) do
      if (chk_Debug.checked) do
        messageBox "error: WCRN numCorners don't match"

    subMesh[curSM].dataWCRN = #()
    for i = 1 to subMesh[curSM].numCorners do
    (
      local b = #()
      local p = ReadVector fin
      local w = ReadFourFF fin
      if ((VersionOf version) > 40) then
        b = ReadFourBB2 fin
      else 
        b = ReadFourBB fin
      local n = ReadVector fin
      local c = ReadFourBB fin
      local t = ReadCoord fin
      -- remove null bone/weights
      for i = 4 to 1 by -1 do
        if (w[i] == 0) do
        (
          deleteItem w i
          deleteItem b i
        )
      subMesh[curSM].dataWCRN[i] = strWCRN pos:p weight:w bone:b nrm:n col:c txc:t
    )
  )


  fn ReadBVMP fin =
  (

    if (chk_Debug.checked) do
      format "debug: reading BVMP section...\n"
    local version = ReadLong fin

    subMesh[curSM].dataBVMP = #()
    local i, j
    for i = 1 to subMesh[curSM].numVertices do
    (
      subMesh[curSM].dataBVMP[i] = strBVMP()
      subMesh[curSM].dataBVMP[i].count = ReadLong fin
      subMesh[curSM].dataBVMP[i].cornerIndex = #()
      for j = 1 to subMesh[curSM].dataBVMP[i].count do
        subMesh[curSM].dataBVMP[i].cornerIndex[j] = (ReadLong fin) + 1
    )
  )


  fn ReadBTRI fin =
  (
    if (chk_Debug.checked) do
      format "debug: reading BTRI section...\n"
    local version = ReadLong fin

    if (subMesh[curSM].numFaces != ReadLong fin) do
      if (chk_Debug.checked) do
        messageBox "error: BTRI numFaces don't match"

    subMesh[curSM].dataBTRI = strBTRI()

    -- read header
    subMesh[curSM].dataBTRI.cornerStart = #()
    subMesh[curSM].dataBTRI.cornerSpan = #()
    if ((VersionOf version) == 22) then
    (
      for i = 1 to subMesh[curSM].numSubTextures do
        subMesh[curSM].dataBTRI.cornerSpan[i] = ReadLong fin
      subMesh[curSM].dataBTRI.cornerStart[1] = 1
      for i = 1 to subMesh[curSM].numSubTextures - 1 do
        subMesh[curSM].dataBTRI.cornerStart[i + 1] = \
            subMesh[curSM].dataBTRI.cornerStart[i] + \
            subMesh[curSM].dataBTRI.cornerSpan[i]
    )
    else if ((VersionOf version) > 22) then
    (
      for i = 1 to subMesh[curSM].numSubTextures do
      (
        subMesh[curSM].dataBTRI.cornerStart[i] = (ReadLong fin) + 1
        subMesh[curSM].dataBTRI.cornerSpan[i] = ReadLong fin
      )
    )
    else
    (
      for i = 1 to subMesh[curSM].numSubTextures do
      (
        subMesh[curSM].dataBTRI.cornerStart[i] = 1
        subMesh[curSM].dataBTRI.cornerSpan[i] = subMesh[curSM].numCorners
      )
    )

    -- read data
    subMesh[curSM].dataBTRI.cornerIndex = #()
    for i = 1 to subMesh[curSM].numFaces do
    (
      local a = (ReadLong fin) as float + 1
      local b = (ReadLong fin) as float + 1
      local c = (ReadLong fin) as float + 1
      subMesh[curSM].dataBTRI.cornerIndex[i] = [a,b,c] as point3
    )
  )


  fn ReadBVWL fin =
  (
    if (chk_Debug.checked) do
      format "debug: reading BVWL section...\n"
    local version = ReadLong fin

    local i, j
    local boneCornID = #()
    local boneCornWt = #()
    for i = 1 to numBones do
    (
      boneCornID[i] = #()
      boneCornWt[i] = #()
    )

    for i = 1 to numBones do
    (
      local count = ReadLong fin
      for j = 1 to count do
      (
        boneCornID[i][j] = (ReadLong fin) + 1
        boneCornWt[i][j] = ReadFloat fin
      )
    )

    -- convert bone->vert/weight mapping to vert->bone/weight mapping
--    subMesh[curSM].dataBVWL = #()
--    for i = 1 to subMesh[curSM].numCorners do
--      subMesh[curSM].dataBVWL[i] = strBVWL weight:#() bone:#()

--    for i = 1 to numBones do
--      if (boneCornID[i].count > 0) do
--        for j = 1 to boneCornID[i].count do
--        (
--          local vert = boneCornID[i][j]
--          local size = subMesh[curSM].dataBVWL[vert].bone.count + 1
--          subMesh[curSM].dataBVWL[vert].bone[size] = i
--          subMesh[curSM].dataBVWL[vert].weight[size] = boneCornWt[i][j]
--        )
  )


  fn ReadSTCH fin =
  (
    if (chk_Debug.checked) do
      format "debug: reading STCH section...\n"
    local version = ReadLong fin

    subMesh[curSM].numStitches = ReadLong fin

    subMesh[curSM].dataSTCH = #()
    local i, j
    for i = 1 to subMesh[curSM].numStitches do
    (
      subMesh[curSM].dataSTCH[i] = strSTCH()
      subMesh[curSM].dataSTCH[i].token = ReadLong fin
      subMesh[curSM].dataSTCH[i].vertexCount = ReadLong fin
      subMesh[curSM].dataSTCH[i].vertexIndex = #()
      for j = 1 to subMesh[curSM].dataSTCH[i].vertexCount do
        subMesh[curSM].dataSTCH[i].vertexIndex[j] = (ReadLong fin) + 1
    )
  )


  fn ReadRPOS fin =
  (
    if (chk_Debug.checked) do
      format "debug: reading RPOS section...\n"
    local version = ReadLong fin

    if (numBones != ReadLong fin) do
      if (chk_Debug.checked) do
        messageBox "error: RPOS numBones don't match"

    dataRPOS = #()
    dataRPOS[1] = #()
    dataRPOS[2] = #()
    local i, j
    for i = 1 to numBones do
      for j = 1 to 2 do
      (
        local q = ReadQuat fin
        local v = ReadVector fin
        dataRPOS[j][i] = strRPOS rot:q pos:v
      )
  )


  fn ReadBBOX fin =
  (
    if (chk_Debug.checked) do
      format "debug: reading BBOX section...\n"
    local version = ReadLong fin

    if (0 != ReadLong fin) do
      if (chk_Debug.checked) do
        messageBox "error: BBOX expected 0"
  )


  fn ReadBEND fin =
  (
    if (chk_Debug.checked) do
      format "debug: reading BEND section...\n"

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


  fn ReadASP fin =
  (
    while true do
    (
      mark = ReadLong fin
           if (mark == BMSH) then (ReadBMSH fin)
      else if (mark == BONH) then (ReadBONH fin)
      else if (mark == BSUB) then (ReadBSUB fin)
      else if (mark == BSMM) then (ReadBSMM fin)
      else if (mark == BVTX) then (ReadBVTX fin)
      else if (mark == BCRN) then (ReadBCRN fin)
      else if (mark == WCRN) then (ReadWCRN fin)
      else if (mark == BVMP) then (ReadBVMP fin)
      else if (mark == BTRI) then (ReadBTRI fin)
      else if (mark == BVWL) then (ReadBVWL fin)
      else if (mark == STCH) then (ReadSTCH fin)
      else if (mark == RPOS) then (ReadRPOS fin)
      else if (mark == BBOX) then (ReadBBOX fin)
      else if (mark == BEND) then
      (
        ReadBEND fin
        return true
      ) else
        return false
    )

    return false
  )


  fn MakeMesh =
  (
    local sm, i, j

    if (chk_Debug.checked) do
      format "debug: creating mesh...\n"

    -- gather vertices
    local meshVert = #()
    for sm = 1 to numSubMeshes do
      for i = 1 to subMesh[sm].numCorners do
        append meshVert ((FlipUp subMesh[sm].dataWCRN[i].pos) * 1000 * spn_Scale.value)

    -- gather texture coordinates
    local meshTexc = #()
    for sm = 1 to numSubMeshes do
      for i = 1 to subMesh[sm].numCorners do
        append meshTexc subMesh[sm].dataWCRN[i].txc

    -- gather faces
    local meshFace = #()
    local cornOffset = 0
    for sm = 1 to numSubMeshes do
    (
      local f = 1
      for i = 1 to subMesh[sm].numSubTextures do
      (
        for j = 1 to subMesh[sm].dataBSMM[i].faceSpan do
        (
          local a = subMesh[sm].dataBTRI.cornerIndex[f].x + subMesh[sm].dataBTRI.cornerStart[i] + cornOffset - 1
          local b = subMesh[sm].dataBTRI.cornerIndex[f].y + subMesh[sm].dataBTRI.cornerStart[i] + cornOffset - 1
          local c = subMesh[sm].dataBTRI.cornerIndex[f].z + subMesh[sm].dataBTRI.cornerStart[i] + cornOffset - 1
          append meshFace ([a,b,c] as point3)
          f += 1
        )
      )
      cornOffset += subMesh[sm].numCorners
    )

    -- gather material ids
    local meshMids = #()
    for sm = 1 to numSubMeshes do
      for i = 1 to subMesh[sm].numSubTextures do
        for j = 1 to subMesh[sm].dataBSMM[i].faceSpan do
          append meshMids subMesh[sm].dataBSMM[i].textureIndex

    -- create mesh
    myMesh = Mesh \
             name:        nnkName  \
             vertices:    meshVert \
             faces:       meshFace \
             materialIDs: meshMids \
             tverts:      meshTexc

    -- set normals
    cornOffset = 0
    for sm = 1 to numSubMeshes do
    (
      for i = 1 to subMesh[sm].numCorners do
        setNormal myMesh (i + cornOffset) (FlipUp subMesh[sm].dataWCRN[i].nrm)
      cornOffset += subMesh[sm].numCorners
    )

    -- set texture faces
    buildTVFaces myMesh
    for i = 1 to meshFace.count do

      setTVFace myMesh i meshFace[i] -- tvfaces == faces

    if (chk_SeeThru.checked) do
      myMesh.xray = true
  )


  fn MakeTexture n =
  (
    t = StandardMaterial \
        name:           n \
        shaderByName:   "blinn" \
        adTextureLock:  true \
        adLock:         true \
        showInViewport: true
    t.diffuseMapEnable = true
--    t.opacityMapEnable = true
    t.diffuseMap = BitmapTexture name:(n + "_diffuse") \
                   filename:(n + ".psd")
--    t.opacityMap = BitmapTexture name:(n + "_opacity") \
--                   filename:(n + ".psd") \
--                   monoOutput: 1 \
--                   alphaSource: 0

    return t
  )


  fn MakeMaterial =
  (
    if (chk_Debug.checked) do
      format "debug: creating textures...\n"

    -- create standard texture, or multi-material if needed
    if (numTextures > 1) then
    (
      myMat = MultiMaterial numSubs:numTextures
      for i = 1 to numTextures do
      (
        myMat.names[i] = textureName[i]
        myMat.materialList[i] = MakeTexture textureName[i]
      )
    )
    else
    (
      myMat = MakeTexture textureName[1]
    )

    -- assign material to mesh
    setMeditMaterial 1 myMat
    myMesh.material = myMat
  )


  fn QuatApply v q =
  (
    m = q as Matrix3
    x = dot m.row1 v
    y = dot m.row2 v
    z = dot m.row3 v

    return ([x, y, z] as Point3)
  )


  fn MakeBones =
  (
    local i, j

    if (chk_Debug.checked) do
      format "debug: creating bones...\n"

    -- build bones
    myBone = #()
    for i = 1 to numBones do
    (
      local parent = boneInfo[i].parent

      -- create bone
      if (parent == i or boneInfo[i].name == "leftfoot_pin" or boneInfo[i].name == "rightfoot_pin") then
      (
        myBone[i] = Sphere radius:(35 * spn_Scale.value) segs:16 \
                    name:boneInfo[i].name pos:[0,0,0] smooth:true
      )
      else if (boneInfo[i].name == "weapon_grip") then
      (
        myBone[i] = GripMarker name:boneInfo[i].name gripType:1 pos:[0,0,0]
      )
      else if (boneInfo[i].name == "shield_grip") then
      (
        myBone[i] = GripMarker name:boneInfo[i].name gripType:2 pos:[0,0,0]
      )
      else
      (
        if (chk_BoneBall.checked) then
        (
          myBone[i] = Sphere radius:(35 * spn_Scale.value) segs:16 \
                      name:boneInfo[i].name pos:[0,0,0] smooth:true
        )
        else
        (
          myBone[i] = BoneSys.createBone [0,0,0] [1,0,0] [0,0,1]
          myBone[i].name   = boneInfo[i].name
          myBone[i].width  = 40 * spn_Scale.value
          myBone[i].height = 40 * spn_Scale.value
        )
      )

      -- link to parent
      if (parent == i or parent == 0) then
        myBone[i].parent = undefined
      else
        myBone[i].parent = myBone[parent]

      -- position and orientate relative to parent
      in coordsys parent myBone[i].rotation = FlipUpQuat dataRPOS[2][i].rot
      in coordsys parent myBone[i].position = (FlipUp dataRPOS[2][i].pos) * spn_Scale.value * 1000

      -- grips must be prerotated (hack fix)
      if (myBone[i].name == "weapon_grip" or myBone[i].name == "shield_grip") do
        in coordsys parent myBone[i].rotation = (FlipUpQuat dataRPOS[2][i].rot) * ((angleAxis 90 [1,0,0]) as quat)

      -- set miscellaneous properties
      if (myBone[i].name != "weapon_grip" and myBone[i].name != "shield_grip" and
          myBone[i].name != "leftfoot_pin" and myBone[i].name != "rightfoot_pin") do
      (
        myBone[i].setBoneEnable on 0
        myBone[i].boneAutoAlign = true
        myBone[i].boneFreezeLength = true
        myBone[i].boneScaleType = #none
      )
      myBone[i].wireColor = (color 0 0 255)
    )

    -- set bone lengths
    for b in myBone do
    (
      if (classOf b == BoneGeometry) do
      (
        -- bone length is guessed to be the average of the distances to its children
        -- if it's a terminating bone, just guess the length to be 100
        if (b.children.count > 0) then
        (
          local distSum = 0
          for b2 in b.children do
            distSum += distance b.pos b2.pos
          b.length = distSum / b.children.count
        )
        else
        (
          b.length = 100
        )
      )
    )
  )


  fn MakeSkin =
  (
    local i

    if (chk_Debug.checked) do
      format "debug: creating skin...\n"

    max modify mode
    select myMesh
    subObjectLevel = 0

    -- create skin, and add bones
    mySkin = Skin()
    addModifier myMesh mySkin
    for i = 1 to numBones do
      skinOps.addBone mySkin myBone[i] 0

    -- reset all vertex weights
    subObjectLevel = 1
    allVerts = #()
    for i = 1 to myMesh.numVerts do
      append allVerts i
    skinOps.SelectVertices mySkin allVerts
    skinOps.resetSelectedVerts mySkin

    -- set vertex weights
    local cornOffset = 0
    with redraw off
      for sm = 1 to numSubMeshes do
      (
        for i = 1 to subMesh[sm].numCorners do
		(
		  if (subMesh[sm].dataWCRN[i].bone != #()) then	
          	skinOps.ReplaceVertexWeights mySkin (i + cornOffset) subMesh[sm].dataWCRN[i].bone subMesh[sm].dataWCRN[i].weight
		)
        cornOffset += subMesh[sm].numCorners
      )

    subObjectLevel = 0
    deselect $
    max utility mode
  )


  fn ImportASP fin =
  (
    if not (ReadASP fin) do
      return false

    MakeMesh()

    if (chk_Texture.checked and numTextures > 0) do
      MakeMaterial()

    if (chk_Bone.checked and numBones > 0) do
    (
      makeBones()
      if (chk_Skin.checked) do
        makeSkin()
    )

    -- add siegemax specific things
    select myMesh
    addModifier myMesh (ASPModDef())

    return true
  )


  fn GetOpenFileStream =
  (
    local fname = GetOpenFileName types:"Aspect Mesh (*.asp)|*.asp|All Files (*.*)|*.*|"
    if (fname == undefined) do
      return false

    -- figure out the node name from the filename
    local tokens = filterString fname "/\\"
    nnkName = (filterString tokens[tokens.count] ".")[1]

    aspFileIn = fopen fname "rb"
    if (aspFileIn == undefined) do
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
      if (ImportASP aspFileIn) then
        format "ASP import successful.\n"
      else
        messageBox "ASP import failed."
      fclose aspFileIn
    )

    set coordsys oldCoordsys
  )

)


fn lanceImportASP =
(
  addRollout ASPImport (newRolloutFloater "ASP Import" 200 265)
)
