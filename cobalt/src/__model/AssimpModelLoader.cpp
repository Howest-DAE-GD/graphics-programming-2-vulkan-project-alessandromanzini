#include <__model/AssimpModelLoader.h>

#include <__validation/dispatch.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <span>
#include <unordered_map>
#include <utility>


namespace cobalt::loader
{
    // +---------------------------+
    // | HELPERS FORWARD DECL      |
    // +---------------------------+
    void extract_meshes( aiScene const*, std::vector<Vertex>&, std::vector<uint32_t>&, std::vector<Mesh>& );
    void extract_materials( aiScene const*, std::vector<SurfaceMap>&, std::vector<TextureGroup>&, std::filesystem::path const& );
    int fetch_texture_data( aiMaterial const*, aiTextureType, std::vector<TextureGroup>&, std::filesystem::path const& );

    [[nodiscard]] glm::vec3 to_vec3( aiVector3D const& vec ) { return { vec.x, vec.y, vec.z }; }
    [[nodiscard]] glm::vec3 to_vec3( aiColor3D const& color ) { return { color.r, color.g, color.b }; }
    [[nodiscard]] glm::vec2 to_vec2( aiVector3D const& vec ) { return { vec.x, vec.y }; }

    [[nodiscard]] TextureType to_tex_type( aiTextureType const type );


    // +---------------------------+
    // | MODEL LOADER              |
    // +---------------------------+
    AssimpModelLoader::AssimpModelLoader( std::filesystem::path path ) : ModelLoader{ std::move( path ) } { }


    void AssimpModelLoader::load( std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<Mesh>& meshes,
                                  std::vector<SurfaceMap>& surface_maps, std::vector<TextureGroup>& textures ) const
    {
        Assimp::Importer importer{};
        aiScene const* const scene = importer.ReadFile(
            model_path_.string( ),
            aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace |
            aiProcess_ConvertToLeftHanded | aiProcess_PreTransformVertices | aiProcess_JoinIdenticalVertices |
            aiProcess_OptimizeMeshes | aiProcess_ValidateDataStructure );

        if ( !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode )
        {
            validation::throw_runtime_error( "Assimp error while loading model: " + std::string( importer.GetErrorString( ) ) );
        }
        extract_meshes( scene, vertices, indices, meshes );
        extract_materials( scene, surface_maps, textures, base_path_ );
    }


    // +---------------------------+
    // | HELPERS IMPL              |
    // +---------------------------+
    void extract_meshes( aiScene const* scene, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
                         std::vector<Mesh>& meshes )
    {
        int32_t vertex_offset{};
        uint32_t index_offset{};

        for ( aiMesh const* const mesh : std::span{ scene->mMeshes, std::next( scene->mMeshes, scene->mNumMeshes ) } )
        {
            uint32_t const num_vertices = mesh->mNumVertices;
            vertices.reserve( vertices.size( ) + num_vertices );
            for ( size_t vertex_idx{}; vertex_idx < num_vertices; ++vertex_idx )
            {
                vertices.emplace_back(
                    to_vec3( mesh->mVertices[vertex_idx] ),
                    to_vec2( mesh->mTextureCoords[0][vertex_idx] ),
                    to_vec3( mesh->mNormals[vertex_idx] ),
                    to_vec3( mesh->mTangents[vertex_idx] ),
                    to_vec3( mesh->mBitangents[vertex_idx] ) );
            }

            uint32_t const num_indices = mesh->mNumFaces * 3;
            indices.reserve( indices.size( ) + num_indices );
            for ( aiFace const& face : std::span{ mesh->mFaces, std::next( mesh->mFaces, mesh->mNumFaces ) } )
            {
                std::copy( face.mIndices, std::next( face.mIndices, face.mNumIndices ), std::back_inserter( indices ) );
            }

            meshes.emplace_back( num_indices, index_offset, vertex_offset, mesh->mMaterialIndex );
            vertex_offset += num_vertices;
            index_offset += num_indices;
        }
    }


    void extract_materials( aiScene const* scene, std::vector<SurfaceMap>& surface_maps, std::vector<TextureGroup>& textures,
                            std::filesystem::path const& base_path )
    {
        surface_maps.reserve( scene->mNumMaterials );
        for ( aiMaterial const* const mat : std::span{ scene->mMaterials, std::next( scene->mMaterials, scene->mNumMaterials ) } )
        {
            int diffuse   = fetch_texture_data( mat, aiTextureType_BASE_COLOR, textures, base_path );
            int normal    = fetch_texture_data( mat, aiTextureType_NORMALS, textures, base_path );
            int metalness = fetch_texture_data( mat, aiTextureType_METALNESS, textures, base_path );
            int roughness = fetch_texture_data( mat, aiTextureType_DIFFUSE_ROUGHNESS, textures, base_path );
            surface_maps.emplace_back( diffuse, normal, metalness, roughness );
        }
    }


    int fetch_texture_data( aiMaterial const* mat, aiTextureType const type, std::vector<TextureGroup>& textures,
                            std::filesystem::path const& base_path )
    {
        if ( mat->GetTextureCount( type ) <= 0 )
        {
            return -1;
        }

        aiString relative_path{};
        mat->GetTexture( type, 0, &relative_path );
        std::filesystem::path const texture_path{ base_path / relative_path.C_Str( ) };
        if ( not exists( texture_path ) )
        {
            return -1;
        }

        auto const it =
            std::ranges::find_if( textures, [&texture_path]( TextureGroup const& tex ) { return tex.path == texture_path; } );
        if ( it != textures.end( ) )
        {
            return std::distance( textures.begin( ), it );
        }

        textures.emplace_back( to_tex_type( type ), std::move( texture_path ) );
        return static_cast<int>( textures.size( ) - 1 );
    }


    TextureType to_tex_type( aiTextureType const type )
    {
        switch ( type )
        {
            case aiTextureType_DIFFUSE:
            case aiTextureType_BASE_COLOR:
                return TextureType::DIFFUSE;

            case aiTextureType_NORMALS:
            case aiTextureType_NORMAL_CAMERA:
                return TextureType::NORMAL;

            case aiTextureType_METALNESS:
                return TextureType::METALNESS;

            case aiTextureType_DIFFUSE_ROUGHNESS:
                return TextureType::ROUGHNESS;

            // Add other texture types as needed
            default:
                throw std::runtime_error( "unsupported texture type" );
        }
    }

}
