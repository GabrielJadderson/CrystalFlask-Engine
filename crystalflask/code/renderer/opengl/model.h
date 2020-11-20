#ifndef MODEL_H
#define MODEL_H

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>


static const uint32_t s_MeshImportFlags =
aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
aiProcess_Triangulate |             // Make sure we're triangles
aiProcess_SortByPType |             // Split meshes by primitive type
//aiProcess_PreTransformVertices |
aiProcess_GenNormals |              // Make sure we have legit normals
aiProcess_GenUVCoords |             // Convert UVs if required
aiProcess_OptimizeMeshes |          // Batch draws where possible
//aiProcess_Debone |                  // This step removes bones losslessly or according to some threshold.
aiProcess_ValidateDataStructure;    // Validation

static glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& from)
{
    glm::mat4 to;
    //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

static Assimp::Importer* Importer = NULL;
static aiScene* AIScene;

internal mesh_data*
ExtractMeshData(char *Filename)
{
    
    mesh_data *Result = (mesh_data*)PushStruct(&GlobalResourceArena, mesh_data);
    
    if (!Importer)
        Importer = new Assimp::Importer();
    
    const aiScene* scene = Importer->ReadFile(Filename, s_MeshImportFlags);
    if (!scene || !scene->HasMeshes())
    {
        CrystalFlaskConsole.AddLog("[ERROR] ReadMesh failed to read file: %s", Filename);
        //failed to read mesh
        return NULL;
    } else
    {
        Result->IsInitialized = true;
    }
    
    
    Result->InverseTransform = glm::inverse(aiMatrix4x4ToGlm(scene->mRootNode->mTransformation));
    
    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;
    
    Result->SubmeshCount = scene->mNumMeshes;
    Result->Submesh = (Submesh*)PushArray(&GlobalResourceArena, Result->SubmeshCount, Submesh);
    
    //calculate the total number of vertices and indices
    for (u64 MeshIndex = 0; MeshIndex < scene->mNumMeshes; MeshIndex++)
    {
        aiMesh* mesh = scene->mMeshes[MeshIndex];
        Result->VertexCount += mesh->mNumVertices;
        Result->IndexCount += mesh->mNumFaces;
    }
    
    Result->Vertices = (Vertex*)PushArray(&GlobalResourceArena, Result->VertexCount, Vertex);
    
    Result->Indices = (Index*)PushArray(&GlobalResourceArena, Result->IndexCount, Index);
    
    for (u64 MeshIndex = 0; MeshIndex < scene->mNumMeshes; MeshIndex++)
    {
        
        aiMesh* mesh = scene->mMeshes[MeshIndex];
        
        Assert(mesh->HasPositions());
        Assert(mesh->HasNormals());
        
        Submesh submesh;
        submesh.BaseVertex = vertexCount;
        submesh.BaseIndex = indexCount;
        submesh.MaterialIndex = mesh->mMaterialIndex;
        submesh.IndexCount = mesh->mNumFaces * 3;
        
        Result->Submesh[MeshIndex] = submesh;
        
        vertexCount += mesh->mNumVertices;
        indexCount += submesh.IndexCount;
        
        // Vertices
        for (u64 VertexIndex = 0; VertexIndex < mesh->mNumVertices; VertexIndex++)
        {
            Vertex vertex;
            vertex.Position = { mesh->mVertices[VertexIndex].x, mesh->mVertices[VertexIndex].y, mesh->mVertices[VertexIndex].z };
            
            if (mesh->HasNormals())
            {
                vertex.Normal = { mesh->mNormals[VertexIndex].x, mesh->mNormals[VertexIndex].y, mesh->mNormals[VertexIndex].z };
            }
            else
            {
                char ErrorBuffer[128];
                sprintf_s(ErrorBuffer, 128, "Mesh %s has no Normals.", Filename);
                Win32MessageBoxError(ErrorBuffer);
            }
            
            
            if (mesh->HasTangentsAndBitangents())
            {
                vertex.Tangent = { mesh->mTangents[VertexIndex].x, mesh->mTangents[VertexIndex].y, mesh->mTangents[VertexIndex].z };
                
                vertex.Binormal = { mesh->mBitangents[VertexIndex].x, mesh->mBitangents[VertexIndex].y, mesh->mBitangents[VertexIndex].z };
            }
            else
            {
                char ErrorBuffer[128];
                sprintf_s(ErrorBuffer, 128, "Mesh %s has no Tangents And Bitangents.", Filename);
                Win32MessageBoxError(ErrorBuffer);
            }
            
            if (mesh->HasTextureCoords((u32)MeshIndex))
            {
                vertex.Texcoord = { mesh->mTextureCoords[MeshIndex][VertexIndex].x, mesh->mTextureCoords[MeshIndex][VertexIndex].y };
            }
            else
            {
                char ErrorBuffer[128];
                sprintf_s(ErrorBuffer, 128, "Mesh %s has no TextureCoords.", Filename);
                Win32MessageBoxError(ErrorBuffer);
            }
            
            Result->Vertices[VertexIndex] = vertex;
        }
        
        if (!mesh->HasTangentsAndBitangents())
        {
            //computeTangentBasis(Result->Vertices, Result->VertexCount);
        }
        
        // Indices
        for (u64 IndexIndex = 0; IndexIndex < mesh->mNumFaces; IndexIndex++)
        {
            Result->Indices[IndexIndex] = {
                mesh->mFaces[IndexIndex].mIndices[0], mesh->mFaces[IndexIndex].mIndices[1], mesh->mFaces[IndexIndex].mIndices[2] };
        }
        
    }
    
    
    return Result;
}

internal mesh_data*
ReadMeshAndUpload(char *Filename)
{
    
    mesh_data* Result = ExtractMeshData(Filename);
    if (Result == NULL)
    {
        return NULL;
    }
    
    u32 VertexBuffer = 0;
    glGenBuffers(1, &VertexBuffer);
    
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, Result->VertexCount * sizeof(Vertex), Result->Vertices, GL_STATIC_DRAW);
    
    
    u32 IndexBuffer = 0;
    glGenBuffers(1, &IndexBuffer);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Result->IndexCount * sizeof(Index), Result->Indices, GL_STATIC_DRAW);
    
    
    u32 VAO;
    glGenVertexArrays(1, &VAO);
    
    Result->VAO;
    Result->VertexBuffer = VertexBuffer;
    Result->IndexBuffer = IndexBuffer;
    Result->IsUploaded = true;
    
    return Result;
}


internal mesh_data*
UploadMeshToGPU(mesh_data *MeshData)
{
    mesh_data *Result = MeshData;
    
    if (Result->IsUploaded)
    {
        CrystalFlaskConsole.AddLog("[WARNING] UploadMeshToGPU: MeshData already uploaded??? wtf bro?");
        return Result; //nothing to do here
        
    }
    if (!Result->IsInitialized)
    {
        CrystalFlaskConsole.AddLog("[WARNING] UploadMeshToGPU: MeshData Not Initialized. Please read the mesh before uploading it.");
        return Result; //nothing to do here
    }
    
    
    u32 VertexBuffer = 0;
    glGenBuffers(1, &VertexBuffer);
    
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, Result->VertexCount * sizeof(Vertex), Result->Vertices, GL_STATIC_DRAW);
    
    
    u32 IndexBuffer = 0;
    glGenBuffers(1, &IndexBuffer);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Result->IndexCount * sizeof(Index), Result->Indices, GL_STATIC_DRAW);
    
    
    u32 VAO;
    glGenVertexArrays(1, &VAO);
    
    Result->VAO;
    Result->VertexBuffer = VertexBuffer;
    Result->IndexBuffer = IndexBuffer;
    Result->IsUploaded = true;
    
    return Result;
}


internal mesh_data*
ReadMeshSequential(char *FileName, mesh_data *Result)
{
    Assimp::Importer importer;
    
    const aiScene* scene = importer.ReadFile(FileName, s_MeshImportFlags);
    if (!scene || !scene->HasMeshes())
    {
        //TODO(Gabriel): lock before writing!!
        CrystalFlaskConsole.AddLog("[ERROR] ReadMesh failed to read file: %s", FileName);
        return NULL;
    }
    
    Result->InverseTransform = glm::inverse(aiMatrix4x4ToGlm(scene->mRootNode->mTransformation));
    
    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;
    
    Result->SubmeshCount = scene->mNumMeshes;
    Result->Submesh = (Submesh*)PushArray(&GlobalResourceArena, Result->SubmeshCount, Submesh);
    
    //calculate the total number of vertices and indices
    for (u64 MeshIndex = 0; MeshIndex < scene->mNumMeshes; MeshIndex++)
    {
        aiMesh* mesh = scene->mMeshes[MeshIndex];
        Result->VertexCount += mesh->mNumVertices;
        Result->IndexCount += mesh->mNumFaces;
    }
    
    Result->Vertices = (Vertex*)PushArray(&GlobalResourceArena, Result->VertexCount, Vertex);
    
    Result->Indices = (Index*)PushArray(&GlobalResourceArena, Result->IndexCount, Index);
    
    for (u64 MeshIndex = 0; MeshIndex < scene->mNumMeshes; MeshIndex++)
    {
        
        aiMesh* mesh = scene->mMeshes[MeshIndex];
        
        Submesh submesh;
        submesh.BaseVertex = vertexCount;
        submesh.BaseIndex = indexCount;
        submesh.MaterialIndex = mesh->mMaterialIndex;
        submesh.IndexCount = mesh->mNumFaces * 3;
        
        Result->Submesh[MeshIndex] = submesh;
        
        vertexCount += mesh->mNumVertices;
        indexCount += submesh.IndexCount;
        
        // Vertices
        for (u64 VertexIndex = 0; VertexIndex < mesh->mNumVertices; VertexIndex++)
        {
            Vertex vertex;
            vertex.Position = { mesh->mVertices[VertexIndex].x, mesh->mVertices[VertexIndex].y, mesh->mVertices[VertexIndex].z };
            vertex.Normal = { mesh->mNormals[VertexIndex].x, mesh->mNormals[VertexIndex].y, mesh->mNormals[VertexIndex].z };
            
            if (mesh->HasTangentsAndBitangents())
            {
                vertex.Tangent = { mesh->mTangents[VertexIndex].x, mesh->mTangents[VertexIndex].y, mesh->mTangents[VertexIndex].z };
                vertex.Binormal = { mesh->mBitangents[VertexIndex].x, mesh->mBitangents[VertexIndex].y, mesh->mBitangents[VertexIndex].z };
            }
            
            if (mesh->HasTextureCoords(0))
                vertex.Texcoord = { mesh->mTextureCoords[0][VertexIndex].x, mesh->mTextureCoords[0][VertexIndex].y };
            
            Result->Vertices[VertexIndex] = vertex;
        }
        
        // Indices
        for (u64 IndexIndex = 0; IndexIndex < mesh->mNumFaces; IndexIndex++)
        {
            Result->Indices[IndexIndex] = {
                mesh->mFaces[IndexIndex].mIndices[0], mesh->mFaces[IndexIndex].mIndices[1], mesh->mFaces[IndexIndex].mIndices[2] };
        }
        
    }
    Result->IsInitialized = true;
    
    return Result;
}

internal mesh_data*
ReadMeshAsync(char* FileName)
{
    mesh_data* MeshData =  GlobalExecutor.Submit([&FileName]() -> mesh_data*
                                                 {
                                                     mesh_data *MeshData = (mesh_data*)PushStruct(&GlobalResourceArena, mesh_data);
                                                     // Access the shared resource.
                                                     ReadMeshSequential(FileName, MeshData);
                                                     
                                                     GlobalUnUploadedMeshDataCache[GlobalUnUploadedMeshDataCacheCounter++] = MeshData;
                                                     
                                                     return MeshData;
                                                 });
    
    return MeshData;
}





internal void
RenderMesh(mesh_data *Mesh)
{
    if (!Mesh)
        return;
    
    if (Mesh->IsUploaded)
    {
        
        //bind VertexBuffer
        glBindVertexArray(Mesh->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, Mesh->VertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Mesh->IndexBuffer);
        
        /*
        0 Position
        1 Normal
        2 Tangent
        3 Binormal
        4 TexCoord
    */
        for (u64 Index = 0; Index < Mesh->SubmeshCount; Index++)
        {
            Submesh submesh = Mesh->Submesh[Index];
            
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Position));
            
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Normal));
            
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Tangent));
            
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Binormal));
            
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Texcoord));
            
            glDrawElementsBaseVertex(GL_TRIANGLES,
                                     submesh.IndexCount,
                                     GL_UNSIGNED_INT, (void*)(sizeof(u32) * submesh.BaseIndex), submesh.BaseVertex);
        }
        
        
    }
    // TODO(Gabriel): Should we do this here?
    else if (Mesh->IsInitialized && !Mesh->IsUploaded)
    {
        //if not uploaded
        UploadMeshToGPU(Mesh);
    }
    
}

//~ text



internal void
Text2DToMesh(char* Text, s32 X, s32 Y, s32 Size,
             mesh_data *Data)
{
    u64 StrLength = StringLength(Text);
    
    glm::vec2 Vertices[500];
    glm::vec2 UVs[500];
    
    u32 VerticesSpan = 0;
    u32 UVsSpan = 0;
    
    for (u32 i = 0; i < StrLength; i++)
    {
        if (VerticesSpan >= StrLength*6) continue;
        if (UVsSpan >= StrLength*6) continue;
        
		glm::vec2 vertex_up_left    = glm::vec2( X + i * Size       , Y + Size );
		glm::vec2 vertex_up_right   = glm::vec2( X + i * Size + Size, Y + Size );
		glm::vec2 vertex_down_right = glm::vec2( X + i * Size + Size, Y      );
		glm::vec2 vertex_down_left  = glm::vec2( X + i * Size       , Y      );
        
        Vertices[VerticesSpan++] = vertex_up_left;
        Vertices[VerticesSpan++] = vertex_down_left;
        Vertices[VerticesSpan++] = vertex_up_right;
        
        Vertices[VerticesSpan++] = vertex_down_right;
		Vertices[VerticesSpan++] = vertex_up_right;
		Vertices[VerticesSpan++] =vertex_down_left;
        
		char character = Text[i];
		float uv_x = (character % 16) / 16.0f;
		float uv_y = (character / 16) / 16.0f;
        
		glm::vec2 uv_up_left    = glm::vec2( uv_x               , uv_y );
		glm::vec2 uv_up_right   = glm::vec2( uv_x + 1.0f / 16.0f, uv_y );
		glm::vec2 uv_down_right = glm::vec2( uv_x + 1.0f / 16.0f, (uv_y + 1.0f / 16.0f) );
		glm::vec2 uv_down_left  = glm::vec2( uv_x               , (uv_y + 1.0f / 16.0f) );
		
        UVs[UVsSpan++] = uv_up_left;
        UVs[UVsSpan++] = uv_down_left;
        UVs[UVsSpan++] = uv_up_right;
        
        UVs[UVsSpan++] = uv_down_right;
		UVs[UVsSpan++] = uv_up_right;
		UVs[UVsSpan++] = uv_down_left;
	}
    
    glBindVertexArray(Data->VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, Data->VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, VerticesSpan * sizeof(glm::vec2), &Vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, Data->VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, UVsSpan * sizeof(glm::vec2), &UVs[0], GL_STATIC_DRAW);
    
    
    Data->VertexCount = VerticesSpan;
    Data->IndexCount = UVsSpan;
    Data->IsInitialized = true;
    Data->IsUploaded = true;
}


internal mesh_data*
InitText2D(char* Text, s32 X, s32 Y, s32 Size)
{
    mesh_data *MeshData = (mesh_data*)PushStruct(&GlobalResourceArena, mesh_data);
    
    u32 VertexBuffer = 0;
    glGenBuffers(1, &VertexBuffer);
    
    u32 UVBuffer = 0;
    glGenBuffers(1, &UVBuffer);
    
    u32 VAO;
    glGenVertexArrays(1, &VAO);
    
    MeshData->VAO = VAO;
    MeshData->VertexBuffer = VertexBuffer;
    MeshData->IndexBuffer = UVBuffer;
    
    Text2DToMesh(Text, X, Y, Size, MeshData);
    
    return MeshData;
}


#endif //MODEL_H
