/*
Title: AABB - Circle Dynamic 2D collision Detection
File Name: main.cpp
Copyright � 2015
Original authors: Srinivasan Thiagarajan
Written under the supervision of David I. Schwartz, Ph.D., and
supported by a professional development seed grant from the B. Thomas
Golisano College of Computing & Information Sciences
(https://www.rit.edu/gccis) at the Rochester Institute of Technology.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Description:
This example demonstrates the dynamic collision detection between a circle 
and a axis aligned bounding box. This test is composed of three tests. First,
to check if the starting point is colliding with the rectangle. Second, Check
if the end state is colliding with the rectangle.Now that we know that the start
and the end state don't collide, then we can presumme that the collision is
occuring along the path if occuring at all. We take the path of the circle (relative
 to the the box) and find the path followed by the end points on the circle which are 
 prependicular to the the movement. Following these poitns we, get two lines. If any 
 of these lines intersect with the box, then the circle will collide in the next timestep.

 use "w,a,s and d " to move the circle. Consider each movement to be done in one timestep.
 If the program detects a collision will occur in the next timestep, then it wont move the circle.

References:
Nicholas Gallagher
AABB-2D by Brockton Roth
*/


#include "GLIncludes.h"

// We change this variable upon detecting collision
float blue = 0.0f;
glm::vec2 displacement (0.0f,0.0f);
bool isKeyPressed = false;
float moverate = 0.33f;

//the number of disvision used to make the structure of the circle
int NumberOfDivisions = 20;

//Tochange the integration from automatic to manual
bool manual = false;

glm::mat4 MVP1, MVP2;

//This struct consists of the basic stuff needed for getting the shape on the screen.
struct stuff_for_drawing{
	
	//This stores the address the buffer/memory in the GPU. It acts as a handle to access the buffer memory in GPU.
	GLuint vbo;

	//This will be used to tell the GPU, how many vertices will be needed to draw during drawcall.
	int numberOfVertices;

	//This function gets the number of vertices and all the vertex values and stores them in the buffer.
	void initBuffer(int numVertices, VertexFormat* vertices)
	{
		numberOfVertices = numVertices;

		// This generates buffer object names
		// The first parameter is the number of buffer objects, and the second parameter is a pointer to an array of buffer objects (yes, before this call, vbo was an empty variable)
		glGenBuffers(1, &vbo);
		
		//// Binds a named buffer object to the specified buffer binding point. Give it a target (GL_ARRAY_BUFFER) to determine where to bind the buffer.
		//// There are several different target parameters, GL_ARRAY_BUFFER is for vertex attributes, feel free to Google the others to find out what else there is.
		//// The second paramter is the buffer object reference. If no buffer object with the given name exists, it will create one.
		//// Buffer object names are unsigned integers (like vbo). Zero is a reserved value, and there is no default buffer for each target (targets, like GL_ARRAY_BUFFER).
		//// Passing in zero as the buffer name (second parameter) will result in unbinding any buffer bound to that target, and frees up the memory.
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		//// Creates and initializes a buffer object's data.
		//// First parameter is the target, second parameter is the size of the buffer, third parameter is a pointer to the data that will copied into the buffer, and fourth parameter is the 
		//// expected usage pattern of the data. Possible usage patterns: GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY, GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY, GL_DYNAMIC_DRAW, 
		//// GL_DYNAMIC_READ, or GL_DYNAMIC_COPY
		//// Stream means that the data will be modified once, and used only a few times at most. Static means that the data will be modified once, and used a lot. Dynamic means that the data 
		//// will be modified repeatedly, and used a lot. Draw means that the data is modified by the application, and used as a source for GL drawing. Read means the data is modified by 
		//// reading data from GL, and used to return that data when queried by the application. Copy means that the data is modified by reading from the GL, and used as a source for drawing.
		glBufferData(GL_ARRAY_BUFFER, sizeof(VertexFormat) * numVertices, vertices, GL_STATIC_DRAW);

		//// By default, all client-side capabilities are disabled, including all generic vertex attribute arrays.
		//// When enabled, the values in a generic vertex attribute array will be accessed and used for rendering when calls are made to vertex array commands (like glDrawArrays/glDrawElements)
		//// A GL_INVALID_VALUE will be generated if the index parameter is greater than or equal to GL_MAX_VERTEX_ATTRIBS
		glEnableVertexAttribArray(0);

		//// Defines an array of generic vertex attribute data. Takes an index, a size specifying the number of components (in this case, floats)(has a max of 4)
		//// The third parameter, type, can be GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_FIXED, or GL_FLOAT
		//// The fourth parameter specifies whether to normalize fixed-point data values, the fifth parameter is the stride which is the offset (in bytes) between generic vertex attributes
		//// The fifth parameter is a pointer to the first component of the first generic vertex attribute in the array. If a named buffer object is bound to GL_ARRAY_BUFFER (and it is, in this case) 
		//// then the pointer parameter is treated as a byte offset into the buffer object's data.
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)16);
		//// You'll note sizeof(VertexFormat) is our stride, because each vertex contains data that adds up to that size.
		//// You'll also notice we offset this parameter by 16 bytes, this is because the vec3 position attribute is after the vec4 color attribute. A vec4 has 4 floats, each being 4 bytes 
		//// so we offset by 4*4=16 to make sure that our first attribute is actually the position. The reason we put position after color in the struct has to do with padding.
		//// For more info on padding, Google it.

		//// This is our color attribute, so the offset is 0, and the size is 4 since there are 4 floats for color.
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
	}
};

// The basic structure for a Circle. We need a center, a radius, and VBO and total number of vertices.
struct Line{
	glm::vec2 point1;
	glm::vec2 point2;
};

// the basic structure required to construct a rectangle.
struct Box{
	glm::vec2 origin;
	float length;
	float breadth;
	stuff_for_drawing base;
}box;


// The basic structure for a Circle. We need a center, a radius, and VBO and total number of vertices.
struct Circle{
	glm::mat4 MVP;
	glm::vec3 origin;
	float radius;
	glm::vec2 velocity;
	stuff_for_drawing base;
}circle;


// This function return the value between min and mx with the least distance value to x. This is called clamping.
float clamp_on_range(float x, float min, float max)
{
	if (x < min)
		return min;
	if (x > max)
		return max;

	return x;
}
glm::vec2 closestPointOnRectangle(glm::vec2 p, Box &b)
{
	glm::vec2 r;
	r.x = clamp_on_range(p.x, b.origin.x - (b.breadth / 2.0f), b.origin.x + (b.breadth / 2.0f));
	r.y = clamp_on_range(p.y, b.origin.y - (b.length / 2.0f), b.origin.y + (b.length / 2.0f));

	return r;
}


float line_rectangle_collide(Line l, Box &r)
{
	glm::vec3 N = glm::rotateZ(glm::vec3(l.point2 - l.point1, 0.0f), (float)glm::radians(90.0f));
	//Get the normal to the line 
	glm::vec2 n(N.x, N.y);

	float dp1, dp2, dp3, dp4;
	float b = r.breadth / 2.0f;
	float le = r.length / 2.0f;

	glm::vec2 A = box.origin + glm::vec2(b, le);
	glm::vec2 B = box.origin + glm::vec2(-b, le);
	glm::vec2 C = box.origin + glm::vec2(-b, -le);
	glm::vec2 D = box.origin + glm::vec2(b, -le);
	
	//find the dot product of all 4 points on that normal.
	glm::vec2 c1, c2, c3, c4;
	c1 = A - l.point1;
	c2 = B - l.point1;
	c3 = C - l.point1;
	c4 = D - l.point1;

	dp1 = glm::dot(n, c1);
	dp2 = glm::dot(n, c2);
	dp3 = glm::dot(n, c3);
	dp4 = glm::dot(n, c4);
	
	float side = dp1;
		//If all the 4 dot products have the same signs, then the object is not colliding with the line
	if ((dp1 * dp2 <= 0) || (dp2 * dp3) <= 0 || (dp3 * dp4) <= 0)
	{
		//Since we are dealing with line segments, we  also need to ensure that they collide within the line segment.
		// We can accomplish this using SAT
		n = l.point2 - l.point1;
		
		dp1 = glm::dot(n, c1);
		dp2 = glm::dot(n, c2);
		dp3 = glm::dot(n, c3);
		dp4 = glm::dot(n, c4);
		
		float min = dp1;
		float max = dp1;

		if (min > dp2)
			min = dp2;

		if (min > dp3)
			min = dp3;

		if (min > dp4)
			min = dp4;

		if (max < dp2)
			max = dp2;

		if (max < dp3)
			max = dp3;

		if (max < dp4)
			max = dp4;

		float d = glm::dot(n, n);

		// If the projections of the box does not overlap with the line, then no collision is detected between them, else collision occours

		if (min < 0.0f)
		{
			if (max < 0.0f)
				return side;
			else
				return 0;
		}
		else
		{
			if (min > d)
				return side;
			else
				return 0;
		}
		//return side;		// Collision detected
	}
	else
	{
		return side;
	}
}


//Convert the circle's position from world coordinate system to the box's model cordinate system.
bool is_colliding(Box &b, Circle &c)
{
	glm::vec2 bc, ac;

	Line l1, l2;
	glm::vec2 endPoint = glm::vec2(c.origin.x, c.origin.y) + displacement;

	// get the closest point on the rectangle to the origin of the circle.
	// bc = the closest point on the rectangle tot he origin of the circle.
	// ac = the closest point on the rectangle to the origin of the circle 
	//after the timestep movement.
	
	bc = closestPointOnRectangle(glm::vec2(c.origin), b);
	ac = closestPointOnRectangle(endPoint, b);

	if (glm::distance(endPoint, ac) <= c.radius || glm::distance(glm::vec2(c.origin), bc) <= c.radius)
	{
		return true;				// The end position or the starting position is colliding with the rectangle : early exit 
	}

	glm::vec3 N = glm::rotateZ(glm::vec3(displacement, 0.0f), (float)glm::radians(90.0f));
	//Get the normal to the line 
	glm::vec2 r(N.x, N.y);

	r = glm::normalize(r);
	
	// get the two points on the circumfrence of the circle and perpendicular to the normal
	l1.point1 = glm::vec2(c.origin.x,c.origin.y) + r * circle.radius;
	l1.point2 = endPoint + r * circle.radius;

	l2.point1 = glm::vec2(c.origin.x, c.origin.y)- r * circle.radius;
	l2.point2 = endPoint -r * circle.radius;

	// Check if either of those lines collide with the box. 
	float t1, t2, min1, max1, min2, max2;

	t1 = line_rectangle_collide(l1, b);
	t2 = line_rectangle_collide(l2, b);
	
	//If they collide then, the circle collides, else not 
	if (t1 == 0.0f || t2 == 0.0f)
	{
		return true;
	}

	return false;
}


//This function sets up the two shapes we need for this example.
void setup()
{
	//Setting up the Circle
	circle.origin = glm::vec3(1.0f, 0.0f, 0.0f);
	circle.velocity = glm::vec2(0.001f, 0.0f);
	circle.radius = 0.05f;
	float radius = circle.radius;

	VertexFormat center(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	std::vector<VertexFormat> vertices;

	float theta = 360.0f / NumberOfDivisions;

	//Circle vertex generation
	//In this example we are not implementing the proper the code for indices. We are just going to produce redundant information in the buffer.
	//since we are only having a small number of objects on the screen currently, this redundancy should not matter.
	for (int i = 0; i < NumberOfDivisions; i++)
	{
		//In every iteration, the center, the point at angle theta and at angle (theta+delta) are fed into the buffer.
		vertices.push_back(center);
		vertices.push_back(VertexFormat(glm::vec3(radius * cos(glm::radians(i*theta)), radius * sin(glm::radians(i*theta)), 0.0f), glm::vec4(0.7f, 0.20f, 0.0f, 1.0f)));
		vertices.push_back(VertexFormat(glm::vec3(radius * cos(glm::radians((i + 1)*theta)), radius * sin(glm::radians((i + 1)*theta)), 0.0f), glm::vec4(0.7f, 0.20f, 0.0f, 1.0f)));
	}

	circle.base.initBuffer(NumberOfDivisions * 3, &vertices[0]);

	// Setting up the AABB/ Box
	box.origin = glm::vec2(0.0f, 0.0f);
	box.breadth = 1.0f;
	box.length = 0.5f;

	float l = box.length / 2.0f;
	float b = box.breadth / 2.0f;

	glm::vec3 A = glm::vec3(box.origin + glm::vec2(b, l), 0.0f);
	glm::vec3 B = glm::vec3(box.origin + glm::vec2(-b, l), 0.0f);
	glm::vec3 C = glm::vec3(box.origin + glm::vec2(-b, -l), 0.0f);
	glm::vec3 D = glm::vec3(box.origin + glm::vec2(b, -l), 0.0f);

	vertices.clear();
	vertices.push_back(VertexFormat(A, glm::vec4(0.3, 0.5, 0.0f, 1.0f)));
	vertices.push_back(VertexFormat(B, glm::vec4(0.3, 0.5, 0.0f, 1.0f)));
	vertices.push_back(VertexFormat(C, glm::vec4(0.3, 0.5, 0.0f, 1.0f)));

	vertices.push_back(VertexFormat(A, glm::vec4(0.3, 0.5, 0.0f, 1.0f)));
	vertices.push_back(VertexFormat(C, glm::vec4(0.3, 0.5, 0.0f, 1.0f)));
	vertices.push_back(VertexFormat(D, glm::vec4(0.3, 0.5, 0.0f, 1.0f)));

	box.base.initBuffer(6, &vertices[0]);
}


// Global data members
#pragma region Global Data member
// This is your reference to your shader program.
// This will be assigned with glCreateProgram().
// This program will run on your GPU.
GLuint program;

// These are your references to your actual compiled shaders
GLuint vertex_shader;
GLuint fragment_shader;

// This is a reference to your uniform MVP matrix in your vertex shader
GLuint uniMVP;
GLuint color;

glm::mat4 view;
glm::mat4 proj;
glm::mat4 PV;
glm::mat4 MVP;

// Reference to the window object being created by GLFW.
GLFWwindow* window;
#pragma endregion			  

// Functions called only once every time the program is executed.
#pragma region Helper_functions
std::string readShader(std::string fileName)
{
	std::string shaderCode;
	std::string line;

	// We choose ifstream and std::ios::in because we are opening the file for input into our program.
	// If we were writing to the file, we would use ofstream and std::ios::out.
	std::ifstream file(fileName, std::ios::in);

	// This checks to make sure that we didn't encounter any errors when getting the file.
	if (!file.good())
	{
		std::cout << "Can't read file: " << fileName.data() << std::endl;

		// Return so we don't error out.
		return "";
	}

	// ifstream keeps an internal "get" position determining the location of the element to be read next
	// seekg allows you to modify this location, and tellg allows you to get this location
	// This location is stored as a streampos member type, and the parameters passed in must be of this type as well
	// seekg parameters are (offset, direction) or you can just use an absolute (position).
	// The offset parameter is of the type streamoff, and the direction is of the type seekdir (an enum which can be ios::beg, ios::cur, or ios::end referring to the beginning, 
	// current position, or end of the stream).
	file.seekg(0, std::ios::end);					// Moves the "get" position to the end of the file.
	shaderCode.resize((unsigned int)file.tellg());	// Resizes the shaderCode string to the size of the file being read, given that tellg will give the current "get" which is at the end of the file.
	file.seekg(0, std::ios::beg);					// Moves the "get" position to the start of the file.

	// File streams contain two member functions for reading and writing binary data (read, write). The read function belongs to ifstream, and the write function belongs to ofstream.
	// The parameters are (memoryBlock, size) where memoryBlock is of type char* and represents the address of an array of bytes are to be read from/written to.
	// The size parameter is an integer that determines the number of characters to be read/written from/to the memory block.
	file.read(&shaderCode[0], shaderCode.size());	// Reads from the file (starting at the "get" position which is currently at the start of the file) and writes that data to the beginning
	// of the shaderCode variable, up until the full size of shaderCode. This is done with binary data, which is why we must ensure that the sizes are all correct.

	file.close(); // Now that we're done, close the file and return the shaderCode.

	return shaderCode;
}

// This method will consolidate some of the shader code we've written to return a GLuint to the compiled shader.
// It only requires the shader source code and the shader type.
GLuint createShader(std::string sourceCode, GLenum shaderType)
{
	// glCreateShader, creates a shader given a type (such as GL_VERTEX_SHADER) and returns a GLuint reference to that shader.
	GLuint shader = glCreateShader(shaderType);
	const char *shader_code_ptr = sourceCode.c_str(); // We establish a pointer to our shader code string
	const int shader_code_size = sourceCode.size();   // And we get the size of that string.

	// glShaderSource replaces the source code in a shader object
	// It takes the reference to the shader (a GLuint), a count of the number of elements in the string array (in case you're passing in multiple strings), a pointer to the string array 
	// that contains your source code, and a size variable determining the length of the array.
	glShaderSource(shader, 1, &shader_code_ptr, &shader_code_size);
	glCompileShader(shader); // This just compiles the shader, given the source code.

	GLint isCompiled = 0;

	// Check the compile status to see if the shader compiled correctly.
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_FALSE)
	{
		char infolog[1024];
		glGetShaderInfoLog(shader, 1024, NULL, infolog);

		// Print the compile error.
		std::cout << "The shader failed to compile with the error:" << std::endl << infolog << std::endl;

		// Provide the infolog in whatever manor you deem best.
		// Exit with failure.
		glDeleteShader(shader); // Don't leak the shader.

		// NOTE: I almost always put a break point here, so that instead of the program continuing with a deleted/failed shader, it stops and gives me a chance to look at what may 
		// have gone wrong. You can check the console output to see what the error was, and usually that will point you in the right direction.
	}

	return shader;
}

// Initialization code
void init()
{
	// Initializes the glew library
	glewInit();

	// Enables the depth test, which you will want in most cases. You can disable this in the render loop if you need to.
	glEnable(GL_DEPTH_TEST);

	// Read in the shader code from a file.
	std::string vertShader = readShader("../Assets/VertexShader.glsl");
	std::string fragShader = readShader("../Assets/FragmentShader.glsl");

	// createShader consolidates all of the shader compilation code
	vertex_shader = createShader(vertShader, GL_VERTEX_SHADER);
	fragment_shader = createShader(fragShader, GL_FRAGMENT_SHADER);

	// A shader is a program that runs on your GPU instead of your CPU. In this sense, OpenGL refers to your groups of shaders as "programs".
	// Using glCreateProgram creates a shader program and returns a GLuint reference to it.
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);		// This attaches our vertex shader to our program.
	glAttachShader(program, fragment_shader);	// This attaches our fragment shader to our program.

	// This links the program, using the vertex and fragment shaders to create executables to run on the GPU.
	glLinkProgram(program);
	// End of shader and program creation

	// Creates the view matrix using glm::lookAt.
	// First parameter is camera position, second parameter is point to be centered on-screen, and the third paramter is the up axis.
	view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	// Creates a projection matrix using glm::perspective.
	// First parameter is the vertical FoV (Field of View), second paramter is the aspect ratio, 3rd parameter is the near clipping plane, 4th parameter is the far clipping plane.
	proj = glm::perspective(45.0f, 800.0f / 800.0f, 0.1f, 100.0f);

	PV = proj * view;

	glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));

	MVP = PV* translation;
	
	// This gets us a reference to the uniform variable in the vertex shader, which is called "color".
	// We're using this variable to change color during runtime, without changing the buffer values.
	// Only 2 parameters required: A reference to the shader program and the name of the uniform variable within the shader code.
	uniMVP = glGetUniformLocation(program, "MVP");
	color = glGetUniformLocation(program, "blue");

	// This is not necessary, but I prefer to handle my vertices in the clockwise order. glFrontFace defines which face of the triangles you're drawing is the front.
	// Essentially, if you draw your vertices in counter-clockwise order, by default (in OpenGL) the front face will be facing you/the screen. If you draw them clockwise, the front face 
	// will face away from you. By passing in GL_CW to this function, we are saying the opposite, and now the front face will face you if you draw in the clockwise order.
	// If you don't use this, just reverse the order of the vertices in your array when you define them so that you draw the points in a counter-clockwise order.
	glFrontFace(GL_CCW);

	// This is also not necessary, but more efficient and is generally good practice. By default, OpenGL will render both sides of a triangle that you draw. By enabling GL_CULL_FACE, 
	// we are telling OpenGL to only render the front face. This means that if you rotated the triangle over the X-axis, you wouldn't see the other side of the triangle as it rotated.
	//glEnable(GL_CULL_FACE);
	//We are disabling hte cull face, because we wish to see both the front and back of the objects in wireframe mode for better understanding the depth.

	// Determines the interpretation of polygons for rasterization. The first parameter, face, determines which polygons the mode applies to.
	// The face can be either GL_FRONT, GL_BACK, or GL_FRONT_AND_BACK
	// The mode determines how the polygons will be rasterized. GL_POINT will draw points at each vertex, GL_LINE will draw lines between the vertices, and 
	// GL_FILL will fill the area inside those lines.
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

#pragma endregion

// Functions called between every frame. game logic
#pragma region util_functions
// This runs once every physics timestep.
void update()
{
	if (isKeyPressed)
	{
		isKeyPressed = false;
		if (!is_colliding(box, circle))
		{			
			circle.origin += glm::vec3(displacement, 0.0f);
		}
	}

	displacement = glm::vec2(0.0f, 0.0f);
	glm::mat4 translation = glm::translate(glm::vec3(circle.origin));
	circle.MVP = PV * translation;
}

// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to white
	glClearColor(1.0 -blue, 1.0-blue, 1.0-blue, 1.0);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);

	//Draw the circle
	glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(circle.MVP));
	glBindBuffer(GL_ARRAY_BUFFER, circle.base.vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)16);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
	glDrawArrays(GL_TRIANGLES, 0, circle.base.numberOfVertices);

	//Draw the box
	glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(MVP));
	glBindBuffer(GL_ARRAY_BUFFER, box.base.vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)16);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
	glDrawArrays(GL_TRIANGLES, 0, box.base.numberOfVertices);
}


// This function is used to handle key inputs.
// It is a callback funciton. i.e. glfw takes the pointer to this function (via function pointer) and calls this function every time a key is pressed in the during event polling.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	////This set of controls are used to move one point (point1) of the line.
	if (key == GLFW_KEY_W && (action == GLFW_PRESS))
	{	
		displacement += glm::vec2(0.0f, moverate);
		isKeyPressed = true;
	}
	if (key == GLFW_KEY_A && (action == GLFW_PRESS))
	{
		displacement += glm::vec2(-moverate, 0.0f);
		isKeyPressed = true;
	}
	if (key == GLFW_KEY_S && (action == GLFW_PRESS))
	{
		displacement += glm::vec2(0.0f, -moverate);
		isKeyPressed = true;
	}
	if (key == GLFW_KEY_D && (action == GLFW_PRESS))
	{
		displacement += glm::vec2(moverate, 0.0f);
		isKeyPressed = true;
	}
}
#pragma endregion


void main()
{
	glfwInit();

	// Creates a window given (width, height, title, monitorPtr, windowPtr).
	// Don't worry about the last two, as they have to do with controlling which monitor to display on and having a reference to other windows. Leaving them as nullptr is fine.
	window = glfwCreateWindow(800, 800, "AABB Circle Dynamic 2D", nullptr, nullptr);

	std::cout << "\n This program demonstrates the detection of dynamic collision detection \n between a circle and a AABB\n\n\n\n\n\n\n\n\n\n";
	std::cout << " use \"w, a, s and d \" to move the circle. Consider each movement to be done in \n one timestep. If the program detects a collision will occur in the next \n timestep, then it wont move the circle.";

	// Makes the OpenGL context current for the created window.
	glfwMakeContextCurrent(window);

	// Sets the number of screen updates to wait before swapping the buffers.
	// Setting this to zero will disable VSync, which allows us to actually get a read on our FPS. Otherwise we'd be consistently getting 60FPS or lower, 
	// since it would match our FPS to the screen refresh rate.
	// Set to 1 to enable VSync.
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	// Sends the funtion as a funtion pointer along with the window to which it should be applied to.
	glfwSetKeyCallback(window, key_callback);

	setup();

	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{
		// Call to update() which will update the gameobjects.
		update();

		// Call the render function.
		renderScene();

		// Swaps the back buffer to the front buffer
		// Remember, you're rendering to the back buffer, then once rendering is complete, you're moving the back buffer to the front so it can be displayed.
		glfwSwapBuffers(window);

		// Checks to see if any events are pending and then processes them.
		glfwPollEvents();
	}

	// After the program is over, cleanup your data!
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);
	// Note: If at any point you stop using a "program" or shaders, you should free the data up then and there.


	// Frees up GLFW memory
	glfwTerminate();
}