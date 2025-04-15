#include <builders/ModelLoader.h>

#include <validation/dispatch.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <unordered_map>
#include <utility>


namespace cobalt_vk::builders
{
    // +---------------------------+
    // | HELPERS FORWARD DECL      |
    // +---------------------------+
    void extract( const aiScene* scene, std::vector<Vertex>& vertices, std::vector<Model::index_t>& indices );
    void populate_vertex( const aiMesh* mesh, unsigned int vertexIndex, Vertex& vertex );


    // +---------------------------+
    // | MODEL LOADER              |
    // +---------------------------+
    ModelLoader::ModelLoader( std::filesystem::path path )
        : model_path_{ std::move( path ) } { }


    void ModelLoader::load( Model& model ) const
    {
        Assimp::Importer importer;

        // Load the model file
        const aiScene* scene = importer.ReadFile(
            model_path_.string( ),
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace );

        if ( !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode )
        {
            validation::throw_runtime_error( "Assimp error while loading model: " + std::string( importer.GetErrorString( ) ) );
        }

        extract( scene, model.vertices_, model.indices_ );
    }


    // +---------------------------+
    // | HELPERS IMPL              |
    // +---------------------------+
    void extract( const aiScene* scene, std::vector<Vertex>& vertices, std::vector<Model::index_t>& indices )
    {
        std::unordered_map<Vertex, Model::index_t> uniqueVertices{};

        for ( unsigned int meshIdx = 0; meshIdx < scene->mNumMeshes; ++meshIdx )
        {
            const aiMesh* mesh = scene->mMeshes[meshIdx];

            for ( unsigned int i = 0; i < mesh->mNumFaces; ++i )
            {
                const aiFace face = mesh->mFaces[i];
                for ( unsigned int j = 0; j < face.mNumIndices; ++j )
                {
                    const unsigned int vertexIndex = face.mIndices[j];

                    Vertex vertex{};
                    populate_vertex( mesh, vertexIndex, vertex );

                    // De-duplication of vertices
                    // If the vertex already exists in the map, just push the index
                    if ( uniqueVertices.contains( vertex ) )
                    {
                        indices.push_back( uniqueVertices[vertex] );
                    }
                    else
                    {
                        // else create a new entry in the map
                        const auto newIndex{ static_cast<Model::index_t>( vertices.size( ) ) };
                        uniqueVertices[vertex] = newIndex;
                        vertices.push_back( vertex );
                        indices.push_back( newIndex );
                    }
                }
            }
        }
    }


    void populate_vertex( const aiMesh* mesh, const unsigned int vertexIndex, Vertex& vertex )
    {
        vertex.position = {
            mesh->mVertices[vertexIndex].x,
            mesh->mVertices[vertexIndex].y,
            mesh->mVertices[vertexIndex].z
        };

        if ( mesh->HasNormals( ) )
        {
            // vertex.normal = {
            //     mesh->mNormals[vertexIndex].x,
            //     mesh->mNormals[vertexIndex].y,
            //     mesh->mNormals[vertexIndex].z
            // };
        }

        if ( mesh->HasTextureCoords( 0 ) )
        {
            vertex.texCoord = {
                mesh->mTextureCoords[0][vertexIndex].x,
                mesh->mTextureCoords[0][vertexIndex].y
            };
        }

        vertex.color = { 1.0f, 1.0f, 1.0f };
    }

}
