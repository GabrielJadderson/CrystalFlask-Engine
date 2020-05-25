
internal entity*
CreateEntity(scene *Scene, char *Name)
{
    if (!Scene)
        return NULL;
    
    entity Entity = {};
    Scene->EntityCount++;
    Entity.EntityIndex = ((u32)Scene->EntityCount);
    
    // NOTE(Gabriel): The Name is constrained to a maximum of 64 characters! this is very important.
    Entity.Name = (stack_string_64*)PushStruct(&GlobalResourceArena, stack_string_64);
    if (Name == NULL)
    {
        Entity.Name->Length = sprintf_s(Entity.Name->String, 64, "Entity %d", Entity.EntityIndex);
    } else
    {
        Entity.Name->Length = sprintf_s(Entity.Name->String, 64, "%s", Name);
    }
    
    Entity.Position = {0,0,0};
    Entity.Rotation = {0,0,0};
    Entity.Scale = {1,1,1};
    Entity.Enabled = true;
    Entity.HasChild = false;
    Entity.Child = NULL;
    
    if (Scene->EntityCache == NULL)
    {
        Scene->EntityCacheMaxCount = 20000;
        Scene->EntityCache = PushArray(&GlobalResourceArena, Scene->EntityCacheMaxCount, entity);
        Scene->EntityCacheCount = 0;
    }
    
    
    
    //get a poitner from an already allocated, but deleted entity.
    if (Scene->DeletedEntityCache.Count > 0)
    {
        entity* Result = (entity*)Scene->DeletedEntityCache.Pop();
        
        *Result = Entity;
        
        return Result;
    }
    
    
    if (Scene->EntityCacheCount > Scene->EntityCacheMaxCount)
    {
        Win32MessageBoxError("ENTITY maximum of %d/%d reached! QUITTING BEFORE UNDEFINED BEHAVIOR STARTS.", Scene->EntityCacheCount, Scene->EntityCacheMaxCount);
        exit(1);
    } else if (Scene->EntityCacheCount == Scene->EntityCacheMaxCount)
    {
        // TODO(Gabriel): Resize
        Win32MessageBoxError("Cannot create any more entities for the scene maximum of %d/%d reached", Scene->EntityCacheCount, Scene->EntityCacheMaxCount);
    }
    
    
    
    Scene->EntityCache[Scene->EntityCacheCount] = Entity;
    entity* Result = &Scene->EntityCache[Scene->EntityCacheCount];
    
    Scene->EntityCacheCount++;
    
    return Result;
}


internal entity*
CreateEntity(scene *Scene, char* Name, glm::vec3 &Position, glm::vec3 &Rotation, glm::vec3 &Scale)
{
    if (!Scene)
        return NULL;
    
    entity Entity = {};
    Scene->EntityCount++;
    Entity.EntityIndex = ((u32)Scene->EntityCount);
    
    // NOTE(Gabriel): The Name is constrained to a maximum of 64 characters! this is very important.
    Entity.Name = (stack_string_64*)PushStruct(&GlobalResourceArena, stack_string_64);
    if (Name == NULL)
    {
        Entity.Name->Length = sprintf_s(Entity.Name->String, 64, "Entity %d", Entity.EntityIndex);
    }
    else
    {
        Entity.Name->Length = sprintf_s(Entity.Name->String, 64, "%s", Name);
    }
    
    Entity.Position = Position;
    Entity.Rotation = Rotation;
    Entity.Scale = Scale;
    Entity.Enabled = true;
    Entity.HasChild = false;
    Entity.Child = NULL;
    
    if (Scene->EntityCache == NULL)
    {
        Scene->EntityCacheMaxCount = 20000;
        Scene->EntityCache = PushArray(&GlobalResourceArena, Scene->EntityCacheMaxCount, entity);
        Scene->EntityCacheCount = 0;
    }
    
    //get a poitner from an already allocated, but deleted entity.
    if (Scene->DeletedEntityCache.Count > 0)
    {
        entity* Result = (entity*)Scene->DeletedEntityCache.Pop();
        *Result = Entity;
        return Result;
    }
    
    
    if (Scene->EntityCacheCount > Scene->EntityCacheMaxCount)
    {
        Win32MessageBoxError("ENTITY maximum of %d/%d reached! QUITTING BEFORE UNDEFINED BEHAVIOR STARTS.", Scene->EntityCacheCount, Scene->EntityCacheMaxCount);
        exit(1);
    } else if (Scene->EntityCacheCount == Scene->EntityCacheMaxCount)
    {
        // TODO(Gabriel): Resize
        Win32MessageBoxError("Cannot create any more entities for the scene maximum of %d/%d reached", Scene->EntityCacheCount, Scene->EntityCacheMaxCount);
    }
    
    
    
    Scene->EntityCache[Scene->EntityCacheCount] = Entity;
    
    entity* Result = &Scene->EntityCache[Scene->EntityCacheCount];
    Scene->EntityCacheCount++;
    
    return Result;
}


internal entity*
CreateEntityPrimitive(scene *Scene, char *Name, primitives Primitive)
{
    entity* Entity = CreateEntity(Scene, Name);
    Entity->MeshData = GetPrimitiveMeshData(Primitive);
    return Entity;
}

internal entity*
CreateEntityPrimitive(scene *Scene, char *Name, primitives Primitive, glm::vec3 &Position, glm::vec3 &Rotation, glm::vec3 &Scale)
{
    entity* Entity = CreateEntity(Scene, Name, Position, Rotation, Scale);
    Entity->MeshData = GetPrimitiveMeshData(Primitive);
    return Entity;
}

internal entity*
CreateEntityWithMesh(scene *Scene, char* Name, char *MeshPath)
{
    entity* Entity = CreateEntity(Scene, Name);
    Entity->MeshData = ReadMeshAndUpload(MeshPath);
    return Entity;
}

internal entity*
CreateEntityWithMesh(scene *Scene, char* Name, char *MeshPath, glm::vec3 &Position, glm::vec3 &Rotation, glm::vec3 &Scale)
{
    entity* Entity = CreateEntity(Scene, Name, Position, Rotation, Scale);
    Entity->MeshData = ReadMeshAndUpload(MeshPath);
    return Entity;
}


//~

internal void
UpdateAndRenderEntity(entity *Entity, glm::mat4 &ViewMatrix,glm::mat4 &ProjectionMatrix)
{
    if (Entity)
    {
        if (Entity->Enabled && !Entity->IsDeleted)
        {
            //1. Update Entity
            
            
            //2. Render the entity from it's meshdata.
            if (Entity->MeshData)
            {
                glBindVertexArray(Entity->MeshData->VAO);
                
                
                glm::mat4 ModelMatrix = glm::mat4(1.0);
                //Translate * Rotate * Scale
                ModelMatrix = glm::translate(ModelMatrix, Entity->Position);
                
                ModelMatrix = ModelMatrix * glm::eulerAngleXYZ(glm::radians(Entity->Rotation.x), glm::radians(Entity->Rotation.y), glm::radians(Entity->Rotation.z));
                ModelMatrix = glm::scale(ModelMatrix, Entity->Scale);
                glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
                
                
                ShaderBasicSubmit(MVP);
                RenderMesh(Entity->MeshData);
            }
        }
    }
}


entity*
CreateEntityFromFile(scene *Scene, char* FilePath)
{
    // TODO(Gabriel): FIX ASSET IMPORTER FIRST!
    win32_file File = Win32ReadDataFromFile(FilePath);
    if (File.Size == 0) return 0;
    
    entity* ImportedEntity = (entity*)File.Data;
    
    entity* Result = CreateEntity(Scene, NULL);
    
    Result->Name = ImportedEntity->Name;
    Result->Position = ImportedEntity->Position;
    Result->Rotation = ImportedEntity->Rotation;
    Result->Scale = ImportedEntity->Scale;
    Result->Material = ImportedEntity->Material;
    Result->MeshData = ImportedEntity->MeshData;
    Result->Enabled = ImportedEntity->Enabled;
    Result->SortIndex = ImportedEntity->SortIndex;
    
    free(File.Data);
    return Result;
}

void
DeleteEntity(scene *Scene, entity* Entity)
{
    if (Entity)
    {
        if (!Entity->IsDeleted)
        {
            Entity->IsDeleted = true;
            Scene->DeletedEntityCache.Push(Entity);
        }
    }
}

void
DeleteAllEntitiesInScene(scene *Scene)
{
    
    for (u64 Index = 0; Index < Scene->EntityCacheCount; Index++)
    {
        entity* Entity = &Scene->EntityCache[Index];
        if (!Entity->IsDeleted)
        {
            Entity->IsDeleted = true;
            Scene->DeletedEntityCache.Push(Entity);
        }
    }
    
}


void
SerializeEntityToFile(entity *Entity, char* FilePath)
{
    Win32SaveDataToFile(FilePath, (unsigned char*)Entity, sizeof(entity));
}

