
#include "PhysicsClientExample.h"

#include "../CommonInterfaces/CommonMultiBodyBase.h"

#include "SharedMemoryCommon.h"
#include "../CommonInterfaces/CommonParameterInterface.h"

#include "PhysicsClient.h"
#include "SharedMemoryCommands.h"

struct MyMotorInfo2
{
	btScalar m_velTarget;
	btScalar m_maxForce;
	int		m_uIndex;
};

#define MAX_NUM_MOTORS 128

class PhysicsClientExample : public SharedMemoryCommon
{
protected:
	PhysicsClientSharedMemory	m_physicsClient;

	
	bool m_wantsTermination;
    btAlignedObjectArray<SharedMemoryCommand> m_userCommandRequests;

	void	createButton(const char* name, int id, bool isTrigger );

	void createButtons();
	
public:
    
    //@todo, add accessor methods
	MyMotorInfo2 m_motorTargetVelocities[MAX_NUM_MOTORS];
	int m_numMotors;

    
	PhysicsClientExample(GUIHelperInterface* helper);
	virtual ~PhysicsClientExample();
    
	virtual void	initPhysics();
    
	virtual void	stepSimulation(float deltaTime);
    
	virtual void resetCamera()
	{
				float dist = 5;
		float pitch = 50;
		float yaw = 35;
		float targetPos[3]={0,0,0};//-3,2.8,-2.5};
		m_guiHelper->resetCamera(dist,pitch,yaw,targetPos[0],targetPos[1],targetPos[2]);

	}
    
    virtual bool wantsTermination()
    {
        return m_wantsTermination;
    }
    
    virtual bool isConnected()
    {
        return m_physicsClient.isConnected();
    }
    
	void enqueueCommand(const SharedMemoryCommand& orgCommand);
	
	virtual void    exitPhysics(){};
	virtual void	renderScene()
	{
		int numLines = m_physicsClient.getNumDebugLines();
		const btVector3* fromLines = m_physicsClient.getDebugLinesFrom();
		const btVector3* toLines = m_physicsClient.getDebugLinesTo();
		const btVector3* colorLines = m_physicsClient.getDebugLinesColor();

		int lineWidth = 1;

		if (1)
		{
			btAlignedObjectArray<btVector3FloatData> points;
			points.resize(numLines*2);
			btAlignedObjectArray<unsigned int> indices;
			indices.resize(numLines*2);

			for (int i=0;i<numLines;i++)
			{
				points[i*2].m_floats[0] = fromLines[i].x();
				points[i*2].m_floats[1] = fromLines[i].y();
				points[i*2].m_floats[2] = fromLines[i].z();
				points[i*2+1].m_floats[0] = toLines[i].x();
				points[i*2+1].m_floats[1] = toLines[i].y();
				points[i*2+1].m_floats[2] = toLines[i].z();
				indices[i*2] = i*2;
				indices[i*2+1] = i*2+1;
			}

		
			
			float color[4] = {1,1,0,1};
			
			if (points.size() && indices.size())
			{
				m_guiHelper->getRenderInterface()->drawLines(&points[0].m_floats[0],color,points.size(),sizeof(btVector3FloatData),&indices[0],indices.size(),lineWidth);
			}
		} else
		{
			for (int i=0;i<numLines;i++)
			{
				m_guiHelper->getRenderInterface()->drawLine(fromLines[i],toLines[i],colorLines[i],lineWidth);
			}
		}

	}

	void prepareControlCommand(SharedMemoryCommand& command)
	{
		int controlMode = CONTROL_MODE_VELOCITY;//CONTROL_MODE_TORQUE;

        command.m_sendDesiredStateCommandArgument.m_controlMode = controlMode;
            
        for (int i=0;i<MAX_DEGREE_OF_FREEDOM;i++)
        {
            command.m_sendDesiredStateCommandArgument.m_desiredStateQdot[i] = 0;
            command.m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[i] = 1000;
        }
        for (int i=0;i<m_numMotors;i++)
        {
            btScalar targetVel = m_motorTargetVelocities[i].m_velTarget;
                
            int uIndex = m_motorTargetVelocities[i].m_uIndex;
            command.m_sendDesiredStateCommandArgument.m_desiredStateQdot[uIndex] = targetVel;
                
        }
            
	}
	virtual void	physicsDebugDraw(int debugFlags){}
	virtual bool	mouseMoveCallback(float x,float y){return false;};
	virtual bool	mouseButtonCallback(int button, int state, float x, float y){return false;}
	virtual bool	keyboardCallback(int key, int state){return false;}

	
	virtual void setSharedMemoryKey(int key)
	{
		m_physicsClient.setSharedMemoryKey(key);
	}
};





void MyCallback(int buttonId, bool buttonState, void* userPtr)
{
	PhysicsClientExample* cl = (PhysicsClientExample*) userPtr;

	SharedMemoryCommand command;

	switch (buttonId)
	{
	case  CMD_LOAD_URDF:
		{
			command.m_type =CMD_LOAD_URDF;
            sprintf(command.m_urdfArguments.m_urdfFileName,"r2d2.urdf");//kuka_lwr/kuka.urdf");
            command.m_urdfArguments.m_initialPosition[0] = 0.0;
			command.m_updateFlags = 
				URDF_ARGS_FILE_NAME| URDF_ARGS_INITIAL_POSITION|URDF_ARGS_INITIAL_ORIENTATION|URDF_ARGS_USE_MULTIBODY|URDF_ARGS_USE_FIXED_BASE;

			command.m_urdfArguments.m_initialPosition[1] = 0.0;
			command.m_urdfArguments.m_initialPosition[2] = 0.0;
			command.m_urdfArguments.m_initialOrientation[0] = 0.0;
			command.m_urdfArguments.m_initialOrientation[1] = 0.0;
			command.m_urdfArguments.m_initialOrientation[2] = 0.0;
			command.m_urdfArguments.m_initialOrientation[3] = 1.0;
			command.m_urdfArguments.m_useFixedBase = false;
			command.m_urdfArguments.m_useMultiBody = true;
			cl->enqueueCommand(command);
			break;
		}
	case CMD_CREATE_BOX_COLLISION_SHAPE:
		{
			command.m_type =CMD_CREATE_BOX_COLLISION_SHAPE;
			command.m_updateFlags = BOX_SHAPE_HAS_INITIAL_POSITION;
			command.m_createBoxShapeArguments.m_initialPosition[0] = 0;
			command.m_createBoxShapeArguments.m_initialPosition[1] = 0;
			command.m_createBoxShapeArguments.m_initialPosition[2] = -3;
			
			cl->enqueueCommand(command);
			break;
		}
	case CMD_REQUEST_ACTUAL_STATE:
		{
			command.m_type =CMD_REQUEST_ACTUAL_STATE;
			cl->enqueueCommand(command);
			break;
		};
	case CMD_STEP_FORWARD_SIMULATION:
		{
			command.m_type =CMD_STEP_FORWARD_SIMULATION;
			cl->enqueueCommand(command);
			command.m_type =CMD_REQUEST_DEBUG_LINES;
            command.m_requestDebugLinesArguments.m_debugMode = btIDebugDraw::DBG_DrawWireframe;//:DBG_DrawConstraints;
            command.m_requestDebugLinesArguments.m_startingLineIndex = 0;
			cl->enqueueCommand(command);
             
			break;
		}
	
    case CMD_SEND_DESIRED_STATE:
		{
			
            command.m_type =CMD_SEND_DESIRED_STATE;
            
			cl->prepareControlCommand(command);

			cl->enqueueCommand(command);		
			break;
		}
		case CMD_RESET_SIMULATION:
		{
			command.m_type = CMD_RESET_SIMULATION;
			cl->enqueueCommand(command);
			break;
		}
	case CMD_SEND_BULLET_DATA_STREAM:
		{
			command.m_type = buttonId;
			cl->enqueueCommand(command);
			break;
		}

	default:
		{
			b3Error("Unknown buttonId");
			btAssert(0);
		}
	};
}

 void PhysicsClientExample::enqueueCommand(const SharedMemoryCommand& orgCommand)
	{
		m_userCommandRequests.push_back(orgCommand);
		SharedMemoryCommand& cmd = m_userCommandRequests[m_userCommandRequests.size()-1];

		//b3Printf("User put command request %d on queue (queue length = %d)\n",cmd.m_type, m_userCommandRequests.size());
	}


PhysicsClientExample::PhysicsClientExample(GUIHelperInterface* helper)
:SharedMemoryCommon(helper),
m_wantsTermination(false),
m_numMotors(0)
{
	b3Printf("Started PhysicsClientExample\n");
}

PhysicsClientExample::~PhysicsClientExample()
{
    b3Printf("~PhysicsClientExample\n");
}

void	PhysicsClientExample::createButton(const char* name, int buttonId, bool isTrigger )
{
	ButtonParams button(name,buttonId,  isTrigger);
	button.m_callback = MyCallback;
	button.m_userPointer = this;
	m_guiHelper->getParameterInterface()->registerButtonParameter(button);
}

void	PhysicsClientExample::createButtons()
{
	bool isTrigger = false;
	
    if (m_guiHelper && m_guiHelper->getParameterInterface())
    {
        createButton("Load URDF",CMD_LOAD_URDF,  isTrigger);
        createButton("Step Sim",CMD_STEP_FORWARD_SIMULATION,  isTrigger);
        createButton("Send Bullet Stream",CMD_SEND_BULLET_DATA_STREAM,  isTrigger);
        createButton("Get State",CMD_REQUEST_ACTUAL_STATE,  isTrigger);
        createButton("Send Desired State",CMD_SEND_DESIRED_STATE,  isTrigger);
        createButton("Create Box Collider",CMD_CREATE_BOX_COLLISION_SHAPE,isTrigger);
        createButton("Reset Simulation",CMD_RESET_SIMULATION,isTrigger);
    }
}

void	PhysicsClientExample::initPhysics()
{
	if (m_guiHelper && m_guiHelper->getParameterInterface())
	{
		int upAxis = 2;
		m_guiHelper->setUpAxis(upAxis);

		createButtons();		
		
	} else
	{
        MyCallback(CMD_LOAD_URDF, true, this);
        MyCallback(CMD_STEP_FORWARD_SIMULATION,true,this);
        MyCallback(CMD_STEP_FORWARD_SIMULATION,true,this);
        MyCallback(CMD_RESET_SIMULATION,true,this);
    //    MyCallback(CMD_LOAD_URDF, true, this);
     //   MyCallback(CMD_STEP_FORWARD_SIMULATION,true,this);
     //   MyCallback(CMD_RESET_SIMULATION,true,this);
        
        /*
		m_userCommandRequests.push_back(CMD_LOAD_URDF);
		m_userCommandRequests.push_back(CMD_REQUEST_ACTUAL_STATE);
		m_userCommandRequests.push_back(CMD_SEND_DESIRED_STATE);
		m_userCommandRequests.push_back(CMD_REQUEST_ACTUAL_STATE);
		//m_userCommandRequests.push_back(CMD_SET_JOINT_FEEDBACK);
		m_userCommandRequests.push_back(CMD_CREATE_BOX_COLLISION_SHAPE);
		//m_userCommandRequests.push_back(CMD_CREATE_RIGID_BODY);
		m_userCommandRequests.push_back(CMD_STEP_FORWARD_SIMULATION);
		m_userCommandRequests.push_back(CMD_REQUEST_ACTUAL_STATE);
		m_userCommandRequests.push_back(CMD_SHUTDOWN);
		*/

	}

	if (!m_physicsClient.connect())
	{
		b3Warning("Cannot connect to physics client");
	}

}


void	PhysicsClientExample::stepSimulation(float deltaTime)
{
    
	if (m_physicsClient.isConnected())
    {
		SharedMemoryStatus status;
		bool hasStatus = m_physicsClient.processServerStatus(status);
		if (hasStatus && status.m_type == CMD_URDF_LOADING_COMPLETED)
		{
			for (int i=0;i<m_physicsClient.getNumJoints();i++)
			{
				b3JointInfo info;
				m_physicsClient.getJointInfo(i,info);
				b3Printf("Joint %s at q-index %d and u-index %d\n",info.m_jointName,info.m_qIndex,info.m_uIndex);
				
			}
		
            if (hasStatus && status.m_type == CMD_URDF_LOADING_COMPLETED)
            {
                for (int i=0;i<m_physicsClient.getNumJoints();i++)
                {
                    b3JointInfo info;
                    m_physicsClient.getJointInfo(i,info);
                    b3Printf("Joint %s at q-index %d and u-index %d\n",info.m_jointName,info.m_qIndex,info.m_uIndex);
                    
                    if (info.m_flags & JOINT_HAS_MOTORIZED_POWER)
                    {
                        if (m_numMotors<MAX_NUM_MOTORS)
                        {
                            char motorName[1024];
                            sprintf(motorName,"%s q'", info.m_jointName);
                            MyMotorInfo2* motorInfo = &m_motorTargetVelocities[m_numMotors];
                            motorInfo->m_velTarget = 0.f;
                            motorInfo->m_uIndex = info.m_uIndex;
                            
                            SliderParams slider(motorName,&motorInfo->m_velTarget);
                            slider.m_minVal=-4;
                            slider.m_maxVal=4;
                            if (m_guiHelper && m_guiHelper->getParameterInterface())
                            {
                            m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
                            }
                            m_numMotors++;
                        }
                    }
                    
                }

            }
		}
    
		if (hasStatus && status.m_type ==CMD_DEBUG_LINES_COMPLETED)
		{
            SharedMemoryCommand command;
			if (status.m_sendDebugLinesArgs.m_numRemainingDebugLines>0)
			{
				//continue requesting debug lines for drawing
				command.m_type =CMD_REQUEST_DEBUG_LINES;
				command.m_requestDebugLinesArguments.m_debugMode = btIDebugDraw::DBG_DrawWireframe;//DBG_DrawConstraints;
				command.m_requestDebugLinesArguments.m_startingLineIndex = status.m_sendDebugLinesArgs.m_numDebugLines+status.m_sendDebugLinesArgs.m_startingLineIndex;
				enqueueCommand(command);
			}
		}
		if (m_physicsClient.canSubmitCommand())
		{
			if (m_userCommandRequests.size())
			{
				//b3Printf("Outstanding user command requests: %d\n", m_userCommandRequests.size());
				SharedMemoryCommand command = m_userCommandRequests[0];

				//a manual 'pop_front', we don't use 'remove' because it will re-order the commands
				for (int i=1;i<m_userCommandRequests.size();i++)
				{
					m_userCommandRequests[i-1] = m_userCommandRequests[i];
				}

				m_userCommandRequests.pop_back();
				
				//for the CMD_RESET_SIMULATION we need to do something special: clear the GUI sliders
				if (command.m_type==CMD_RESET_SIMULATION)
				{
                    if (m_guiHelper->getParameterInterface())
                    {
                        m_guiHelper->getParameterInterface()->removeAllParameters();
                    }
                    m_numMotors=0;
					createButtons();
				}
				
				m_physicsClient.submitClientCommand(command);
			}  else
			{
				if (m_numMotors)
				{
					SharedMemoryCommand command;
					command.m_type =CMD_SEND_DESIRED_STATE;
					prepareControlCommand(command);
					enqueueCommand(command);		

					command.m_type =CMD_STEP_FORWARD_SIMULATION;
					enqueueCommand(command);

					command.m_type = CMD_REQUEST_ACTUAL_STATE;
					enqueueCommand(command);
				}
			}
		}
	}
	

}

extern int gSharedMemoryKey;


class CommonExampleInterface*    PhysicsClientCreateFunc(struct CommonExampleOptions& options)
{
    PhysicsClientExample* example = new PhysicsClientExample(options.m_guiHelper);
	if (gSharedMemoryKey>=0)
	{
		example->setSharedMemoryKey(gSharedMemoryKey);
	}
	return example;
}