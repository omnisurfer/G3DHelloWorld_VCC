#include "DemoScene.h"
#include "PlayerEntity.h"
#include "G3D/Random.h"

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
			UniversalMaterial::Specification {
				lambertian = Texture::Specification { filename = "image/checker-32x32-1024x1024.png"; encoding = Color3(1.0,0.7,0.15); };
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
			CFrame::fromXYZYPRDegrees(pos.x, pos.y, pos.z, r.uniform(0, 360), r.uniform(0, 360), r.uniform(0, 360)));

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

	const int   smallFaces = 20;
	const float smallRadius = 0.5f * units::meters();

	const int   largeFaces = 50;
	const float largeRadius = 2.0f * units::meters();

	Array<CPUVertexArray::Vertex>& vertexArray = geometry->cpuVertexArray.vertex;
	Array<int>& indexArray = mesh->cpuIndexArray;

	for (int t = 0; t <= largeFaces; ++t) {

		const float   thetaDegrees = 360.0f * t / float(largeFaces);
		const CFrame& smallRingFrame = (Matrix4::yawDegrees(thetaDegrees) * Matrix4::translation(largeRadius, 0.0f, 0.0f)).approxCoordinateFrame();

		for (int p = 0; p <= smallFaces; ++p) {
			const float phi = 2.0f * pif() * p / float(smallFaces);

			CPUVertexArray::Vertex& v = vertexArray.next();
			v.position = smallRingFrame.pointToWorldSpace(Point3(cos(phi), sin(phi), 0.0f) * smallRadius);

			v.texCoord0 = Point2(4.0f * t / float(largeFaces), p / float(smallFaces));

			// Set to NaN to trigger automatic vertex normal and tangent computation
			v.normal = Vector3::nan();
			v.tangent = Vector4::nan();

			if ((t < largeFaces) && (p < smallFaces)) {
				// Create the corresponding face out of two triangles.
				// Because the texture coordinates are unique, we can't
				// wrap the geometry around and instead duplicate vertices
				// along the two seams.
				//
				// D-----C
				// |   / |
				// | /   |
				// A-----B
				const int A = (t + 0) * (smallFaces + 1) + (p + 0);
				const int B = (t + 1) * (smallFaces + 1) + (p + 0);
				const int C = (t + 1) * (smallFaces + 1) + (p + 1);
				const int D = (t + 0) * (smallFaces + 1) + (p + 1);
				indexArray.append(
					A, B, C,
					C, D, A);
			}
		} // p 
	} // t

	  // Tell the ArticulatedModel to generate bounding boxes, GPU vertex arrays,
	  // normals and tangents automatically. We already ensured correct
	  // topology, so avoid the vertex merging optimization.
	ArticulatedModel::CleanGeometrySettings geometrySettings;
	geometrySettings.allowVertexMerging = false;
	model->cleanGeometry(geometrySettings);

	/*
	Point3 pos;

	pos.x = 0.0f, pos.y = 0.0f, pos.z = 0.0f;

	Random r(1023, false);

	const shared_ptr<VisibleEntity>& visibleEntityX =
		VisibleEntity::create
		(format("torus%02d", 1),
			this,
			m_modelTable[modelName].resolve(),
			CFrame::fromXYZYPRDegrees(pos.x, pos.y, pos.z, r.uniform(0, 360), r.uniform(0, 360), r.uniform(0, 360)));

	// Don't serialize generated objects
	visibleEntityX->setShouldBeSaved(false);

	insert(visibleEntityX);
	*/

	return model;
}
