#include <btBulletDynamicsCommon.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <BulletSoftBody/btDefaultSoftBodySolver.h>
#include <BulletSoftBody/btSoftBodyHelpers.h>

class PhysicsV2
{
public:

	btDefaultCollisionConfiguration* collisionConfiguration;

	btCollisionDispatcher* collisionDispatcher;
	btBroadphaseInterface* broadphaseInterface;
	btConstraintSolver* constraintSolver;

	btSoftBodySolver* softBodySolver;

	btSoftRigidDynamicsWorld* world;

public:

	void setupPhysics()
	{
		collisionConfiguration = new btSoftBodyRigidBodyCollisionConfiguration();
		collisionDispatcher = new btCollisionDispatcher(collisionConfiguration);

		broadphaseInterface = new btDbvtBroadphase();
		constraintSolver = new btSequentialImpulseConstraintSolver();

		softBodySolver = new btDefaultSoftBodySolver();

		world = new btSoftRigidDynamicsWorld(collisionDispatcher, broadphaseInterface,
			constraintSolver, collisionConfiguration, softBodySolver);

		world->setGravity(btVector3(0, -10, 0));

		//this->world = world;

	}

	void deletePhysics()
	{
		int i;

		//Remove and delete rigid bodies
		for (i = world->getNumCollisionObjects() - 1; i >= 0; i--)
		{
			//Remove collision object
			btCollisionObject* collisionObject = world->getCollisionObjectArray()[i];
			world->removeCollisionObject(collisionObject);

			//Delete the rigig body linked to the collision object
			btRigidBody* rigidBody = btRigidBody::upcast(collisionObject);

			btMotionState* motionState = rigidBody->getMotionState();
			delete motionState;

			btCollisionShape* collisionShape = rigidBody->getCollisionShape();
			delete collisionShape;

			delete rigidBody;

		}

		//Remove and delete soft bodies
		for (i = 0; i < world->getSoftBodyArray().size(); i++)
		{
			world->removeSoftBody(world->getSoftBodyArray()[i]);
			delete world->getSoftBodyArray()[i];
		}

		//Delete world
		delete collisionConfiguration;
		delete collisionDispatcher;
		delete broadphaseInterface;
		delete constraintSolver;
		//delete softBodySolver;

		delete world;

	}

	//Create world plane aside from other bodies
	btRigidBody* genWorldPlane(glm::vec3 scale, btScalar mass)
	{
		btVector3 pos = btVector3(0.0f, -3.0f, 0.0f);
		btQuaternion quat;
		quat.setEuler(0.0f, 0.0f, 0.0f);

		//btVector3 dim = btVector3(200.0f, 0.1f, 200.0f);
		btVector3 dim = btVector3(scale.x, scale.y, scale.z);

		btCollisionShape* cShape = new btBoxShape(dim);
		cShape->setMargin(0.1f);

		// We set the initial transformations
		btTransform objTransform;
		objTransform.setIdentity();
		objTransform.setRotation(quat);
		// we set the initial position (it must be equal to the position of the corresponding model of the scene)
		objTransform.setOrigin(pos);

		// if it is dynamic (mass > 0) then we calculates local inertia
		btVector3 localInertia(0.0f, 0.0f, 0.0f);
		if (mass != 0.0f)
			cShape->calculateLocalInertia(mass, localInertia);

		btDefaultMotionState* motionState = new btDefaultMotionState(objTransform);

		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, cShape, localInertia);
		rbInfo.m_friction = 0.3f;
		rbInfo.m_restitution = 0.3f;

		// we create the rigid body
		btRigidBody* body = new btRigidBody(rbInfo);

		this->world->addRigidBody(body);
		return body;

	}

	//Final versions for rigid and soft bodies
	void generateRigidBody(glm::vec3 position, glm::vec3 rotation)
	{
		//Initialize body transform
		btTransform transform;
		transform.setIdentity();
		//Set body position
		btVector3 pos(position.x, position.y, position.z);
		transform.setOrigin(pos);
		//Set body rotation
		btQuaternion rot;
		rot.setEuler(rotation.x, rotation.y, rotation.z);
		transform.setRotation(rot);


	}

	//Handle the correct spawning of the soft body
	btSoftBody* generateSoftBodyTest(
		ModelV2 model,
		float position[3],
		float rotation[3],
		float scale[3],
		float mass,
		float internalPressure
	)
	{

		//Generate the soft body
		btSoftBody* softBody = generateSoftBodyFromModel(model);

		//Initialize body transform
		btTransform transform;
		transform.setIdentity();
		//Set body position
		btVector3 pos(position[0], position[1], position[2]);
		transform.setOrigin(pos);
		//Set body rotation
		btQuaternion quat;
		quat.setEuler(rotation[0], rotation[1], rotation[2]);
		transform.setRotation(quat);
		//Set the transform
		softBody->transform(transform);


		//Set the soft body scale
		//btVector3 scaling(scale[0], scale[1], scale[2]);
		//Set the soft body scale -> Rallenta tanto la simulazione + soft solver diventa estremamente impreciso!
		//softBody->scale(scaling);

		//Set the soft body internal pressure
		softBody->setTotalMass(mass, true);
		softBody->m_cfg.kPR = internalPressure;

		//NB Setting total mass to 0 using setTotalMass makes the soft body disappear
		//To make the sotf body static you must iterate over all the nodes and set their mass to 0

		return softBody;

	}


	//Given a model generate its corresponding soft body
	btSoftBody* generateSoftBodyFromModel(ModelV2 model)
	{

		vector<btVector3> vertices = model.vertices;
		vector<GLuint> indices = model.indices;

		btSoftBody* body = new btSoftBody(
			&world->getWorldInfo(),
			vertices.size(),
			&vertices[0],
			0
		);

		for (unsigned int j = 0; j < indices.size(); j += 3)
		{

			// Create a face using the three indices at j, j+1, and j+2
			body->appendFace(indices[j], indices[j + 1], indices[j + 2]);
		}


		for (unsigned int j = 0; j < indices.size(); j += 3)
		{
			// Create links between the three nodes of the current face
			body->appendLink(indices[j], indices[j + 1], 0, true);
			body->appendLink(indices[j + 1], indices[j + 2], 0, true);
			body->appendLink(indices[j + 2], indices[j], 0, true);
		}

		// Set the pressure
		//body->m_cfg.kPR = 1.0f;

		//Test
		//Enable collision between soft bodies
		body->m_materials[0]->m_kLST = 1;
		body->m_materials[0]->m_kVST = 1;
		body->m_materials[0]->m_kAST = 1;
		body->generateBendingConstraints(2, body->m_materials[0]);
		body->m_cfg.piterations = 5;
		//body->m_cfg.viterations = 2;
		body->m_cfg.kDF = 0.5;
		body->m_cfg.collisions |= btSoftBody::fCollision::VF_SS;
		body->randomizeConstraints();
		body->getCollisionShape()->setMargin(0.075f);

		// Add the soft body to the world
		this->world->addSoftBody(body);

		return body;

	}

};