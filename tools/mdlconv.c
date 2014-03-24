#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <smmintrin.h>

#include "linmath.h"

#define AI_CONFIG_PP_SBP_REMOVE aiPrimitiveType_LINE|aiPrimitiveType_POINT

static const char MAGIC_NUMBER = 236;

static int fwritestr(char *str, FILE *fp)
{
	int len = strlen(str);
	assert(len <= 255);
	fputc((unsigned char)len, fp);
	return fwrite(str, len, 1, fp);
}

static int processMaterialTexture(struct aiMaterial *m, unsigned int type)
{
	struct aiString path;
	enum aiReturn result;

	result = aiGetMaterialString(m, AI_MATKEY_TEXTURE(type, 0), &path);

	if (result == aiReturn_SUCCESS) {
		fprintf(stderr, "type(%d) = %s\n", type, path.data);
	}

	fputc((unsigned char)type, stdout);
	fwritestr(path.data, stdout);

	return 0;
}

static int processMaterial(struct aiMaterial *m)
{
	struct aiString name;

	aiGetMaterialString(m, AI_MATKEY_NAME, &name);
	fwritestr(name.data, stdout);

	processMaterialTexture(m, aiTextureType_DIFFUSE);
	processMaterialTexture(m, aiTextureType_SPECULAR);
	processMaterialTexture(m, aiTextureType_NORMALS);
	processMaterialTexture(m, aiTextureType_EMISSIVE);

	return 0;
}

// TODO(cloudhead): Reduce fwrite calls.
static int processMesh(struct aiMesh *m, char *name)
{
	int nverts = m->mNumVertices;

	fwritestr(name, stdout);

	// Shader name
	char *shader = "default";
	fwritestr(shader, stdout);

	// Material index
	fwrite(&m->mMaterialIndex, sizeof(m->mMaterialIndex), 1, stdout);

	// Vertex count
	fwrite(&nverts, 4, 1, stdout);

	assert(m->mVertices);
	assert(m->mTangents);
	assert(m->mBitangents);
	assert(m->mNormals);

	for (int i = 0; i < nverts; i++) {
		// Vertex positions
		fwrite(&m->mVertices[i].x, 4, 1, stdout);
		fwrite(&m->mVertices[i].y, 4, 1, stdout);
		fwrite(&m->mVertices[i].z, 4, 1, stdout);

		// Vertex UV coordinates
		assert(m->mTextureCoords[0]);
		fwrite(&m->mTextureCoords[0][i].x, 4, 1, stdout);
		fwrite(&m->mTextureCoords[0][i].y, 4, 1, stdout);

		// Vertex normals
		fwrite(&m->mNormals[i].x, 4, 1, stdout);
		fwrite(&m->mNormals[i].y, 4, 1, stdout);
		fwrite(&m->mNormals[i].z, 4, 1, stdout);

		// Vertex tangents (should already be normalized)
		vec3 t = (vec3){m->mTangents[i].x, m->mTangents[i].y, m->mTangents[i].z};
		vec3 n = (vec3){m->mNormals[i].x, m->mNormals[i].y, m->mNormals[i].z};
		vec3 b = (vec3){m->mBitangents[i].x, m->mBitangents[i].y, m->mBitangents[i].z};

		fwrite(&t.x, 4, 1, stdout);
		fwrite(&t.y, 4, 1, stdout);
		fwrite(&t.z, 4, 1, stdout);

		// Determinant
		float d = vec3dot(vec3cross(n, t), b) < 0.0f ? -1.0f : 1.0f;
		fwrite(&d, 4, 1, stdout);
	}

	int nfaces = m->mNumFaces;
	fwrite(&nfaces, 4, 1, stdout);

	for (int i = 0; i < nfaces; i++) {
		struct aiFace f = m->mFaces[i];

		assert(f.mNumIndices == 3);

		for (int j = 0; j < 3; j++) {
			fwrite(&f.mIndices[j], 4, 1, stdout);
		}
	}
	return 0;
}

static int process(const char *path)
{
	const struct aiScene *scene = NULL;

	scene = aiImportFile(path,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_FindInvalidData |
		aiProcess_FlipWindingOrder |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType
	);

	if (! scene) {
		fprintf(stderr, "import error: %s\n", aiGetErrorString());
		return 1;
	}
	int nMeshes = scene->mNumMeshes;
	int nMaterials = scene->mNumMaterials;
	struct aiMesh **meshes = scene->mMeshes;
	struct aiMaterial **materials = scene->mMaterials;

	// Header
	unsigned char magic = MAGIC_NUMBER;
	fwrite(&magic, 1, 1, stdout);

	if (0) {
		// Materials
		fprintf(stderr, "loading %d materials..\n", nMaterials);
		fwrite(&nMaterials, 4, 1, stdout);

		for (int i = 0; i < nMaterials; i++) {
			processMaterial(materials[i]);
		}
	}

	// Nodes & Meshes
	fprintf(stderr, "loading %d meshes..\n", nMeshes);
	fwrite(&nMeshes, 4, 1, stdout);

	int nNodes = scene->mRootNode->mNumChildren;
	for (int i = 0; i < nNodes; i++) {
		struct aiNode *n = scene->mRootNode->mChildren[i];
		assert(n->mNumMeshes == 1);
		processMesh(meshes[n->mMeshes[0]], n->mName.data);

	}
	aiReleaseImport(scene);

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s <filepath>\n", argv[0]);
		exit(1);
	}
	return process(argv[1]);
}
