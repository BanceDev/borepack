#include "glad/glad.h"
#include <iostream>
#include <string_view>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "world.h"
#include "camera.h"
#include "map.h"

GLuint shaderProgram;

GLuint LoadShader(const char *vert, const char *frag) {
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vert, NULL);
	glCompileShader(vertexShader);

	GLint success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cerr << "Vertex Shader Compilation Error: " << infoLog << std::endl;
		return 0;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &frag, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar infoLog[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cerr << "Fragment Shader Compilation Error: " << infoLog << std::endl;
		return 0;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		GLchar infoLog[512];
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		std::cerr << "Shader Program Linking Error: " << infoLog << std::endl;
		return 0;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return program;
}

//
// Initialize World
//
bool World::Initialize(Map *map, int width, int height)
{
	this->map = map;

	// Enable OpenGL features
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glDrawBuffer(GL_BACK);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Setup the viewport and frustum
	glViewport(0, 0, width, height);

	// Initialize shaders (replace with your own shader sources)
	const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec2 texCoord;
        uniform mat4 modelview;
        uniform mat4 projection;
        out vec2 TexCoord;
        void main()
        {
            gl_Position = projection * modelview * vec4(position, 1.0);
            TexCoord = texCoord;
        })";

	const char* fragmentShaderSource = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D texture1;
        void main()
        {
            FragColor = texture(texture1, TexCoord);
        })";
	shaderProgram = LoadShader(vertexShaderSource, fragmentShaderSource);
		
	// TODO: move this shit to some kind of player config later
	float fov = 90.0f;        // Field of view in degrees (vertical)
	float nearPlane = 4.0f;    // Near clipping plane
	float farPlane = 4096.0f;  // Far clipping plane
	float aspectRatio = 1.33f; // Assuming a 4:3 aspect ratio

	// Convert FOV to radians
	float fovRad = glm::radians(fov);

	// Calculate the projection matrix using glm::frustum
	float tangent = tan(fovRad / 2.0f);
	float view_height = nearPlane * tangent; // Half the height of the near plane
	float view_width = view_height * aspectRatio; // Half the width of the near plane

	glm::mat4 projection = glm::frustum(-view_width, view_width, -view_height, view_height, nearPlane, farPlane);
	// projection and view matrices
	
	glUseProgram(shaderProgram);
	GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
	if (projectionLoc == -1) {
		std::cerr << "Uniform 'projection' not found in shader program!" << std::endl;
		return false;
	}
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);

	// Build textures
	if (!InitializeTextures()) {
		return false;
	}

	// Calculate surfaces
	if (!InitializeSurfaces()) {
		return false;
	}

	return true;
}

//
// Draw the scene
//
void World::DrawScene(Camera *camera)
{
	// Clear the drawbuffer
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	// Setup a viewing matrix and transformation
	glm::mat4 view = glm::lookAt(glm::vec3(camera->head[0], camera->head[1], camera->head[2]),
                                 glm::vec3(camera->head[0] + camera->view[0], camera->head[1] + camera->view[1], camera->head[2] + camera->view[2]),
                                 glm::vec3(0.0f, 0.0f, 1.0f));
	
	glUseProgram(shaderProgram);
	GLint modelViewLoc = glGetUniformLocation(shaderProgram, "modelview");
	glUniformMatrix4fv(modelViewLoc, 1, GL_FALSE, &view[0][0]);

	// Find the leaf the camera is in
	bspleaf_t *leaf = FindLeaf(camera);

	// Render the scene
	DrawLeafVisibleSet(leaf);
}

//
// Draw the surface
//
void World::DrawSurface(int surface)
{
	auto it = map->surfaceVAOs.find(surface);
	if (it != map->surfaceVAOs.end()) {
		GLuint VAO = it->second;
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, map->getNumEdges(surface));
		glBindVertexArray(0);
	} else {
		std::cerr << "Error: VAO not found for surface " << surface << std::endl;
	}
}

//
// Draw the visible surfaces
//
void World::DrawSurfaceList(int *visibleSurfaces, int numVisibleSurfaces)
{
	// Loop through all the visible surfaces and activate the texture objects
	for (int i = 0; i < numVisibleSurfaces; i++) {
		// Get a pointer to the texture info so we know which texture we should bind and draw
		texinfo_t *textureInfo = map->getTextureInfo(visibleSurfaces[i]);
		// Bind the previously created texture object
		glBindTexture(GL_TEXTURE_2D, textureObjNames[textureInfo->texid]);
		// Draw the surface
		DrawSurface(visibleSurfaces[i]);
	}
}

//
// Calculate which other leaves are visible from the specified leaf, fetch the associated surfaces and draw them
//
void World::DrawLeafVisibleSet(bspleaf_t *pLeaf)
{
	int numVisibleSurfaces = 0;

	// Get a pointer to the visibility list that is associated with the BSP leaf
	unsigned char *visibilityList = map->getVisibilityList(pLeaf->vislist);

	// Loop trough all leaves to see if they are visible
	for (int i = 1; i < map->getNumLeaves(); visibilityList++) {
		// Check the visibility list if the current leaf is seen
		if (*visibilityList) {
			// Loop through the visible leaves
			for (int j = 0; j < 8; j++, i++) {
				// Fetch the leaf that is seen from the array of leaves
				bspleaf_t *visibleLeaf = map->getLeaf(i);
				// Copy all the visible surfaces from the List of surfaces
				int firstSurface = visibleLeaf->firstsurf;
				int lastSurface = firstSurface + visibleLeaf->numsurf;
				for (int k = firstSurface; k < lastSurface; k++) {
					visibleSurfaces[numVisibleSurfaces++] = map->getSurfaceList(k);
				}
			}
		} else {
			// No, skip some leaves
			i += (*visibilityList++) << 3;
		}
	}

	// Draw the copied surfaces
	DrawSurfaceList(visibleSurfaces, numVisibleSurfaces);
}

//
// Traverse the BSP tree to find the leaf containing visible surfaces from a specific position
//
bspleaf_t *World::FindLeaf(Camera *camera)
{
	bspleaf_t *leaf = NULL;

	// Fetch the start node
	bspnode_t *node = map->getStartNode();

	while (!leaf) {
		short nextNodeId;

		// Get a pointer to the plane which intersects the node
		plane_t *plane = map->getPlane(node->planenum);

		// Calculate distance to the intersecting plane
		float distance = CalculateDistance(plane->normal, camera->head);

		// If the camera is in front of the plane, traverse the right (front) node, otherwise traverse the left (back) node
		if (distance > plane->dist) {
			nextNodeId = node->front;
		} else {
			nextNodeId = node->back;
		}

		// If next node >= 0, traverse the node, otherwise use the inverse of the node as the index to the leaf (and we are done!)
		if (nextNodeId >= 0) {
			node = map->getNode(nextNodeId);
		} else {
			leaf = map->getLeaf(~nextNodeId);
		}
	}

	return leaf;
}

//
// Build mipmaps from the textures, create texture objects and assign mipmaps to the created objects
//
bool World::InitializeTextures(void)
{
	int numAnimTextues = 0;
	int numSkyTextures = 0;

	// Loop through all textures to count animations and sky textures
	for (int i = 0; i < map->getNumTextures(); ++i) {
		// Point to the stored mipmaps
		miptex_t* mipTexture = map->getMipTexture(i);

		// Convert name to std::string_view for safer and easier substring comparison
		std::string_view name(mipTexture->name);

		if (name.starts_with('*')) {
			numAnimTextues++;
		} else if (name.starts_with("sky")) {
			numSkyTextures++;
		}
	}
	//printf("%d animated textures, %d sky textures\n", numAnimTextues, numSkyTextures);

	// Calculate the total number of textures
	int numTotalTextures = map->getNumTextures() + numAnimTextues * (ANIM_TEX_FRAMES - 1) + numSkyTextures * 2;

	// Allocate memory for texture names and an array to store the textures we are using
	textureObjNames = new unsigned int [numTotalTextures];

	// Get unused texture names
	glGenTextures(numTotalTextures, textureObjNames);

	// Loop through all texture objects to associate them with a texture and calculate mipmaps from it
	for (int i = 0; i < map->getNumTextures(); i++) {
		// Point to the stored mipmaps
		miptex_t *mipTexture = map->getMipTexture(i);

		// NULL textures exist, don't use them!
		if (!mipTexture->name[0]) {
			continue;
		}

		int width = mipTexture->width;
		int height = mipTexture->height;

		// Allocate memory for the texture which will be created
		unsigned int *texture = new unsigned int [width * height];

		// Point to the raw 8-bit texture data
		unsigned char *rawTexture = map->getRawTexture(i);

		// Create a texture to assign with the texture object
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				texture[x + y * width] = map->palette[rawTexture[x + y * width]];
			}
		}

		// Create a new texture object
		glBindTexture(GL_TEXTURE_2D, textureObjNames[i]);
		// Create mipmaps from the created texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);

		delete[] texture;
	}

	return true;
}

//
// Calculate primitive surfaces
//
bool World::InitializeSurfaces(void)
{
	// Allocate memory for the visible surfaces array
	visibleSurfaces = new int [map->getNumSurfaceLists()];

	// Calculate max number of edges per surface
	numMaxEdgesPerSurface = 0;
	for (int i = 0; i < map->getNumSurfaces(); i++) {
		if (numMaxEdgesPerSurface < map->getNumEdges(i)) {
			numMaxEdgesPerSurface = map->getNumEdges(i);
		}
	}

	// Allocate memory for the surface primitive array
	surfacePrimitives = new primdesc_t [map->getNumSurfaces() * numMaxEdgesPerSurface];

	// Loop through all the surfaces to fetch the vertices and calculate it's texture coords
	for (int i = 0; i < map->getNumSurfaces(); i++) {
		int numEdges = map->getNumEdges(i);

		// Get a pointer to texinfo for this surface
		texinfo_t *textureInfo = map->getTextureInfo(i);
		// Get a pointer to the surface's miptextures
		miptex_t *mipTexture = map->getMipTexture(textureInfo->texid);

		// Point to a surface primitive array
		primdesc_t *primitives = &surfacePrimitives[i * numMaxEdgesPerSurface];

		for (int j = 0; j < numEdges; j++, primitives++) {
			// Get an edge id from the surface. Fetch the correct edge by using the id in the Edge List.
			// The winding is backwards!
			int edgeId = map->getEdgeList(map->getSurface(i)->firstedge + (numEdges - 1 - j));
			// Fetch one of the egde's vertex
			int vertexId = ((edgeId >= 0) ? map->getEdge(edgeId)->startVertex : map->getEdge(-edgeId)->endVertex);

			// Store the vertex in the primitive array
			vec3_t *vertex = map->getVertex(vertexId);
			primitives->v[0] = ((float *)vertex)[0];
			primitives->v[1] = ((float *)vertex)[1];
			primitives->v[2] = ((float *)vertex)[2];

			// Calculate the vertex's texture coords and store it in the primitive array
			primitives->t[0] = (CalculateDistance(textureInfo->snrm, primitives->v) + textureInfo->soff) / mipTexture->width;
			primitives->t[1] = (CalculateDistance(textureInfo->tnrm, primitives->v) + textureInfo->toff) / mipTexture->height;
		}
	}

	for (int i = 0; i < map->getNumSurfaces(); i++) {
		primdesc_t *primitives = &surfacePrimitives[numMaxEdgesPerSurface * i];
		// setup VAO and VBO for rendering a surface
		GLuint VAO, VBO;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(primdesc_t) * map->getNumEdges(i), primitives, GL_DYNAMIC_DRAW);

		GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(primdesc_t), (void*)(sizeof(float) * 2));
		glEnableVertexAttribArray(posAttrib);

		GLint texAttrib = glGetAttribLocation(shaderProgram, "texCoord");
		glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(primdesc_t), (void*)0);
		glEnableVertexAttribArray(texAttrib);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		map->surfaceVAOs[i] = VAO;
		map->surfaceVBOs[i] = VBO;

	}

	return true;
}

//
// Return the dotproduct of two vectors
//
float World::CalculateDistance(vec3_t a, vec3_t b)
{
	return (a[0] * b[0] + a[1] * b[1] + a[2] * b[2]); 
}
