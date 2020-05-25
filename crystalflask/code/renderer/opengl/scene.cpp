

internal scene*
CreateScene(const char *SceneName)
{
    scene *Scene = PushStruct(&GlobalResourceArena, scene);
    sprintf_s(Scene->SceneName, 256, "%s", SceneName);
    //use memcpy instead?
    
    
    return Scene;
}

internal b32
LoadScene(scene *Scene)
{
    if (!Scene) return false;
    
    GlobalPointerEntityCache = Scene->EntityCache;
    GlobalPointerEntityCacheCount = Scene->EntityCacheCount;
    
    
    return true;
}

