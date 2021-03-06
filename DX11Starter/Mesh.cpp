//-----------------------------
// Include statements.
//-----------------------------

#include <fstream>
#include <vector>
#include "Mesh.h"

//-----------------------------
// Namespace statements.
//-----------------------------
using namespace DirectX;

// Constructor.

/// <summary>
/// Constructor for Mesh objects. Mesh constructor contains Vertex
/// array and Index array for the appropriate DirectX 11 data buffer.
/// </summary>
/// <param name="vertices">Vertex array.</param>
/// <param name="vertexCount">Number of vertices in the array.</param>
/// <param name="indices">Index array.</param>
/// <param name="indexCount">Number of indices in the array.</param>
/// <param name="device">Reference to the Direct3D Device.</param>
Mesh::Mesh(
	Vertex* vertices, // Vertex data.
	unsigned int vertexCount, // Number of vertices.
	unsigned int* indices, // Indices for Index Buffer.
	unsigned int indexCount, // Number of indices.
	ID3D11Device* device) // Direct3D Device pointer.
{
	// Initialize fields.
	vertexBuffer = 0;
	indexBuffer = 0;

	// Assign index count.
	this->indexCount = indexCount;

	// Assign values.
	CreateVertexBuffer(vertices, vertexCount, device);
	CreateIndexBuffer(indices, indexCount, device);
}

/// <summary>
/// Initializes a new instance of the <see cref="Mesh"/> class.
/// </summary>
/// <param name="filename">The filename to load a mesh from.</param>
Mesh::Mesh(char* filename,
	ID3D11Device* device)
	: indexCount(0)
{
	// Initialize fields.
	vertexBuffer = 0;
	indexBuffer = 0;
	
	// File input object
	std::ifstream obj(filename);

	// Check for successful open
	if (!obj.is_open())
		return;

	// Variables used while reading the file
	std::vector<XMFLOAT3> positions;     // Positions from the file
	std::vector<XMFLOAT3> normals;       // Normals from the file
	std::vector<XMFLOAT2> uvs;           // UVs from the file
	std::vector<Vertex> verts;           // Verts we're assembling
	std::vector<UINT> indices;           // Indices of these verts
	unsigned int vertCounter = 0;        // Count of vertices/indices
	char chars[100];                     // String for line reading

	// Still have data left?
	while (obj.good())
	{
		// Get the line (100 characters should be more than enough)
		obj.getline(chars, 100);

		// Check the type of line
		if (chars[0] == 'v' && chars[1] == 'n')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 norm;
			sscanf_s(
				chars,
				"vn %f %f %f",
				&norm.x, &norm.y, &norm.z);

			// Add to the list of normals
			normals.push_back(norm);
		}
		else if (chars[0] == 'v' && chars[1] == 't')
		{
			// Read the 2 numbers directly into an XMFLOAT2
			XMFLOAT2 uv;
			sscanf_s(
				chars,
				"vt %f %f",
				&uv.x, &uv.y);

			// Add to the list of uv's
			uvs.push_back(uv);
		}
		else if (chars[0] == 'v')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 pos;
			sscanf_s(
				chars,
				"v %f %f %f",
				&pos.x, &pos.y, &pos.z);

			// Add to the positions
			positions.push_back(pos);
		}
		else if (chars[0] == 'f')
		{
			// Read the face indices into an array
			unsigned int i[12];
			int facesRead = sscanf_s(
				chars,
				"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
				&i[0], &i[1], &i[2],
				&i[3], &i[4], &i[5],
				&i[6], &i[7], &i[8],
				&i[9], &i[10], &i[11]);

			// - Create the verts by looking up
			//    corresponding data from vectors
			// - OBJ File indices are 1-based, so
			//    they need to be adjusted
			Vertex v1;
			v1.Position = positions[i[0] - 1];
			v1.UV = uvs[i[1] - 1];
			v1.Normal = normals[i[2] - 1];

			Vertex v2;
			v2.Position = positions[i[3] - 1];
			v2.UV = uvs[i[4] - 1];
			v2.Normal = normals[i[5] - 1];

			Vertex v3;
			v3.Position = positions[i[6] - 1];
			v3.UV = uvs[i[7] - 1];
			v3.Normal = normals[i[8] - 1];

			// The model is most likely in a right-handed space,
			// especially if it came from Maya.  We want to convert
			// to a left-handed space for DirectX.  This means we 
			// need to:
			//  - Invert the Z position
			//  - Invert the normal's Z
			//  - Flip the winding order
			// We also need to flip the UV coordinate since DirectX
			// defines (0,0) as the top left of the texture, and many
			// 3D modeling packages use the bottom left as (0,0)

			// Flip the UV's since they're probably "upside down"
			v1.UV.y = 1.0f - v1.UV.y;
			v2.UV.y = 1.0f - v2.UV.y;
			v3.UV.y = 1.0f - v3.UV.y;

			// Flip Z (LH vs. RH)
			v1.Position.z *= -1.0f;
			v2.Position.z *= -1.0f;
			v3.Position.z *= -1.0f;

			// Flip normal Z
			v1.Normal.z *= -1.0f;
			v2.Normal.z *= -1.0f;
			v3.Normal.z *= -1.0f;

			// Add the verts to the vector (flipping the winding order)
			verts.push_back(v1);
			verts.push_back(v3);
			verts.push_back(v2);

			// Add three more indices
			indices.push_back(vertCounter); vertCounter += 1;
			indices.push_back(vertCounter); vertCounter += 1;
			indices.push_back(vertCounter); vertCounter += 1;

			// Was there a 4th face?
			if (facesRead == 12)
			{
				// Make the last vertex
				Vertex v4;
				v4.Position = positions[i[9] - 1];
				v4.UV = uvs[i[10] - 1];
				v4.Normal = normals[i[11] - 1];

				// Flip the UV, Z pos and normal
				v4.UV.y = 1.0f - v4.UV.y;
				v4.Position.z *= -1.0f;
				v4.Normal.z *= -1.0f;

				// Add a whole triangle (flipping the winding order)
				verts.push_back(v1);
				verts.push_back(v4);
				verts.push_back(v3);

				// Add three more indices
				indices.push_back(vertCounter); vertCounter += 1;
				indices.push_back(vertCounter); vertCounter += 1;
				indices.push_back(vertCounter); vertCounter += 1;
			}
		}
	}

	// Close the file and create the actual buffers
	obj.close();


	// - At this point, "verts" is a vector of Vertex structs, and can be used
	//    directly to create a vertex buffer:  &verts[0] is the address of the first vert
	//
	// - The vector "indices" is similar. It's a vector of unsigned ints and
	//    can be used directly for the index buffer: &indices[0] is the address of the first int
	//
	// - "vertCounter" is BOTH the number of vertices and the number of indices
	// - Yes, the indices are a bit redundant here (one per vertex).  Could you skip using
	//    an index buffer in this case?  Sure!  Though, if your mesh class assumes you have
	//    one, you'll need to write some extra code to handle cases when you don't.

	// Call the helper methods.
	CreateVertexBuffer(&verts[0], vertCounter, device);
	CreateIndexBuffer(&indices[0], vertCounter, device);

	// Assign index count.
	this->indexCount = vertCounter;


}

// Destructor.

/// <summary>
/// Destructor releases DirectX resources used by Mesh and
/// deletes outstanding objects to prevent memory leaks.
/// </summary>
Mesh::~Mesh()
{
	// Release resources.
	if (vertexBuffer) { vertexBuffer->Release(); }
	if (indexBuffer) { indexBuffer->Release(); }

	// Set to nullptr.
	vertexBuffer = nullptr;
	indexBuffer = nullptr;
}

// Accessor functions.

/// <summary>
/// Returns pointer to the vertex buffer.
/// </summary>
/// <returns>Returns Vertex Buffer pointer.</returns>
ID3D11Buffer* Mesh::GetVertexBuffer() const {
	return vertexBuffer;
}

/// <summary>
/// Returns pointer to the index buffer.
/// </summary>
/// <returns>Returns Index Buffer pointer.</returns>
ID3D11Buffer* Mesh::GetIndexBuffer() const {
	return indexBuffer;
}

/// <summary>
/// Return number of indices in index buffer.
/// </summary>
/// <returns>Return index count.</returns>
unsigned int Mesh::GetIndexCount() const {
	return indexCount;
}

// Helper functions.

/// <summary>
/// Creates the vertex buffer and assigns it to the member.
/// </summary>
/// <param name="vertices">Vertex array.</param>
/// <param name="count">Size of array.</param>
/// <param name="device">Direct3D Device pointer.</param>
void Mesh::CreateVertexBuffer(
	Vertex* vertices,
	unsigned int count,
	ID3D11Device* device) {

	// Create the buffer description.
	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_IMMUTABLE; // no changing after it reaches GPU.
	desc.ByteWidth = sizeof(Vertex) * count; // bytewidth = number of verts * size of single vert.
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	// Create the proper struct for vertex data.
	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem = vertices;

	// Create the actual buffer using the device. Pass items in by reference.
	device->CreateBuffer(&desc, &initialData, &vertexBuffer);
}

/// <summary>
/// Creates the index buffer and assigns it to the member.
/// </summary>
/// <param name="indices">Index array.</param>
/// <param name="count">Size of array.</param>
/// <param name="device">Direct3D Device pointer.</param>
void Mesh::CreateIndexBuffer(
	unsigned int* indices,
	unsigned int count,
	ID3D11Device* device) {


	// Create the buffer description.
	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_IMMUTABLE; // no changing after it reaches GPU.
	desc.ByteWidth = sizeof(int) * count; // bytewidth = number of indices * size of single index.
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	// Create the proper struct for index data.
	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem = indices;

	// Create the actual buffer using the device. Pass items in by reference.
	device->CreateBuffer(&desc, &initialData, &indexBuffer);
}