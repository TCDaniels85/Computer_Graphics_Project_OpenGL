#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include <map>
#include <conio.h>

#include <raaCamera/raaCamera.h>
#include <raaUtilities/raaUtilities.h>
#include <raaMaths/raaMaths.h>
#include <raaMaths/raaVector.h>
#include <raaSystem/raaSystem.h>
#include <raaPajParser/raaPajParser.h>
#include <raaText/raaText.h>

#include "raaConstants.h"
#include "raaParse.h"
#include "raaControl.h"

#include <time.h>

// NOTES
// look should look through the libraries and additional files I have provided to familarise yourselves with the functionallity and code.
// The data is loaded into the data structure, managed by the linked list library, and defined in the raaSystem library.
// You will need to expand the definitions for raaNode and raaArc in the raaSystem library to include additional attributes for the siumulation process
// If you wish to implement the mouse selection part of the assignment you may find the camProject and camUnProject functions usefull


// core system global data
raaCameraInput g_Input; // structure to hadle input to the camera comming from mouse/keyboard events
raaCamera g_Camera; // structure holding the camera position and orientation attributes
raaSystem g_System; // data structure holding the imported graph of data - you may need to modify and extend this to support your functionallity
raaControl g_Control; // set of flag controls used in my implmentation to retain state of key actions

// global var: parameter name for the file to load
const static char csg_acFileParam[] = {"-input"};

// global var: file to load data from
char g_acFile[256];

// core functions -> reduce to just the ones needed by glut as pointers to functions to fulfill tasks
void display(); // The rendering function. This is called once for each frame and you should put rendering code here
void idle(); // The idle function is called at least once per frame and is where all simulation and operational code should be placed
void reshape(int iWidth, int iHeight); // called each time the window is moved or resived
void keyboard(unsigned char c, int iXPos, int iYPos); // called for each keyboard press with a standard ascii key
void keyboardUp(unsigned char c, int iXPos, int iYPos); // called for each keyboard release with a standard ascii key
void sKeyboard(int iC, int iXPos, int iYPos); // called for each keyboard press with a non ascii key (eg shift)
void sKeyboardUp(int iC, int iXPos, int iYPos); // called for each keyboard release with a non ascii key (eg shift)
void mouse(int iKey, int iEvent, int iXPos, int iYPos); // called for each mouse key event
void motion(int iXPos, int iYPos); // called for each mouse motion event

// Non glut functions
void myInit(); // the myinit function runs once, before rendering starts and should be used for setup
void nodeDisplay(raaNode *pNode); // callled by the display function to draw nodes
void arcDisplay(raaArc *pArc); // called by the display function to draw arcs
void buildGrid(); // 

//Constants
const float REST_COEF = 0.1f; //coefficient of restitution
const float TIME_STEP = 10.0f; 
const float DAMP_COEF = 0.2f;  //Dampening coefficient
int speedMultiplier = 1;

//Method added by me to perform simulation
void setForce(raaNode* pNode);  //resets force
void setVelocity(raaNode* pNode);  //resets velocity
void setArcs(raaArc* pArc);    //loops through arcs to draw lines
void setSpring(raaNode* pNode);   //loops though nodes to set resultant force
void randomiseNodes(raaNode* pNode);
void organiseNodesByWorldSystem(raaNode* pNode);
void resetCounters();
void changeSimulationSpeed(char direction); //alter simulation speed
void resetSimulationSpeed();  //reset simulation speed

//counters for tracking node positions when defining layout positioning
float counterA = 1.0f;
float counterB = 1.0f;
float counterC = 1.0f;
float counterD = 1.0f;
float counterE = 1.0f;
float counterF = 1.0f;

//menu functions
void createPopupMenu();
void menuEvents(int opt);
bool isRunning = false;  //is the simulation running
bool isGrid = true;  //is grid displayed
//Boolean values for displaying text
bool viewGridText = false;  
bool viewSimText = false;
bool viewSpeedText = false;
bool viewHelpText = false;

bool layoutChange = false;  //boolean value to indicate layout change is required

void text();  //prints state text to the HUD
void helpText();  //prints help text to the HUD

//method to animate layout changes
void changeLayout(raaNode* pNode);
int layoutCounter = 0;   //counter to track when nodes are in poisiton


/*
* Function to render the nodes, this is called from the display function.
*/
void nodeDisplay(raaNode *pNode) 
{
	// put your node rendering (ogl) code here
	float* position = pNode->m_afPosition;
	unsigned int continent = pNode->m_uiContinent;	

	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS); 

	char* countryName = pNode->m_acName; // create a char array to hold the country name of the node

	float afCol[4];//local array variable to hold node colour information

	//if statement to assign a colour to a node based on the continent it belongs to
	if (pNode->m_uiContinent == 1) {
		//green
		afCol[0] = 0.0f;
		afCol[1] = 1.0f;
		afCol[2] = 0.0f;
		afCol[3] = 1.0f;
				
	} else if (pNode->m_uiContinent == 2) {
		// Red
		afCol[0] = 1.0f;
		afCol[1] = 0.0f;
		afCol[2] = 0.0f;
		afCol[3] = 1.0f;
		
	} else if (pNode->m_uiContinent == 3) {
		//yellow
		afCol[0] = 1.0f;
		afCol[1] = 1.0f;
		afCol[2] = 0.0f;
		afCol[3] = 1.0f;
		
	} else if (pNode->m_uiContinent == 4) {
		//purple
		afCol[0] = 1.0f;
		afCol[1] = 0.0f;
		afCol[2] = 1.0f;
		afCol[3] = 1.0f;
	} else if (pNode->m_uiContinent == 5) {
		//blue
		afCol[0] = 0.0f;
		afCol[1] = 1.0f;
		afCol[2] = 1.0f;
		afCol[3] = 1.0f;
	} else if (pNode->m_uiContinent == 6) {
		 //orange
		afCol[0] = 1.0f;
		afCol[1] = 0.5f;
		afCol[2] = 0.0f;
		afCol[3] = 1.0f;
		
	}	
	utilitiesColourToMat(afCol, 2.0f); //assigns material to node using colour
	
	
	glTranslated(position[0], position[1], position[2]); //position node
	//if statement to set shape of node depending on world system
	if (pNode->m_uiWorldSystem == 1) {
		glutSolidSphere(mathsRadiusOfSphereFromVolume(pNode->m_fMass), 15, 15);	//Draw sphere
		glTranslated(0.0f, mathsRadiusOfSphereFromVolume(pNode->m_fMass), 0.0f); //translates text to just outside sphere
		glScalef(10.0f, 10.0f, 1.0f); //scale the text by 10 so it is readable
		glMultMatrixf(camRotMatInv(g_Camera)); //apply transformation matrix to align text with camera
		outlinePrint(countryName);  //adds country name to node
	} else if (pNode->m_uiWorldSystem == 2) {
		glutSolidCube(mathsDimensionOfCubeFromVolume(pNode->m_fMass)); //Draw cube
		glTranslated(0.0f, mathsDimensionOfCubeFromVolume(pNode->m_fMass)/1.8f, 0.0f); //translates text to just outside cube
		glScalef(10.0f, 10.0f, 1.0f); //scale the text by 10 so it is readable
		glMultMatrixf(camRotMatInv(g_Camera)); //apply transformation matrix to align text with camera
		outlinePrint(countryName);
	} else if (pNode->m_uiWorldSystem == 3) {		
		glRotated(270, 1.0, 0.0, 0.0);		//rotate cone shape to appear upright in world
		glutSolidCone(mathsRadiusOfConeFromVolume(pNode->m_fMass), 20, 10, 10); //Draw cone
		glRotated(90, 1.0, 0.0, 0.0); //rotates to enable text in correct position		
		glTranslated(0.0f, 20, 0.0f);//translates text to just above cone
		glScalef(10.0f, 10.0f, 1.0f); //scale the text by 10 so it is readable
		glMultMatrixf(camRotMatInv(g_Camera)); //apply transformation matrix to align text with camera
		outlinePrint(countryName);
		
	}
	

	glPopMatrix();
	glPopAttrib();	
}

/*
* Function to incrementally change the layout to create an animation between changes.
* the new position is set in the appropriate layout method, then current node position is increment/decrement by
* one until it has reached the correct position
*/
void changeLayout(raaNode* pNode) {
	float* curPos = pNode->m_afPosition;
	float* newPos = pNode->new_afPosition;
	
	if ((int)curPos[0] != (int)newPos[0]) { //compares position as a cast to int to avoid fractional differences
		if (curPos[0] < newPos[0]) {  
			curPos[0] += 1.0f;
		}
		else if (curPos[0] > newPos[0]) {
			curPos[0] -= 1.0f;
		}
	}
	
	if((int)curPos[1] != (int)newPos[1]) {
		if (curPos[1] < newPos[1]) {
			curPos[1] += 1.0f;
		}
		else if (curPos[1] > newPos[1]) {
			curPos[1] -= 1.0f;
		}
	}
	if ((int)curPos[2] != (int)newPos[2]) {
		if (curPos[2] < newPos[2]) {
			curPos[2] += 1.0f;
		}
		else if (curPos[2] > newPos[2]) {
			curPos[2] -= 1.0f;
		}
	}

	pNode->m_afPosition[0] = curPos[0];
	pNode->m_afPosition[1] = curPos[1];
	pNode->m_afPosition[2] = curPos[2];

	if ((int)curPos[0] == (int)newPos[0] && (int)curPos[1] == (int)newPos[1] && (int)curPos[2]== (int)newPos[2]) {
		
		layoutCounter++; //increases layout counter when node is in position
		
	}

}

/*
* Randomises nodes in the world space, sets postion between 100 and 600.
*/
void randomiseNodes(raaNode* pNode) {
	
	float max = 600.0f;
	float x = (float)rand() / ((float)RAND_MAX / max);
	pNode->new_afPosition[0] = (float)rand() / ((float)RAND_MAX / max) + 100;
	pNode->new_afPosition[1] = (float)rand() / ((float)RAND_MAX / max) + 100;
	pNode->new_afPosition[2] = (float)rand() / ((float)RAND_MAX / max) + 100;
}

/*
* Organises nodes by continent, loops through nodes and sets new position reletive to the node below it
* using a counter
*/
void organiseNodesByWorldSystem(raaNode* pNode) {
	pNode->new_afPosition[0] = 100.0f;
	if (pNode->m_uiWorldSystem == 1) {		
		pNode->new_afPosition[1] = 100.0f * counterA;
		pNode->new_afPosition[2] = 400.0f;
		counterA += 0.5f;
	}
	else if (pNode->m_uiWorldSystem == 2) {		
		pNode->new_afPosition[1] = 100.0f * counterB;
		pNode->new_afPosition[2] = 500.0f;
		counterB += 0.5f;
	} else if (pNode->m_uiWorldSystem == 3) {		
		pNode->new_afPosition[1] = 100.0f * counterC;
		pNode->new_afPosition[2] = 600.0f;
		counterC += 0.5f;
	}
}
/*
* Organises nodes by continent, loops through nodes and sets position reletive to the node below it
* using a counter
*/
void organiseNodesByContinent(raaNode* pNode) {
	pNode->new_afPosition[0] = 100.0f;
	if (pNode->m_uiContinent == 1) {
		pNode->new_afPosition[1] = 100.0f * counterA;
		pNode->new_afPosition[2] = 100.0f;
		counterA += 0.5f;
	} else if (pNode->m_uiContinent == 2) {
		pNode->new_afPosition[1] = 100.0f * counterB;
		pNode->new_afPosition[2] = 200.0f;
		counterB += 0.5f;
	} else if (pNode->m_uiContinent == 3) {
		pNode->new_afPosition[1] = 100.0f * counterC;
		pNode->new_afPosition[2] = 300.0f;
		counterC += 0.5f;
	} else if (pNode->m_uiContinent == 4) {
		pNode->new_afPosition[1] = 100.0f * counterD;
		pNode->new_afPosition[2] = 400.0f;
		counterD += 0.5f;
	} else if (pNode->m_uiContinent == 5) {
		pNode->new_afPosition[1] = 100.0f * counterE;
		pNode->new_afPosition[2] = 500.0f;
		counterE += 0.5f;
	} else if (pNode->m_uiContinent == 6) {
		pNode->new_afPosition[1] = 100.0f * counterF;
		pNode->new_afPosition[2] = 600.0f;
		counterF += 0.5f;
	}	
}
/*
* Resets counters to 1, these are used to position layouts
*/
void resetCounters() {
	counterA = 1.0f;
	counterB = 1.0f;
	counterC = 1.0f;
	counterD = 1.0f;
	counterE = 1.0f;
	counterF = 1.0f;
}

/*
* Function to render arcs between nodes, loops through each pair of connected nodes to create a line
*/
void arcDisplay(raaArc *pArc) 
{
	//assign nodes
	raaNode* m_pNode0 = pArc->m_pNode0;
	raaNode* m_pNode1 = pArc->m_pNode1;
	//assign position for each end of arc
	float* arcPosition0 = m_pNode0->m_afPosition;
	float* arcPosition1 = m_pNode1->m_afPosition;
	

	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES); //draws a line that fades from blue to magenta
	glColor3f( 0.0f, 0.0f, 1.0f); //blue
	glVertex3f(arcPosition0[0], arcPosition0[1], arcPosition0[2]);
	glColor3f(1.0f, 0.0f, 1.0f); //magenta
	glVertex3f(arcPosition1[0], arcPosition1[1], arcPosition1[2]);
	glEnd();


}

/*
* Sets resultant force  and velocity of each node to zero set by visitNodes(&g_System, setForce);
*/
void setForce(raaNode* pNode) {
	
	pNode->res_force[0] = 0.0f;
	pNode->res_force[1] = 0.0f;
	pNode->res_force[2] = 0.0f;
	pNode->res_force[3] = 0.0f;	
}

/*
* Set the velocity of the node to 0
*/
void setVelocity(raaNode* pNode) {
	pNode->m_velocity[0] = 0.0f;
	pNode->m_velocity[1] = 0.0f;
	pNode->m_velocity[2] = 0.0f;
	pNode->m_velocity[3] = 0.0f;
}
/*
* Loops through arcs and calculates the resultant force for each node
*/
void setArcs(raaArc* pArc) {
	raaNode* node0 = pArc->m_pNode0;
	raaNode* node1 = pArc->m_pNode1;
	
	float* arcPosition0 = node0->m_afPosition;
	float* arcPosition1 = node1->m_afPosition;
	

	float* resForce0 = node0->res_force;
	float* resForce1 = node1->res_force;

	//Work out the distance between the two nodes
	float arcDistance = vecDistance(arcPosition0, arcPosition1); 
	
	//Declare direction bector and calculate by subtracting the position of one node from the other
	float directionVec[3];	
	vecSub(arcPosition0, arcPosition1, directionVec); //subtracts same as above
	
	//Multiply directional vector by the distance and normalise this value
	for (int i = 0; i < 3; i++) {
		directionVec[i] = directionVec[i] * arcDistance;
	}
	
	
	float norm = vecNormalise(directionVec, directionVec); 	
	
	

	//Calculate the spring force as a vector, coefficient of restitution multiplied by the directional vector 
	float springForce[4];
	springForce[0] = REST_COEF  * directionVec[0];
	springForce[1] = REST_COEF  * directionVec[1];
	springForce[2] = REST_COEF  * directionVec[2];
	springForce[3] = 0.0f;
	
	
	//Calculate the vector force  for both nodes, first setting the vector force to the scalar product for each node one + one -
	float vectorForce0[4] = { 0.0f,0.0f,0.0f, 0.0f }; 
	float vectorForce1[4] = { 0.0f,0.0f,0.0f, 0.0f };
	
	vecScalarProduct(springForce, -1.0f, vectorForce0); //scalar of spring force
	vecScalarProduct(springForce, 1.0f, vectorForce1);	
	//Then multiply spring scalar force(saved as vectorforce) by the directional vector
		
	vecVectorProduct(vectorForce0, directionVec, vectorForce0);
	vecVectorProduct(vectorForce1, directionVec, vectorForce1);
	
	//add the vector force to each nodes resultant force vector
	vecAdd(vectorForce0, resForce0, node0->res_force);
	vecAdd(vectorForce1, resForce1, node1->res_force);
	
	
}
/*
* Loops through the nodes to calculate the motion for each node
*/
void setSpring(raaNode* pNode) {
	
	raaNode* node = pNode;
	float nodeMass = pNode->m_fMass;
	float* resForce = pNode->res_force;
	float* initVelocity = pNode->m_velocity;
	float* position = pNode->m_afPosition;
	

	// Calculate acceleration of each node
	float accel[3];
	for (int i = 0; i < 3; i++) {
		accel[i] = resForce[i] /nodeMass ;
	}
	
	//calculate velocity of body following acceleration, multiplied by user input value
	float newVelocity[3];
	for (int i = 0; i < 3; i++) {
		newVelocity[i] = initVelocity[i] + accel[i] * (TIME_STEP * speedMultiplier);
		
	}		
	
	//calculate motion of the body
	float motion[3];	
	for (int i = 0; i < 3; i++) {
		motion[i] = newVelocity[i] * TIME_STEP * (accel[i] * (TIME_STEP * TIME_STEP))/2.0f;
	}
	
	//add motion to body
	vecAdd(position, motion, pNode->m_afPosition);	

	
	//calculate velocity after motion added, v=s/t		
	for (int i = 0; i < 3; i++) {
		//newVelocity[i] = initVelocity[i] + accel[i] * (TIME_STEP * speedMultiplier);
		newVelocity[i] = motion[i] / TIME_STEP;
	}
	
	
	//set the nodes new velocity and apply damping
	for (int i = 0; i < 3; i++) {
		pNode->m_velocity[i] = (newVelocity[i]) * DAMP_COEF; //add damping * 0.2
	}
	
	
	
}
/*
* Creates a popup menu on the mouse button right click, called in init block
*/
void createPopupMenu() {
	int menuOpt;
	int layout;
	int hud;
	int spd;
	//calback function passes related int valu from menu entry to menuEvents function
	layout = glutCreateMenu(menuEvents);
	glutAddMenuEntry("Randomise node layout", 6);
	glutAddMenuEntry("Organise by continent", 7);
	glutAddMenuEntry("Organise by world system", 8);
	hud = glutCreateMenu(menuEvents);
	glutAddMenuEntry("Toggle grid status", 4);
	glutAddMenuEntry("Toggle simulation status", 5);
	glutAddMenuEntry("Toggle simulation speed", 10);
	glutAddMenuEntry("Toggle help text", 11);
	spd = glutCreateMenu(menuEvents);
	glutAddMenuEntry("Increase speed", 12);
	glutAddMenuEntry("Decrease speed", 13);

	menuOpt = glutCreateMenu(menuEvents);	
	
	//Add menu entries
	glutAddMenuEntry("Turn grid on/off",1);
	glutAddMenuEntry("Reset camera", 2);
	glutAddMenuEntry("Start/Stop simulation", 3);	
	glutAddSubMenu("Layouts", layout);
	glutAddSubMenu("Heads up display", hud);
	glutAddSubMenu("Change speed", spd);
	glutAddMenuEntry("Reset simulation", 9);
	
	


	//Set menu to right mouse button
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
}
/*
* Handles the logic for each menu option
*/
void menuEvents(int opt) {
	switch (opt) {
	case 1:
		//Toggle grid on/off
		controlToggle(g_Control, csg_uiControlDrawGrid);
		if (isGrid) {
			isGrid = false;
		}
		else {
			isGrid = true;
		}
		break;
	case 2:
		//resets camera to starting position
		camReset(g_Camera); 
		break;
	case 3:		
		//Sets simulation running or stops it
		if (isRunning) {
			isRunning = false;
		} else if (!isRunning) {
			isRunning = true;
		}
		break;		
	case 4:
		//Toggle HUD text for grid status
		if (viewGridText) {
			viewGridText = false;
		}
		else {
			viewGridText = true;
		}
		break;
	case 5:
		//Toggle HUD text for simulation status
		if (viewSimText) {
			viewSimText = false;
		}
		else {
			viewSimText = true;
		}
		break;
	case 6:
		//randomise layout
		visitNodes(&g_System, randomiseNodes);
		layoutChange = true;
		break;
	case 7:
		//change layout to continent
		resetCounters();		
		visitNodes(&g_System, organiseNodesByContinent);
		layoutChange = true;
		break;
	case 8:
		//change layout to world system
		resetCounters();
		visitNodes(&g_System, organiseNodesByWorldSystem);
		layoutChange = true;
		break;
	case 9:
		//reset simulation
		myInit();
		resetSimulationSpeed();
		layoutChange = false;  //incase this is running when reset
		break;
	case 10:
		//Toggle HUD text to display simulation speed
		if (viewSpeedText) {
			viewSpeedText = false;
		}
		else {
			viewSpeedText = true;
		}
		break;
	case 11:
		//Toggle HUD help text
		if (viewHelpText) {
			viewHelpText = false;
		}
		else {
			viewHelpText = true;
		}
		break;
	case 12:
		//incerase speed		
		changeSimulationSpeed('p');
		break;
	case 13:
		//Decrease speed
		changeSimulationSpeed('l');
		break;
	}
}
/*
* Increases or decreases speed multiplier to allow user control over simulatopn speed
*/
void changeSimulationSpeed(char direction) {
	if (direction == 'p' && speedMultiplier < 99) {
		
		speedMultiplier++;
		
	}
	if (direction == 'l' && speedMultiplier > 1) {
		
		speedMultiplier--;
		
	}
}

/*
* Resets speed multiplier back to 1
*/
void resetSimulationSpeed() {
	speedMultiplier = 1;
}

/*
* Sets text to be displayed on HUD
*/
void text() {
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix(); 
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	//Disable depth and lighting for text
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	
	//Sets text dependant on simulation status
	char* string;	
	if (isRunning) {
		string = "Simulation Running\n";
		 
	}
	else {
		string = "Simulation Stopped\n";
		
	}
	glRasterPos2f(-1.0f, 0.7f); //positions text on screen
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);  //sets screen
	//Boolean value set with popup menu decides if text is displayed or not
	if (viewSimText) {
		int len = strlen(string);
		for (int i = 0; i < len; i++) {

			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, string[i]);
		}
		
	}
	//Sets grid text dependant on status
	char* gridString;
	if (isGrid) {
		gridString = "Grid Display On\n";
	}
	else {
		gridString = "Grid Display Off";
	}
	glRasterPos2f(-1.0f, 0.8f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	//Boolean value set with popup menu decides if text is displayed or not
	if (viewGridText) {
		int len = strlen(gridString);
		for (int i = 0; i < len; i++) {

			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, gridString[i]);
		}
	}
	//Logic to display speed status
	glRasterPos2f(-1.0f, 0.6f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	if (viewSpeedText) {
		char* userSpeed;
		//dynamically allocate memory according to current size of int
		userSpeed = (char*)malloc(sizeof(speedMultiplier));
		if (userSpeed != NULL) {
			_itoa(speedMultiplier, userSpeed, 10); //convert in to char array
		}
				
		
		char speed[] = "Simulation speed multiplier:  " ;
		
		char* speedString;
		//dynamically allocate memory according to the size of the two char arrays to be concatenated
		speedString = (char*)malloc(sizeof(userSpeed) + sizeof(speed)); 
		
		
		if (speedString != NULL && userSpeed != NULL) {  //check to ensure char arrays are not null
			//copy and concatenate strings
			strcpy(speedString, speed);
			strcat(speedString, userSpeed);
			int len = strlen(speedString);
			for (int i = 0; i < len; i++) {

				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, speedString[i]);
			}
		}
		
			
		
		free(userSpeed);  //free up memory allocation for dynamic arrays
		free(speedString);
	}
	
	
	//re-enable settings and return matrix to previous settings
	glEnable(GL_DEPTH_TEST); 
	glEnable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix(); 
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
/*
* Function to display help text on hud 
*/
void helpText() {	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	//Disable depth and lighting for text
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	char *num[5];
	//char helpText[5];
	
	//Multidimensional array containing help text to be displayed on hud
	char helpText[14][35] = { 
		{"Key help:"},
		{"w: Zoom in"},
		{"s: Zoom out"},
		{"c: Print camera details to console"},
		{"y: Toggle grid lines"},
		{"t: Start simulation"},
		{"y: Stop simulation"},
		{"r: Reset simulation"},
		{"p: Increase simulation speed"},
		{"l: Decrease simulation speed"},
		{"n: Toggle gird status(HUD)"},
		{"v: Toggle simulation status(HUD)"},
		{"b: Toggle speed status(HUD)"},
		{"h: Toggle help text"}
	};

	glColor4f(1.0f, 1.0f, 0.0f, 1.0f);  //sets colour

	float y = 0.6; // y value which will change each loop to move to next line of text
	if (viewHelpText) {
		for (int i = 0; i < 14; i++){  //loops through array of char arrays
			int a = strlen(helpText[i]);
			y -= 0.1;
			glRasterPos2f(-0.1f, y); //positions text on screen
			
				for (int j = 0; j < a; j++) {

					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, helpText[i][j]);
				}
		}
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);  //sets colour back to white
		
	}


	//re-enable settings and return matrix to previous settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}




// draw the scene. Called once per frame and should only deal with scene drawing (not updating the simulator)
void display() 
{
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT); // clear the rendering buffers

	glLoadIdentity(); // clear the current transformation state
	glMultMatrixf(camObjMat(g_Camera)); // apply the current camera transform

	// draw the grid if the control flag for it is true	
	if (controlActive(g_Control, csg_uiControlDrawGrid)) glCallList(gs_uiGridDisplayList);
	
	glPushAttrib(GL_ALL_ATTRIB_BITS); // push attribute state to enable constrained state changes
	visitNodes(&g_System, nodeDisplay); // loop through all of the nodes and draw them with the nodeDisplay function
	visitArcs(&g_System, arcDisplay); // loop through all of the arcs and draw them with the arcDisplay function
	glPopAttrib();


	// draw a simple sphere
	float afCol[] = { 0.3f, 1.0f, 0.5f, 1.0f };
	utilitiesColourToMat(afCol, 2.0f);
	
	glPushMatrix();
	glTranslatef(0.0f, 30.0f, 0.0f);
	//glutSolidSphere(5.0f, 10, 10);
	
	if (viewGridText || viewSimText || viewSpeedText) {
		text();
	}	
	if (viewHelpText) {
		helpText();
	}
	glPopMatrix();
	
	glFlush(); // ensure all the ogl instructions have been processed
	glutSwapBuffers(); // present the rendered scene to the screen
	
}

// processing of system and camera data outside of the renderng loop
void idle() 
{
	controlChangeResetAll(g_Control); // re-set the update status for all of the control flags
	camProcessInput(g_Input, g_Camera); // update the camera pos/ori based on changes since last render
	camResetViewportChanged(g_Camera); // re-set the camera's viwport changed flag after all events have been processed
	glutPostRedisplay();// ask glut to update the screen
	//boolean to set simulation running or stop it
	if (isRunning) {
		visitNodes(&g_System, setForce); //reset force
		
		visitArcs(&g_System, setArcs);
		visitNodes(&g_System, setSpring);
	}
	//checks boolean to indcate layout chage
	if (layoutChange) {
		layoutCounter = 0;
		visitNodes(&g_System, changeLayout);   //visit nodes to change position gradually
		if (layoutCounter == 80) {
			layoutChange = false;  //once the layout counter =80 (the number of countries) indicates every node is in position	
		}			
	}	
}


// respond to a change in window position or shape
void reshape(int iWidth, int iHeight)  
{
	glViewport(0, 0, iWidth, iHeight);  // re-size the rendering context to match window
	camSetViewport(g_Camera, 0, 0, iWidth, iHeight); // inform the camera of the new rendering context size
	glMatrixMode(GL_PROJECTION); // switch to the projection matrix stack 
	glLoadIdentity(); // clear the current projection matrix state
	gluPerspective(csg_fCameraViewAngle, ((float)iWidth)/((float)iHeight), csg_fNearClip, csg_fFarClip); // apply new state based on re-sized window
	glMatrixMode(GL_MODELVIEW); // swap back to the model view matrix stac
	glGetFloatv(GL_PROJECTION_MATRIX, g_Camera.m_afProjMat); // get the current projection matrix and sort in the camera model
	glutPostRedisplay(); // ask glut to update the screen
}

// detect key presses and assign them to actions
void keyboard(unsigned char c, int iXPos, int iYPos)
{
	switch(c)
	{
	case 'w':
		camInputTravel(g_Input, tri_pos); // mouse zoom
		break;
	case 's':
		camInputTravel(g_Input, tri_neg); // mouse zoom
		break;
	case 'c':
		camPrint(g_Camera); // print the camera data to the comsole
		break;
	case 'g':
		controlToggle(g_Control, csg_uiControlDrawGrid); // toggle the drawing of the grid
		break;
	case 't':
		//Starts simulation
		isRunning = true;
		break;
	case 'y':
		//Stops simulation
		isRunning = false;
		break;
	case 'r':
		//resets simulation
		myInit();
		resetSimulationSpeed();
		layoutChange = false;  //incase reset is performed while this is running
		break;
	case 'p':
		//increase speed
		changeSimulationSpeed('p');
		break;
	case 'l':
		//decrease speed
		changeSimulationSpeed('l');
		break;
	case 'n':
		//Toggle HUD text for grid status
		if (viewGridText) {
			viewGridText = false;
		}
		else {
			viewGridText = true;
		}
		break;
	case 'v':
		//Toggle HUD text for simulation status
		if (viewSimText) {
			viewSimText = false;
		}
		else {
			viewSimText = true;
		}
		break;
	case 'b':
		//Toggle HUD text to display simulation speed
		if (viewSpeedText) {
			viewSpeedText = false;
		}
		else {
			viewSpeedText = true;
		}
		break;
	case 'h':
		//view help text in hud
		if (viewHelpText) {
			viewHelpText = false;
		}
		else {
			viewHelpText = true;
		}
		break;
	}
}

// detect standard key releases
void keyboardUp(unsigned char c, int iXPos, int iYPos) 
{
	switch(c)
	{
		// end the camera zoom action
		case 'w': 
		case 's':
			camInputTravel(g_Input, tri_null);
			break;
	}
}

void sKeyboard(int iC, int iXPos, int iYPos)
{
	// detect the pressing of arrow keys for ouse zoom and record the state for processing by the camera
	switch(iC)
	{
		case GLUT_KEY_UP:
			camInputTravel(g_Input, tri_pos);
			break;
		case GLUT_KEY_DOWN:
			camInputTravel(g_Input, tri_neg);
			break;
	}
}

void sKeyboardUp(int iC, int iXPos, int iYPos)
{
	// detect when mouse zoom action (arrow keys) has ended
	switch(iC)
	{
		case GLUT_KEY_UP:
		case GLUT_KEY_DOWN:
			camInputTravel(g_Input, tri_null);
			break;
	}
}

void mouse(int iKey, int iEvent, int iXPos, int iYPos)
{
	// capture the mouse events for the camera motion and record in the current mouse input state
	if (iKey == GLUT_LEFT_BUTTON)
	{
		camInputMouse(g_Input, (iEvent == GLUT_DOWN) ? true : false);
		if (iEvent == GLUT_DOWN)camInputSetMouseStart(g_Input, iXPos, iYPos);
	}
	else if (iKey == GLUT_MIDDLE_BUTTON)
	{
		camInputMousePan(g_Input, (iEvent == GLUT_DOWN) ? true : false);
		if (iEvent == GLUT_DOWN)camInputSetMouseStart(g_Input, iXPos, iYPos);
	}
}

void motion(int iXPos, int iYPos)
{
	// if mouse is in a mode that tracks motion pass this to the camera model
	if(g_Input.m_bMouse || g_Input.m_bMousePan) camInputSetMouseLast(g_Input, iXPos, iYPos);
}


void myInit()
{
	// setup my event control structure
	controlInit(g_Control);

	// initalise the maths library
	initMaths();

	// Camera setup
	camInit(g_Camera); // initalise the camera model
	camInputInit(g_Input); // initialise the persistant camera input data 
	camInputExplore(g_Input, true); // define the camera navigation mode

	// opengl setup - this is a basic default for all rendering in the render loop
	glClearColor(csg_afColourClear[0], csg_afColourClear[1], csg_afColourClear[2], csg_afColourClear[3]); // set the window background colour
	glEnable(GL_DEPTH_TEST); // enables occusion of rendered primatives in the window
	glEnable(GL_LIGHT0); // switch on the primary light
	glEnable(GL_LIGHTING); // enable lighting calculations to take place
	glEnable(GL_BLEND); // allows transparency and fine lines to be drawn
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // defines a basic transparency blending mode
	glEnable(GL_NORMALIZE); // normalises the normal vectors used for lighting - you may be able to switch this iff (performance gain) is you normalise all normals your self
	glEnable(GL_CULL_FACE); // switch on culling of unseen faces
	glCullFace(GL_BACK); // set culling to not draw the backfaces of primatives

	// build the grid display list - display list are a performance optimization 
	buildGrid();

	//Allow user to create a menu
	createPopupMenu();
	
	

	// initialise the data system and load the data file
	initSystem(&g_System);
	parse(g_acFile, parseSection, parseNetwork, parseArc, parsePartition, parseVector);
	//set node force and velocity to 0
	visitNodes(&g_System, setForce);
	visitNodes(&g_System, setVelocity);
}

int main(int argc, char* argv[])
{
	// check parameters to pull out the path and file name for the data file
	for (int i = 0; i<argc; i++) if (!strcmp(argv[i], csg_acFileParam)) sprintf_s(g_acFile, "%s", argv[++i]);


	if (strlen(g_acFile)) 
	{ 
		// if there is a data file

		glutInit(&argc, (char**)argv); // start glut (opengl window and rendering manager)

		glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA); // define buffers to use in ogl
		glutInitWindowPosition(csg_uiWindowDefinition[csg_uiX], csg_uiWindowDefinition[csg_uiY]);  // set rendering window position
		glutInitWindowSize(csg_uiWindowDefinition[csg_uiWidth], csg_uiWindowDefinition[csg_uiHeight]); // set rendering window size
		glutCreateWindow("raaAssignment1-2017");  // create rendering window and give it a name

		buildFont(); // setup text rendering (use outline print function to render 3D text


		myInit(); // application specific initialisation

		// provide glut with callback functions to enact tasks within the event loop
		glutDisplayFunc(display);
		glutIdleFunc(idle);
		glutReshapeFunc(reshape);
		glutKeyboardFunc(keyboard);
		glutKeyboardUpFunc(keyboardUp);
		glutSpecialFunc(sKeyboard);
		glutSpecialUpFunc(sKeyboardUp);
		glutMouseFunc(mouse);
		glutMotionFunc(motion);
		glutMainLoop(); // start the rendering loop running, this will only ext when the rendering window is closed 

		killFont(); // cleanup the text rendering process

		return 0; // return a null error code to show everything worked
	}
	else
	{
		// if there isn't a data file 

		printf("The data file cannot be found, press any key to exit...\n");
		_getch();
		return 1; // error code
	}
}

void buildGrid()
{
	if (!gs_uiGridDisplayList) gs_uiGridDisplayList= glGenLists(1); // create a display list

	glNewList(gs_uiGridDisplayList, GL_COMPILE); // start recording display list

	glPushAttrib(GL_ALL_ATTRIB_BITS); // push attrib marker
	glDisable(GL_LIGHTING); // switch of lighting to render lines

	glColor4fv(csg_afDisplayListGridColour); // set line colour

	// draw the grid lines
	glBegin(GL_LINES);
	for (int i = (int)csg_fDisplayListGridMin; i <= (int)csg_fDisplayListGridMax; i++)
	{
		glVertex3f(((float)i)*csg_fDisplayListGridSpace, 0.0f, csg_fDisplayListGridMin*csg_fDisplayListGridSpace);
		glVertex3f(((float)i)*csg_fDisplayListGridSpace, 0.0f, csg_fDisplayListGridMax*csg_fDisplayListGridSpace);
		glVertex3f(csg_fDisplayListGridMin*csg_fDisplayListGridSpace, 0.0f, ((float)i)*csg_fDisplayListGridSpace);
		glVertex3f(csg_fDisplayListGridMax*csg_fDisplayListGridSpace, 0.0f, ((float)i)*csg_fDisplayListGridSpace);
	}
	glEnd(); // end line drawing

	glPopAttrib(); // pop attrib marker (undo switching off lighting)

	glEndList(); // finish recording the displaylist
}