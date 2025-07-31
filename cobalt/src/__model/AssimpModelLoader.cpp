#include <__model/AssimpModelLoader.h>

#include <__validation/dispatch.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <unordered_map>
#include <utility>


namespace cobalt::loader
{
    // +---------------------------+
    // | HELPERS FORWARD DECL      |
    // +---------------------------+
    void extract( aiScene const*, std::vector<Vertex>&, std::vector<Model::index_t>& );
    void populate_vertex( aiMesh const*, unsigned int, Vertex& );


    // +---------------------------+
    // | MODEL LOADER              |
    // +---------------------------+
    AssimpModelLoader::AssimpModelLoader( std::filesystem::path path ) : ModelLoader{ std::move( path ) } { }


    void AssimpModelLoader::load( Model& model ) const
    {
        Assimp::Importer importer;

        // Load the model file
        aiScene const* scene = importer.ReadFile(
            model_path_.string( ),
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace );

        if ( !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode )
        {
            validation::throw_runtime_error( "Assimp error while loading model: " + std::string( importer.GetErrorString( ) ) );
        }

        extract( scene, get_vertices( model ), get_indices( model ) );
    }


    // +---------------------------+
    // | HELPERS IMPL              |
    // +---------------------------+
    void extract( aiScene const* scene, std::vector<Vertex>& vertices, std::vector<Model::index_t>& indices )
    {
        std::unordered_map<Vertex, Model::index_t> unique_vertices{};

        for ( unsigned int mesh_idx = 0; mesh_idx < scene->mNumMeshes; ++mesh_idx )
        {
            const aiMesh* mesh = scene->mMeshes[mesh_idx];

            for ( unsigned int i = 0; i < mesh->mNumFaces; ++i )
            {
                const aiFace face = mesh->mFaces[i];
                for ( unsigned int j = 0; j < face.mNumIndices; ++j )
                {
                    const unsigned int vertex_index = face.mIndices[j];

                    Vertex vertex{};
                    populate_vertex( mesh, vertex_index, vertex );

                    // De-duplication of vertices
                    // If the vertex already exists in the map, just push the index
                    if ( unique_vertices.contains( vertex ) )
                    {
                        indices.push_back( unique_vertices[vertex] );
                    }
                    else
                    {
                        // else create a new entry in the map
                        const auto new_index{ static_cast<Model::index_t>( vertices.size( ) ) };
                        unique_vertices[vertex] = new_index;
                        vertices.push_back( vertex );
                        indices.push_back( new_index );
                    }
                }
            }
        }
    }


    void populate_vertex( aiMesh const* mesh, unsigned int const vertex_index, Vertex& vertex )
    {
        vertex.position = {
            mesh->mVertices[vertex_index].x,
            mesh->mVertices[vertex_index].y,
            mesh->mVertices[vertex_index].z
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
                mesh->mTextureCoords[0][vertex_index].x,
                mesh->mTextureCoords[0][vertex_index].y
            };
        }

        vertex.color = { 1.0f, 1.0f, 1.0f };
    }

}
