#include <stdlib.h>
#include <stdio.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <smmintrin.h>
#include <assert.h>

#include "linmath.h"
#include "common.h"

#define AI_CONFIG_PP_SBP_REMOVE aiPrimitiveType_LINE|aiPrimitiveType_POINT
#define elems(a) (sizeof(a) / sizeof(a[0]))

static const char MAGIC_NUMBER = 236;

static struct aiMatrix4x4 aiMatrix4x4mul(struct aiMatrix4x4 *a, struct aiMatrix4x4 *b)
{
	mat4 a1 = (mat4){
		(vec4){a->a1, a->b1, a->c1, a->d1},
		(vec4){a->a2, a->b2, a->c2, a->d2},
		(vec4){a->a3, a->b3, a->c3, a->d3},
		(vec4){a->a4, a->b4, a->c4, a->d4}
	};
	mat4 b1 = (mat4){
		(vec4){b->a1, b->b1, b->c1, b->d1},
		(vec4){b->a2, b->b2, b->c2, b->d2},
		(vec4){b->a3, b->b3, b->c3, b->d3},
		(vec4){b->a4, b->b4, b->c4, b->d4}
	};
	mat4 result = mat4mul(a1, b1);

	return (struct aiMatrix4x4){
		result.a1, result.b1, result.c1, result.d1,
		result.a2, result.b2, result.c2, result.d2,
		result.a3, result.b3, result.c3, result.d3,
		result.a4, result.b4, result.c4, result.d4,
	};
}

static struct aiNode *findNode(struct aiNode *node, const char *name)
{
	if (! strcmp(node->mName.data, name))
		return node;

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		struct aiNode *p = findNode(node->mChildren[i], name);

		if (p)
			return p;
	}
	return NULL;
}

static int fwritestr(char *str, FILE *fp)
{
	int len = strlen(str);
	assert(len <= 255);
	fputc((unsigned char)len, fp);
	return fwrite(str, len, 1, fp);
}

static int fwritemat4(struct aiMatrix4x4 *m, FILE *fp)
{
	mat4 mat = (mat4){
		(vec4){m->a1, m->b1, m->c1, m->d1},
		(vec4){m->a2, m->b2, m->c2, m->d2},
		(vec4){m->a3, m->b3, m->c3, m->d3},
		(vec4){m->a4, m->b4, m->c4, m->d4}
	};

	return fwrite(&mat, 4, 16, stdout);
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

static int processMesh(struct aiMesh *m, char *name, struct aiNode *root)
{
	int nverts = m->mNumVertices;
	int nbones = m->mNumBones;

	struct vertex vertices[nverts];

	fprintf(stderr, "mesh '%s'\n", name);
	fwritestr(name, stdout);

	// Shader name
	char *shader = "default";
	fwritestr(shader, stdout);

	// Material index
	fwrite(&m->mMaterialIndex, sizeof(m->mMaterialIndex), 1, stdout);

	// Bone count
	fwrite(&nbones, 4, 1, stdout);
	for (int i = 0; i < nbones; i++) {
		struct aiString name = m->mBones[i]->mName;
		struct aiNode *node = findNode(root, name.data);

		assert(node);

		fwritestr(name.data, stdout);
		fwritemat4(&m->mBones[i]->mOffsetMatrix, stdout);

		struct aiNode *n = node;
		struct aiMatrix4x4 t = node->mTransformation;
		while (n != root) {
			n = n->mParent;
			t = aiMatrix4x4mul(&n->mTransformation, &t);
		}
		// Write transform matrix
		fwritemat4(&t, stdout);

		int parentId = -1;
		assert(node->mParent);

		if (node->mParent != root) {
			for (int j = 0; j < nbones; j++) {
				if (! strcmp(m->mBones[j]->mName.data, node->mParent->mName.data)) {
					parentId = j;
				}
			}
		}
		fwrite(&parentId, 4, 1, stdout);
	}
	assert(m->mVertices);
	assert(m->mTangents);
	assert(m->mBitangents);
	assert(m->mNormals);

	for (int i = 0; i < nverts; i++) {
		// Vertex tangents (should already be normalized)
		vec3  t = (vec3){m->mTangents[i].x, m->mTangents[i].y, m->mTangents[i].z};
		vec3  n = (vec3){m->mNormals[i].x, m->mNormals[i].y, m->mNormals[i].z};
		vec3  b = (vec3){m->mBitangents[i].x, m->mBitangents[i].y, m->mBitangents[i].z};
		float d = vec3dot(vec3cross(n, t), b) < 0.0f ? -1.0f : 1.0f;

		vertices[i] = (struct vertex){
			.pos     = {m->mVertices[i].x, m->mVertices[i].y, m->mVertices[i].z},
			.uv      = {m->mTextureCoords[0][i].x, m->mTextureCoords[0][i].y},
			.normal  = {n.x, n.y, n.z},
			.tangent = {t.x, t.y, t.z, d},
			.bones   = {-1, -1, -1, -1},
			.weights = {0.0f, 0.0f, 0.0f, 0.0f}
		};
	}

	for (int i = 0; i < nbones; i++) {
		struct aiBone *b = m->mBones[i];

		fprintf(stderr, "bone '%s'\n", b->mName.data);

		for (int j = 0; j < b->mNumWeights; j++) {
			struct vertex *v = &vertices[b->mWeights[j].mVertexId];
			float weight = b->mWeights[j].mWeight;

			if (weight < 0.01f)
				continue;

			// Find index to store blend data
			for (int k = 0; k < elems(v->bones); k++) {
				if (v->bones[k] == -1) {
					v->bones[k] = i;
					v->weights[k] = weight;
				}
			}
		}
	}
	// Vertex count
	fwrite(&nverts, 4, 1, stdout);
	fwrite(vertices, sizeof(struct vertex), nverts, stdout);

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

static int processNode(struct aiNode *node, struct aiMesh **meshes, struct aiNode *root)
{
	int nNodes = node->mNumChildren;

	for (int i = 0; i < nNodes; i++) {
		struct aiNode *n = node->mChildren[i];
		fprintf(stderr, "node '%s'\n", n->mName.data);

		if (n->mNumMeshes > 0) {
			int index = n->mMeshes[0];
			struct aiMesh *m = meshes[index];
			assert(m);

			processMesh(m, n->mName.data, root);
		}
		processNode(n, meshes, root);
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
		fprintf(stderr, "loading materials (%d)..\n", nMaterials);
		fwrite(&nMaterials, 4, 1, stdout);

		for (int i = 0; i < nMaterials; i++) {
			processMaterial(materials[i]);
		}
	}

	// Nodes & Meshes
	fprintf(stderr, "loading meshes (%d)..\n", nMeshes);
	fwrite(&nMeshes, 4, 1, stdout);

	processNode(scene->mRootNode, meshes, scene->mRootNode);

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
