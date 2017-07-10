#include "DemoScene.h"
#include "PlayerEntity.h"
#include "G3D/Random.h"
#include <iostream>
#include <fstream>

shared_ptr<DemoScene> DemoScene::create(const shared_ptr<AmbientOcclusion>& ao) {
	return shared_ptr<DemoScene>(new DemoScene(ao));
}


void DemoScene::spawnAsteroids() {

	// An example of how to spawn Entitys at runtime

	Random r(1023, false);
	for (int i = 0; i < 10; ++i) {
		//const String& modelName = format("asteroid%dModel", r.integer(0, 4));
		const String& modelName = "cubeModel";

		// Find a random position that is not too close to the space ship
		Point3 pos;
		while (pos.length() < 15) {
			for (int a = 0; a < 3; ++a) {
				pos[a] = r.uniform(-50.0f, 50.0f);
			}
		}

		/*
		const shared_ptr<VisibleEntity>& v =
			VisibleEntity::create
			(format("asteroid%02d", i),
			 this,
			 m_modelTable[modelName].resolve(),
			 CFrame::fromXYZYPRDegrees(pos.x, pos.y, pos.z, r.uniform(0, 360), r.uniform(0, 360), r.uniform(0, 360)));
		*/
		const shared_ptr<VisibleEntity>& v =
			VisibleEntity::create
			(format("cube%02d", i),
				this,
				m_modelTable[modelName].resolve(),
				CFrame::fromXYZYPRDegrees(pos.x, pos.y, pos.z, r.uniform(0, 360), r.uniform(0, 360), r.uniform(0, 360)));

		// Don't serialize generated objects
		v->setShouldBeSaved(false);

		insert(v);
	}
}

void DemoScene::spawnCubes()
{
	//look at proceduralGeometry example under G3D10->samples
	const shared_ptr<ArticulatedModel>& model = ArticulatedModel::createEmpty("cubeModel");

	ArticulatedModel::Part* part = model->addPart("root");
	ArticulatedModel::Geometry* geometry = model->addGeometry("geom");
	ArticulatedModel::Mesh*	mesh = model->addMesh("mesh", part, geometry);

	mesh->material = UniversalMaterial::create(
		PARSE_ANY(
			UniversalMaterial::Specification{
				lambertian = Texture::Specification { filename = "image/checker-32x32-1024x1024.png"; encoding = Color3(1.0,0.7,0.15); };
	//lambertian = Texture::Specification{encoding = Color3(1.0,0.7,0.15); };
	glossy = Color4(Color3(0.1), 0.2);
	}
		)
	);

	const float cubeSideLength = 2.0f * units::meters();

	Array<CPUVertexArray::Vertex>& vertexArray = geometry->cpuVertexArray.vertex;
	Array<int>& indexArray = mesh->cpuIndexArray;

	CPUVertexArray::Vertex& cpuVertex = vertexArray.next();
	cpuVertex.position = Point3(0.0f, 0.0f, 0.0f);

	cpuVertex.texCoord0 = Point2(0.0f, 0.0f);

	Point3 pos;

	pos.x = 0.0f;
	pos.y = 0.0f;
	pos.z = 0.0f;

	Random r(1023, false);

	const shared_ptr<VisibleEntity>& visibleEntity =
		VisibleEntity::create
		(format("cubeX%02d", 1),
			this,
			m_modelTable["cubeModel"].resolve(),
			CFrame::fromXYZYPRDegrees(pos.x, pos.y, pos.z, 0, 0, 0));

	// Don't serialize generated objects
	visibleEntity->setShouldBeSaved(false);

	insert(visibleEntity);
}

shared_ptr<Model> DemoScene::createTorusModel() {

	const String modelName = "torusModel";

	const shared_ptr<ArticulatedModel>& model = ArticulatedModel::createEmpty(modelName);

	ArticulatedModel::Part*     part = model->addPart("root");
	ArticulatedModel::Geometry* geometry = model->addGeometry("geom");
	ArticulatedModel::Mesh*     mesh = model->addMesh("mesh", part, geometry);

	// Assign a material	
	mesh->material = UniversalMaterial::create(
		PARSE_ANY(
			UniversalMaterial::Specification{
		lambertian = Texture::Specification{
		filename = "image/checker-32x32-1024x1024.png";
	// Orange
	encoding = Color3(1.0, 0.7, 0.15);
	};

	glossy = Color4(Color3(0.01), 0.2);
	}));	

	// Create the vertices and faces in the following unwrapped pattern:
	//     ___________
	//    |  /|  /|  /|   
	//    |/__|/__|/__|
	// ^  |  /|  /|  /|
	// |  |/__|/__|/__|
	// p    
	//    t ->

	//p = number of small faces that form the skin of the circle swept about the axis perpendicular to the hole.	
	const int   smallFaces = 4;//20;	
	const float smallRadius = 5.0f * units::meters();
	
	//t = number of large faces that form the circumference of the toroid from the center "hole"
	const int   largeFaces = 4;
	const float largeRadius = 20.0f * units::meters();

	Array<CPUVertexArray::Vertex>& vertexArray = geometry->cpuVertexArray.vertex;
	Array<int>& indexArray = mesh->cpuIndexArray;	
	
	//redirect console output to file
	//https://stackoverflow.com/questions/191842/how-do-i-get-console-output-in-c-with-a-windows-program

	std::ofstream file;
	file.open("cout.txt");
	std::streambuf* sbuf = std::cout.rdbuf();

	//file << "TEST WRITE";
		
	for (int t = 0; t <= largeFaces; ++t) {

		//map number of faces to a rotation around a cirlce given the large radius of the torus. This is where the small ring will be rendered (its frame)
		const float   thetaDegrees = 360.0f * t / float(largeFaces);		
		const CFrame& smallRingFrame = (Matrix4::yawDegrees(thetaDegrees) * Matrix4::translation(largeRadius, 0.0f, 0.0f)).approxCoordinateFrame();

		//printf("CFrame Rotation, thetaDegrees=%f\n", thetaDegrees);

		file << "CFrame Rotation, thetaDegrees=" << std::fixed << thetaDegrees << std::endl;

		for (int p = 0; p <= smallFaces; ++p) {
			//map number of faces to a rotation around the center of the circle that will be swept to create the torus.
			const float phi = 2.0f * pif() * p / float(smallFaces);			

			CPUVertexArray::Vertex& v = vertexArray.next();
			//vertex position offset relative to a point in world space defined by camera???
			//printf("vertexArray - rotating points, cos(phi)=%f, sin(phi)=%f\n", cos(phi), sin(phi));

			file << "vertexArray - rotating points, cos(phi)=" << std::fixed << cos(phi) << ", sin(phi)=" << std::fixed << sin(phi) << std::endl;

			v.position = smallRingFrame.pointToWorldSpace(Point3(cos(phi), sin(phi), 0.0f) * smallRadius);
			//v.position = smallRingFrame.pointToWorldSpace(Point3(0.0f, 0.0f, 0.0f) * smallRadius);

			//printf("vertexArray - creating vertices, v.x=%f, v.y=%f, v.z=%f\n", v.position.x, v.position.y, v.position.z);		

			file << "vertexArray - creating vertices, v.x=" << std::fixed << v.position.x << ", v.y=" << std::fixed << v.position.y << ", v.z=" << std::fixed << v.position.z << std::endl;

			//texture coordinate on the face of the vertex? not sure what 4.0f is about
			v.texCoord0 = Point2(4.0f * t / float(largeFaces), p / float(smallFaces));

			//printf("texCoord0 - creating texture coordintes, v.texCoord0x=%f, v.texCoord0y=%f\n", v.texCoord0.x, v.texCoord0.y);

			file << "texCoord0 - creating texture coordintes, v.texCoord0x=" << std::fixed << v.texCoord0.x << ", v.texCoord0y=" << std::fixed << v.texCoord0.y << std::endl;

			// Set to NaN to trigger automatic vertex normal and tangent computation
			v.normal = Vector3::nan();
			v.tangent = Vector4::nan();
			
			int A = 0;
			int B = 0;
			int C = 0;
			int D = 0;

			if ((t < largeFaces) && (p < smallFaces)) {
				// Create the corresponding face out of two triangles.
				// Because the texture coordinates are unique, we can't
				// wrap the geometry around and instead duplicate vertices
				// along the two seams.
				//
				// ^	D-----C
				// |	|   / |
				// p	| /   |
				//		A-----B
				// t ->
				int constant = smallFaces + 1;

				A = (t + 0) * (constant) + (p + 0);
				D = (t + 0) * (constant) + (p + 1);

				C = (t + 1) * (constant) + (p + 1);
				B = (t + 1) * (constant) + (p + 0);								
				//indexArray.append(
					//A, B, C,
					//C, D, A);
				indexArray.append(
					A, B, C,
					C, D, A);
				//printf("indexArray - creating triangles, A=%i, B=%i, C=%i, C=%i, D=%i, A=%i\n", A, B, C, C, D, A);
				file << "indexArray - creating triangles, A=" << std::fixed << A << ", B=" << std::fixed << B << ", C=" << std::fixed << C << ", C=" << std::fixed << C << ", D=" << std::fixed << D << ", A=" << std::fixed << A << std::endl;
			}			
			//printf("loopInfot, t=%i, p=%i\n", t, p);
			file << "loopInfot, t=" << std::fixed << t << ", p=" << std::fixed << p << std::endl;
		} // p 
	} // t	
	file.close();

	  // Tell the ArticulatedModel to generate bounding boxes, GPU vertex arrays,
	  // normals and tangents automatically. We already ensured correct
	  // topology, so avoid the vertex merging optimization.
	ArticulatedModel::CleanGeometrySettings geometrySettings;
	geometrySettings.allowVertexMerging = false;
	model->cleanGeometry(geometrySettings);

	return model;
}

shared_ptr<Model> DemoScene::createCubeModel() {

	const String modelName = "cubeModel";

	const shared_ptr<ArticulatedModel>& model = ArticulatedModel::createEmpty(modelName);

	ArticulatedModel::Part*     part = model->addPart("root");
	ArticulatedModel::Geometry* geometry = model->addGeometry("geom");
	ArticulatedModel::Mesh*     mesh = model->addMesh("mesh", part, geometry);

	// Assign a material	
	mesh->material = UniversalMaterial::create(
		PARSE_ANY(
		UniversalMaterial::Specification{
			lambertian = Texture::Specification{
			filename = "image/checker-32x32-1024x1024.png";
			// Orange
			encoding = Color3(1.0, 0.7, 0.15);
		};
		glossy = Color4(Color3(0.01), 0.2);
	}));

	// Create a cube with six faces
	Array<CPUVertexArray::Vertex>& vertexArray = geometry->cpuVertexArray.vertex;
	Array<int>& indexArray = mesh->cpuIndexArray;
	
	CFrame& smallRingFrame = (Matrix4::yawDegrees(0.0f) * Matrix4::translation(0.0f, 0.0f, 0.0f)).approxCoordinateFrame();
	//v0
	CPUVertexArray::Vertex& a = vertexArray.next();

	//wchar_t debugOutput[200] = { 0 };
	//swprintf(debugOutput, _countof(debugOutput), L"v address %04X\n", &v);
	//OutputDebugString(debugOutput);

	a.position = smallRingFrame.pointToWorldSpace(Point3(0.0f, 0.0f, 0.0f));

	a.texCoord0 = Point2(0.0f, 0.0f);
		
	a.normal = Vector3::nan();
	a.tangent = Vector4::nan();

	//v1
	CPUVertexArray::Vertex& b = vertexArray.next();

	b.position = smallRingFrame.pointToWorldSpace(Point3(10.0f, 0.0f, 0.0f));

	b.texCoord0 = Point2(0.0f, 0.25f);

	b.normal = Vector3::nan();
	b.tangent = Vector4::nan();

	//v2
	CPUVertexArray::Vertex& c = vertexArray.next();

	c.position = smallRingFrame.pointToWorldSpace(Point3(10.0f, 0.0f, 10.0f));

	c.texCoord0 = Point2(0.0f, 0.50f);

	c.normal = Vector3::nan();
	c.tangent = Vector4::nan();

	//v3
	CPUVertexArray::Vertex& d = vertexArray.next();

	d.position = smallRingFrame.pointToWorldSpace(Point3(0.0f, 0.0f, 10.0f));

	d.texCoord0 = Point2(0.0f, 0.75f);

	d.normal = Vector3::nan();
	d.tangent = Vector4::nan();

	//v4 - repeats v0 so that textures can wrap properly
	CPUVertexArray::Vertex& e = vertexArray.next();

	e.position = smallRingFrame.pointToWorldSpace(Point3(0.0f, 0.0f, 0.0f));

	e.texCoord0 = Point2(0.0f, 1.0f);

	e.normal = Vector3::nan();
	e.tangent = Vector4::nan();

	//v5
	CPUVertexArray::Vertex& f = vertexArray.next();

	f.position = smallRingFrame.pointToWorldSpace(Point3(0.0f, 10.0f, 0.0f));

	f.texCoord0 = Point2(1.0f, 0.0f);

	f.normal = Vector3::nan();
	f.tangent = Vector4::nan();
	
	//v6
	CPUVertexArray::Vertex& g = vertexArray.next();

	g.position = smallRingFrame.pointToWorldSpace(Point3(10.0f, 10.0f, 0.0f));

	g.texCoord0 = Point2(1.0f, 0.25f);

	g.normal = Vector3::nan();
	g.tangent = Vector4::nan();

	//v7
	CPUVertexArray::Vertex& h = vertexArray.next();

	h.position = smallRingFrame.pointToWorldSpace(Point3(10.0f, 10.0f, 10.0f));

	h.texCoord0 = Point2(1.0f, 0.50f);

	h.normal = Vector3::nan();
	h.tangent = Vector4::nan();

	//v8
	CPUVertexArray::Vertex& j = vertexArray.next();

	j.position = smallRingFrame.pointToWorldSpace(Point3(0.0f, 10.0f, 10.0f));

	j.texCoord0 = Point2(1.0f, 0.75f);

	j.normal = Vector3::nan();
	j.tangent = Vector4::nan();	

	//v9 - repeats v5 so that texture can wrap properly
	CPUVertexArray::Vertex& k = vertexArray.next();

	k.position = smallRingFrame.pointToWorldSpace(Point3(0.0f, 10.0f, 10.0f));

	k.texCoord0 = Point2(1.0f, 1.0f);

	k.normal = Vector3::nan();
	k.tangent = Vector4::nan();

	//Final geometry setup

	//	Side 1 ("front")
	//   v5_____v6
	//	  |	    |
	//	  |     |
	//   v0_____v1

	indexArray.append(1, 0, 6, 5, 6, 0);

	/*
	indexArray.append(1, 0, 4);
	indexArray.append(3, 2, 4);

	indexArray.append(2, 1, 4);
	indexArray.append(0, 3, 4);
	indexArray.append(0, 1, 2);
	*/
	
	//	Side 2 ("right")
	//   v6_____v7
	//	  |	    |
	//	  |     |
	//   v1_____v2

	indexArray.append(2, 1, 7, 6, 7, 1);

	//	Side 3 ("back", oposite side 1)
	//   v7_____v8
	//	  |	    |
	//	  |     |
	//   v2_____v3	
	
	indexArray.append(3, 2, 8, 7, 8, 2);

	//	Side 4 ("left", oposite side 2)
	//   v8_____v5
	//	  |	    |
	//	  |     |
	//   v3_____v0

	indexArray.append(0, 3, 5, 8, 5, 3);

	//	Side 5 ("top")
	//   v8_____v7
	//	  |	    |
	//	  |     |
	//   v5_____v6
	
	//indexArray.append()

	//	Side 6 ("bottom")
	//   v3_____v2
	//	  |	    |
	//	  |     |
	//   v0_____v1

	//indexArray.append();	

	ArticulatedModel::CleanGeometrySettings geometrySettings;
	geometrySettings.allowVertexMerging = false;
	model->cleanGeometry(geometrySettings);
	
	return model;
}

shared_ptr<Model> DemoScene::createCubeModel_Flat() {

	const String modelName = "cubeModel";

	const shared_ptr<ArticulatedModel>& model = ArticulatedModel::createEmpty(modelName);

	ArticulatedModel::Part*     part = model->addPart("root");
	ArticulatedModel::Geometry* geometry = model->addGeometry("geom");
	ArticulatedModel::Mesh*     mesh = model->addMesh("mesh", part, geometry);

	// Assign a material	
	mesh->material = UniversalMaterial::create(
		PARSE_ANY(
			UniversalMaterial::Specification{
		lambertian = Texture::Specification{
		filename = "image/checker-32x32-1024x1024.png";
	// Orange
	encoding = Color3(1.0, 0.7, 0.15);
	};
	glossy = Color4(Color3(0.01), 0.2);
	}));

	// Create a cube with six faces
	Array<CPUVertexArray::Vertex>& vertexArray = geometry->cpuVertexArray.vertex;
	Array<int>& indexArray = mesh->cpuIndexArray;

	CFrame& smallRingFrame = (Matrix4::yawDegrees(0.0f) * Matrix4::translation(0.0f, 0.0f, 0.0f)).approxCoordinateFrame();
	//v0
	CPUVertexArray::Vertex& a = vertexArray.next();

	//wchar_t debugOutput[200] = { 0 };
	//swprintf(debugOutput, _countof(debugOutput), L"v address %04X\n", &v);
	//OutputDebugString(debugOutput);

	a.position = smallRingFrame.pointToWorldSpace(Point3(0.0f, 0.0f, 0.0f));

	a.texCoord0 = Point2(0.0f, 0.0f);

	a.normal = Vector3::nan();
	a.tangent = Vector4::nan();

	//v1
	CPUVertexArray::Vertex& b = vertexArray.next();

	b.position = smallRingFrame.pointToWorldSpace(Point3(10.0f, 0.0f, 0.0f));

	b.texCoord0 = Point2(0.0f, 0.33f);

	b.normal = Vector3::nan();
	b.tangent = Vector4::nan();

	//v2
	CPUVertexArray::Vertex& c = vertexArray.next();

	c.position = smallRingFrame.pointToWorldSpace(Point3(20.0f, 0.0f, 0.0f));

	c.texCoord0 = Point2(0.0f, 0.67f);

	c.normal = Vector3::nan();
	c.tangent = Vector4::nan();

	//v3
	CPUVertexArray::Vertex& d = vertexArray.next();

	d.position = smallRingFrame.pointToWorldSpace(Point3(30.0f, 0.0f, 0.0f));

	d.texCoord0 = Point2(0.0f, 1.0f);

	d.normal = Vector3::nan();
	d.tangent = Vector4::nan();

	//v4
	CPUVertexArray::Vertex& e = vertexArray.next();

	e.position = smallRingFrame.pointToWorldSpace(Point3(0.0f, 10.0f, 0.0f));

	e.texCoord0 = Point2(1.0f, 0.0f);

	e.normal = Vector3::nan();
	e.tangent = Vector4::nan();

	//v5
	CPUVertexArray::Vertex& f = vertexArray.next();

	f.position = smallRingFrame.pointToWorldSpace(Point3(10.0f, 10.0f, 0.0f));

	f.texCoord0 = Point2(1.0f, 0.33f);

	f.normal = Vector3::nan();
	f.tangent = Vector4::nan();

	//v6
	CPUVertexArray::Vertex& g = vertexArray.next();

	g.position = smallRingFrame.pointToWorldSpace(Point3(20.0f, 10.0f, 0.0f));

	g.texCoord0 = Point2(1.0f, 0.67f);

	g.normal = Vector3::nan();
	g.tangent = Vector4::nan();

	//v7
	CPUVertexArray::Vertex& h = vertexArray.next();

	h.position = smallRingFrame.pointToWorldSpace(Point3(30.0f, 10.0f, 0.0f));

	h.texCoord0 = Point2(1.0f, 1.0f);

	h.normal = Vector3::nan();
	h.tangent = Vector4::nan();

	//Final geometry setup

	//	Side 1 ("front")
	//   v4_____v5
	//	  |	    |
	//	  |     |
	//   v0_____v1

	//texture OK	
	indexArray.append(1, 0, 5, 4, 5, 0);
	//indexArray.append(1, 0, 5);
	//indexArray.append(4, 5, 0);

	/*
	indexArray.append(1, 0, 4);
	indexArray.append(3, 2, 4);

	indexArray.append(2, 1, 4);
	indexArray.append(0, 3, 4);
	indexArray.append(0, 1, 2);
	*/

	//	Side 2 ("right")
	//   v5_____v6
	//	  |	    |
	//	  |     |
	//   v1_____v2

	//texture missing	
	indexArray.append(2, 1, 6, 5, 6, 1);
	//indexArray.append(2, 1, 6);
	//indexArray.append(5, 6, 1);

	//	Side 3 ("back", oposite side 1)
	//   v6_____v7
	//	  |	    |
	//	  |     |
	//   v2_____v3	

	//texture OK	
	indexArray.append(3, 2, 7, 6, 7, 2);
	//indexArray.append(3, 2, 7);
	//indexArray.append(6, 7, 2);

	//	Side 4 ("left", oposite side 2)
	//   v7_____v4
	//	  |	    |
	//	  |     |
	//   v3_____v0

	//texture missing	
	indexArray.append(0, 3, 4, 7, 4, 3);
	//indexArray.append(0, 3, 4);
	//indexArray.append(7, 4, 3);

	//	Side 5 ("top")
	//   v7_____v6
	//	  |	    |
	//	  |     |
	//   v4_____v5

	//indexArray.append(3, 2, 6, 6, 7, 3);
	//indexArray.append(3, 2, 6, 6, 7, 3);

	//	Side 6 ("bottom")
	//   v3_____v2
	//	  |	    |
	//	  |     |
	//   v0_____v1

	//indexArray.append(1, 0, 5, 4, 5, 0);
	//indexArray.append(1, 0, 5, 4, 5, 0);

	ArticulatedModel::CleanGeometrySettings geometrySettings;
	geometrySettings.allowVertexMerging = false;
	model->cleanGeometry(geometrySettings);

	return model;
}

shared_ptr<Model> DemoScene::createCubeModel_Parametric()
{
	const String modelName = "cubeModel";

	const shared_ptr<ArticulatedModel>& model = ArticulatedModel::createEmpty(modelName);

	ArticulatedModel::Part*     part = model->addPart("root");
	ArticulatedModel::Geometry* geometry = model->addGeometry("geom");
	ArticulatedModel::Mesh*     mesh = model->addMesh("mesh", part, geometry);

	// Assign a material	
	mesh->material = UniversalMaterial::create(
		PARSE_ANY(
			UniversalMaterial::Specification{
		lambertian = Texture::Specification{
		//filename = "image/checker-32x32-1024x1024.png";
		// Orange
		encoding = Color3(1.0, 0.7, 0.15);
	};

	glossy = Color4(Color3(0.01), 0.2);
	}));

	// Create a cube with six faces
	Array<CPUVertexArray::Vertex>& vertexArray = geometry->cpuVertexArray.vertex;
	Array<int>& indexArray = mesh->cpuIndexArray;

	float lengthOfSide = 10.0f;
	int numberOfSideFaces = 4;
	int numberOfTopFaces = 1;
	int numberOfBottomFaces = 1;

	//Create side faces
	for (int side = 0; side <= 4; ++side)
	{
		const float	thetaDegrees = 360.0f * side / float(numberOfSideFaces);
		CFrame&		cubeFaceFrame = (Matrix4::yawDegrees(thetaDegrees) * Matrix4::translation(lengthOfSide, 0.0f, 0.0f)).approxCoordinateFrame();
		
		//Create vertices that make up a face
		for (int vertex = 0; vertex <= 4; ++vertex)
		{
			const float phi = 2.0f * pif() * vertex / float(numberOfSideFaces);

			CPUVertexArray::Vertex& v = vertexArray.next();

			v.position = cubeFaceFrame.pointToWorldSpace(Point3((0.0f + (vertex * lengthOfSide)) * cos(phi), (0.0f + (side * lengthOfSide)) * sin(phi), 0.0f));

			//v.texCoord0 = Point2(0.0f, 0.0f);

			v.normal = Vector3::nan();
			v.tangent = Vector4::nan();

			if ((side < numberOfSideFaces) && (vertex < 4))
			{
				int A = vertex + 0;
				int B = vertex + 1;
				int C = vertex + 6;
				int D = vertex + 5;

				indexArray.append(A, B, C, C, D, A);
			}
		}
	}

	ArticulatedModel::CleanGeometrySettings geometrySettings;
	geometrySettings.allowVertexMerging = false;
	model->cleanGeometry(geometrySettings);

	return model;
}

