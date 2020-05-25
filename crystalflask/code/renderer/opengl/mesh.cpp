
internal mesh_data*
GetPrimitiveMeshData(primitives Primitive)
{
    return GlobalPrimitiveMeshDataCache[(unsigned char)Primitive];
}

internal void
LoadPrimitives()
{
    
    GlobalPrimitiveMeshDataCache[(unsigned char)PRIMITIVE_SPHERE] = ReadMeshAsync("resources/primitives/sphere.obj");
    
    GlobalPrimitiveMeshDataCache[(unsigned char)PRIMITIVE_PLANE] = ReadMeshAsync("resources/primitives/plane.obj");
    
    GlobalPrimitiveMeshDataCache[(unsigned char)PRIMITIVE_CUBE] = ReadMeshAsync("resources/primitives/cube.obj");
    
    GlobalPrimitiveMeshDataCache[(unsigned char)PRIMITIVE_CUBE_BLENDER] = ReadMeshAsync("resources/primitives/cube_blender.obj");
    
    GlobalPrimitiveMeshDataCache[(unsigned char)PRIMITIVE_CYLINDER] = ReadMeshAsync("resources/primitives/cylinder.obj");
    
    GlobalPrimitiveMeshDataCache[(unsigned char)PRIMITIVE_CONE] = ReadMeshAsync("resources/primitives/cone.obj");
    
    GlobalPrimitiveMeshDataCache[(unsigned char)PRIMITIVE_ICOSPHERE] = ReadMeshAsync("resources/primitives/icosphere.obj");
    
    GlobalPrimitiveMeshDataCache[(unsigned char)PRIMITIVE_MONKEY] = ReadMeshAsync("resources/primitives/monkey.obj");
    
    GlobalPrimitiveMeshDataCache[(unsigned char)PRIMITIVE_WATER] = ReadMeshAsync("resources/primitives/water.obj");
}