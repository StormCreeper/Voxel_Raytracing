/*
	fragmentShader.glsl
	author: Telo PHILIPPE

	Use shell texturing to create a grass effect
*/

#version 460 core

#define FLT_MAX 3.402823466e+38

const int value_flag = 0xF0000000;
const int value_mask = 0x00FFFFFF;

const int address_flag = 0x0F000000;
const int address_mask = 0x00FFFFFF;

out vec4 outColor;

in vec2 screenPos;

uniform mat4 u_viewMat;
uniform mat4 u_projMat;
uniform mat4 u_invViewMat;
uniform mat4 u_invProjMat;

uniform vec3 u_cameraPosition;

uniform float u_time;

struct Voxel {
	vec3 color;
	vec3 normal;
};

vec3 lightDir = normalize(vec3(1, 2, -0.5));

struct Ray {
	vec3 origin;
	vec3 direction;

	bool hit;
	float t;
	vec3 surfaceColor;
	vec3 surfaceNormal;
};

struct VoxelMap {
	usampler3D colorTex;
	usampler3D normalTex;
	ivec3 size;
};

uniform usampler3D u_octreeTex;
uniform int u_octreeDepth;

const int mapSize = 1 << (u_octreeDepth);

float projectToCube(vec3 ro, vec3 rd) {
	
	float tx1 = (0 - ro.x) / rd.x;
	float tx2 = (mapSize - ro.x) / rd.x;

	float ty1 = (0 - ro.y) / rd.y;
	float ty2 = (mapSize - ro.y) / rd.y;

	float tz1 = (0 - ro.z) / rd.z;
	float tz2 = (mapSize - ro.z) / rd.z;

	float tx = max(min(tx1, tx2), 0);
	float ty = max(min(ty1, ty2), 0);
	float tz = max(min(tz1, tz2), 0);

	float t = max(tx, max(ty, tz));
	
	return t;
}

const int nCells = 1 << (u_octreeDepth-1);

Voxel sampleOctree(int x, int y, int z, inout int d) {
	if(x < 0 || y < 0 || z < 0 || x >= 1 << u_octreeDepth || y >= 1 << u_octreeDepth || z >= 1 << u_octreeDepth) return Voxel(vec3(0.0, 0.0, 0.0), vec3(0));
	
	ivec3 currentCell = ivec3(0, 0, 0);
	for(d=0; d<u_octreeDepth; d++) {
		uint c = u_octreeDepth - d - 1;
		ivec3 localPos = ivec3((x & (1 << c)) >> c, (y & (1 << c)) >> c, (z & (1 << c)) >> c);
		ivec3 cellPos = currentCell * 2 + localPos;

		uint cell = texture(u_octreeTex, vec3(cellPos) / float(1 << u_octreeDepth)).r;

		if((cell & (~value_mask)) == value_flag) {
			return Voxel(vec3((cell & 0xFF0000) >> 16, (cell & 0xFF00) >> 8, cell & 0xFF) / 255.0f, vec3(0));
		} else if((cell & (~address_mask)) == address_flag) {
			int newIndex = int(cell) & address_mask;
			currentCell.x = newIndex % nCells;
			currentCell.y = (newIndex / nCells) % nCells;
			currentCell.z = newIndex / (nCells * nCells);
		} else {
			return Voxel(vec3(0.0, 0.0, 0.0), vec3(0));
		}
	}
	return Voxel(vec3(0.0, 0.0, 0.0), vec3(0));
}


vec3 getDepthColor(int depth) {
	int r = (912873911 + depth * 1239879) % 255;
	int g = (12938192 + depth * 223143223) % 255;
	int b = (123981 + depth * 498273987) % 255;

	return normalize(vec3(r, g, b) / 255.0f);
}

float voxel_traversal(vec3 orig, vec3 direction, inout vec3 normal, inout Voxel vox, inout int mapX, inout int mapY, inout int mapZ, inout float through) {
	through = 1;
	vec3 origin = orig;
	
	float t1 = max(projectToCube(origin, direction) - 0.001f, 0);
	origin += t1 * direction;

	mapX = int(floor(origin.x));
	mapY = int(floor(origin.y));
	mapZ = int(floor(origin.z));

	float sideDistX;
	float sideDistY;
	float sideDistZ;

	float deltaDX = abs(1 / direction.x);
	float deltaDY = abs(1 / direction.y);
	float deltaDZ = abs(1 / direction.z);
	float perpWallDist = -1;

	int stepX;
	int stepY;
	int stepZ;

	int hit = 0;
	int side;

	if (direction.x < 0) {
		stepX = -1;
		sideDistX = (origin.x - mapX) * deltaDX;
	} else {
		stepX = 1;
		sideDistX = (mapX + 1.0 - origin.x) * deltaDX;
	}
	if (direction.y < 0) {
		stepY = -1;
		sideDistY = (origin.y - mapY) * deltaDY;
	} else {
		stepY = 1;
		sideDistY = (mapY + 1.0 - origin.y) * deltaDY;
	}
	if (direction.z < 0) {
		stepZ = -1;
		sideDistZ = (origin.z - mapZ) * deltaDZ;
	} else {
		stepZ = 1;
		sideDistZ = (mapZ + 1.0 - origin.z) * deltaDZ;
	}

	int step = 1;

	for (int i = 0; i < 1000; i++) {
		if ((mapX >= mapSize && stepX > 0) || (mapY >= mapSize && stepY > 0) || (mapZ >= mapSize && stepZ > 0)) break;
		if ((mapX < 0 && stepX < 0) || (mapY < 0 && stepY < 0) || (mapZ < 0 && stepZ < 0)) break;

		if (sideDistX < sideDistY && sideDistX < sideDistZ) {
			sideDistX += deltaDX;
			mapX += stepX * step;
			side = 0;
		} else if(sideDistY < sideDistX && sideDistY < sideDistZ){
			sideDistY += deltaDY;
			mapY += stepY * step;
			side = 1;
		} else {
			sideDistZ += deltaDZ;
			mapZ += stepZ * step;
			side = 2;
		}

		int depth;
		Voxel block = sampleOctree(mapX, mapY, mapZ, depth);
		through *= pow(0.995f, float(depth));
		if (length(block.color) > 0) {
			if (side == 0) {
				perpWallDist = (mapX - origin.x + (1 - stepX * step) / 2) / direction.x + t1;
				normal = vec3(1, 0, 0) * -stepX;
			}
			else if (side == 1) {
				perpWallDist = (mapY - origin.y + (1 - stepY * step) / 2) / direction.y + t1;
				normal = vec3(0, 1, 0) * -stepY;
			}
			else {
				perpWallDist = (mapZ - origin.z + (1 - stepZ * step) / 2) / direction.z + t1;
				normal = vec3(0, 0, 1) * -stepZ;
			}
			vox = block;
			break;
		}
	}
	return perpWallDist;
}

void main() {
	vec2 uv = screenPos;// * 2.0f - 1.0f;

	vec4 clip = vec4(uv, -1.0, 1.0);
	vec4 eye = vec4(vec2(u_invProjMat * clip), -1.0, 0.0);
	vec3 rayDir = vec3(u_invViewMat * eye);
	vec3 rayOrigin = u_invViewMat[3].xyz;

	Ray ray = Ray(rayOrigin, rayDir, false, FLT_MAX, vec3(0), vec3(0));

	Voxel vox;
	int mapX, mapY, mapZ;

	vec3 normal;

	float through;
	float t = voxel_traversal(rayOrigin, rayDir, normal, vox, mapX, mapY, mapZ, through);
	if (t > 0) {
		ray.hit = true;
		ray.t = t;
		ray.surfaceNormal = normal;
		ray.surfaceColor = vox.color;
		vec3 vertexPos = rayOrigin + rayDir * ray.t;
		ray.surfaceNormal = normalize(vec3(mapX, mapY, mapZ) - vec3(1 << (u_octreeDepth - 1)));

		// Phong shading
		vec3 ambient = 0.1f * vox.color;
		vec3 diffuse = max(dot(ray.surfaceNormal, lightDir), 0.0f) * vox.color;
		
		vec3 reflectDir = reflect(-lightDir, ray.surfaceNormal);
		vec3 viewDir = normalize(u_cameraPosition - vertexPos);
		vec3 halfwayDir = normalize(lightDir + viewDir);
		vec3 specular = pow(max(dot(ray.surfaceNormal, halfwayDir), 0.0f), 32.0f) * vec3(1.0f);

		ray.surfaceColor = ambient + diffuse + specular;
	}
	if(ray.hit) {
		outColor = vec4(ray.surfaceColor, 1.0f);
	} else {
		outColor = vec4(rayDir, 1.0f);
	}
	outColor.r = min(outColor.r, 1.0f);
	outColor.g = min(outColor.g, 1.0f);
	outColor.b = min(outColor.b, 1.0f);
	outColor.rgb *= 1 - pow(1-through, 1.9f);
}