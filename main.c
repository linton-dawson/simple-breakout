//step0:-opening a window
//step1:drawing a triangle
//step2:move shader files out of src code and into separate files
//step3:load shaders into program
//step4:set vertex using orthogonal coord
//step5:drawing a circle(on origin)
//step6:moving the circle using on_idle function
//step7:creating ball struct for uniformity
//step8:creating pad
//step9:custom color for pad (chages in fragment shader file)
//step10:keydown callbacks using Gtk(Gdk)
//step11:paddle hit detection
//step12:creating grid of bricks
#include <epoxy/gl.h>
#include <epoxy/glx.h>
#include <gtk/gtk.h>
#include "DashGL/dashgl.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 640.0f //defining width and height for setting vertex(step4)
#define HEIGHT 480.0f

static void on_realize(GtkGLArea *area);
static void on_render(GtkGLArea *area, GdkGLContext *context);
static gboolean on_idle(gpointer data);
static gboolean on_keydown(GtkWidget *widget , GdkEventKey *event);
static gboolean on_keyup(GtkWidget *widget , GdkEventKey *event);

struct {                //ball struct
	float dx, dy;
	vec3 pos;
	mat4 mvp;
    vec3 color;
	GLuint vbo;
    float hit;
    gboolean left,right,top,bottom;
} ball;

struct {
    float dx;
    float width,height;
    vec3 pos;
    mat4 mvp;
    vec3 color;
    GLuint vbo;
    gboolean key_left;
    gboolean key_right;
} pad;

struct {
    float width, height;
    mat4 mvp[36];
    vec3 pos[36];
    gboolean on[36];
    vec3 color[6];
    GLuint vbo;
} bricks;

float dx, dy;
vec3 pos = { 650.0f, 650.0f, 650.0f };
mat4 mvp;

GLuint program;
GLuint vao;//vbo;<-(redundant after struct definition).  
//vao=vertex array obj. vbo=vertex buffer obj. vbo allows vao to be stored in gpu
GLint attribute_coord2d;
GLint uniform_mvp , uniform_color;

int main(int argc, char *argv[])
{
    
    GtkWidget *window;
    GtkWidget *glArea;
    
    //FOR INITIALIZING WINDOW 
    gtk_init(&argc, &argv);
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Brickout Trial");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);
    gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_UTILITY);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_keydown), NULL);
    g_signal_connect(window, "key-release-event", G_CALLBACK(on_keyup), NULL);
    
    //INITIALIZING GTK GL AREA
    glArea = gtk_gl_area_new();
    gtk_widget_set_vexpand(glArea, TRUE);
    gtk_widget_set_hexpand(glArea, TRUE);
    g_signal_connect(glArea, "realize", G_CALLBACK(on_realize), NULL);
    g_signal_connect(glArea, "render", G_CALLBACK(on_render), NULL);
    gtk_container_add(GTK_CONTAINER(window), glArea);
    
    //MAKE WIDGETS VISIBLE
    gtk_widget_show_all(window);
    
    gtk_main();
    
    return 0;
}

static void on_realize(GtkGLArea *area)
{
    //Debug msg
    g_print("on realize\n");
    
    gtk_gl_area_make_current(area); //makes the context as "area"
    
    if(gtk_gl_area_get_error(area) != NULL) {   //return GError(if error) or NULL(if ok)
        fprintf(stderr, "Unknown Error\n");
        return;
    }
    
    const GLubyte *renderer = glGetString(GL_RENDER); //glGetString used to return string describing 
    const GLubyte *version =  glGetString(GL_VERSION); //the current GL connection.Returns 0 if error.
    const GLubyte *shader = glGetString(GL_SHADING_LANGUAGE_VERSION);
    
    printf("Shader %s\n", shader);
    printf("Renderer: %s\n",renderer);
    printf("OpenGl version supported %s\n",version);
    
    glClearColor(1.0f, 1.0f ,1.0f, 1.0f);//func prototype: glClearColor(GLclampf red,green,blue,alpha)
    //used to sort of initialize the screen i.e clear stuff that was drawn before.
    
    glGenVertexArrays(1, &vao); //args = (number of vertex array names, array where these are stored)
    glBindVertexArray(vao); //args = (name of vertex array to bind)
    
    int i;
	float angle, nextAngle;
	int num_segments = 99;

	GLfloat circle_vertices[6 * 100];
	
    ball.hit = 18.0f;
	for(i = 0; i < num_segments; i++) {

		angle = i * 2.0f * M_PI / (num_segments - 1);
		nextAngle = (i+1) * 2.0f * M_PI / (num_segments - 1);

		circle_vertices[i*6 + 0] = cos(angle) * ball.hit; //hitting wrt ball collison on pad
		circle_vertices[i*6 + 1] = sin(angle) * ball.hit;

		circle_vertices[i*6 + 2] = cos(nextAngle) * ball.hit;
		circle_vertices[i*6 + 3] = sin(nextAngle) * ball.hit;

		circle_vertices[i*6 + 4] = 0.0f;
		circle_vertices[i*6 + 5] = 0.0f;

	}

	glGenBuffers(1, &ball.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, ball.vbo);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(circle_vertices),
		circle_vertices,
		GL_STATIC_DRAW
	);
    
    pad.width = 50.0f;
    pad.height = 7.0f;
    
    GLfloat pad_vertices[] = {
        -pad.width, -pad.height,
		-pad.width,  pad.height,
		 pad.width,  pad.height,
		 pad.width,  pad.height,
		 pad.width, -pad.height,
		-pad.width, -pad.height
    };
    
    glGenBuffers(1, &pad.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, pad.vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(pad_vertices)
        ,pad_vertices,
        GL_STATIC_DRAW
        );
        
    bricks.width = 51.0f;
    bricks.height = 13.0f;
    
    GLfloat bricks_vertices[] = {
        -bricks.width , -bricks.height,
        -bricks.width , bricks.height,
        bricks.width, bricks.height,
        bricks.width, bricks.height,
        bricks.width, -bricks.height,
        -bricks.width, -bricks.height
    };
    
    glGenBuffers(1, &bricks.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, bricks.vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(bricks_vertices)
        ,bricks_vertices,
        GL_STATIC_DRAW
        );
        
   /* GLfloat triangle_vertices[] = {
        20.0, 10.0,
        50.0, 90.0,
        80.0, 10.0
    }; //(x1,y1),(x2,y2),(x3,y3)
    
    glGenBuffers(1, &vbo_triangle);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(triangle_vertices),
        triangle_vertices,
        GL_STATIC_DRAW
        );
        */ //disabled in step5
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glDisableVertexAttribArray(0);
    
    const char *vs = "shader/vertex.glsl";
    const char *fs = "shader/fragment.glsl";
    
    program = shader_load_program(vs, fs);
	/*GLint compile_ok = GL_FALSE;
	GLint link_ok = GL_FALSE;
    
    GLuint vs = shader_load_file("shader/vertex.glsl", GL_VERTEX_SHADER); //only in step 2
    GLuint fs = shader_load_file("shader/fragment.glsl", GL_FRAGMENT_SHADER);//only in step 2
    
	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if(!link_ok) {
		fprintf(stderr, "Error when linking program\n");
		return;
	} */ //Disabled in step 3.

    const char *attribute_name = "coord2d";
    attribute_coord2d = glGetAttribLocation(program, attribute_name);
    if(attribute_coord2d == -1) {
        fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
        return; 
        }
        
    const char *uniform_name = "orthograph";
        GLint uniform_ortho = glGetUniformLocation(program, uniform_name);
        if(uniform_ortho == -1) {
            fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
            return;
        }
        
    uniform_name = "mvp";
    GLint uniform_mvp = glGetUniformLocation(program, uniform_name);
    if(uniform_mvp==-1) {
        fprintf(stderr, "Could not bind uniform %s\n",uniform_name);
        return;
    }
    
    uniform_name = "color";
    uniform_color = glGetUniformLocation(program, uniform_name);
    if(uniform_color == -1) {
        fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
        return;
    }
        
    glUseProgram(program);
        
    mat4 orthograph; //mat4 is 4x4 matrix data type
    mat4_orthagonal(WIDTH, HEIGHT, orthograph);
        
    glUniformMatrix4fv(uniform_ortho, 1, GL_FALSE, orthograph);
        
    /*vec3 pos = { 50.0f, 50.0f, 0.0f};
    mat4 mvp;
    mat4_translate(pos, mvp);
    
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, mvp); */ //redundant in step 6
    g_timeout_add(20, on_idle, (void*)area); //calling on_idle function after every 20ms
 
//ball init pos   
    ball.dx = 2.0f;
    ball.dy = 3.0f;
    ball.pos[0] = 320.0f;
    ball.pos[1] = 240.0f;
    ball.pos[2] = 0.0f;
    ball.color[0] = 0.3f;
    ball.color[1] = 1.0f;
    ball.color[2] = 0.0f;
    
//pad init pos
    pad.dx = 2.0f;
    pad.pos[0] = WIDTH / 2.0f;
    pad.pos[1] = 15.0f;
    pad.pos[2] = 0.0f;
    pad.color[0] = 0.0f;
    pad.color[1] = 0.0f;
    pad.color[2] = 0.0f;
    
    pad.key_left = FALSE;
    pad.key_right = FALSE;
    
	bricks.color[0][0] = 1.0f;
	bricks.color[0][1] = 0.0f;
	bricks.color[0][2] = 0.0f;
	
	bricks.color[1][0] = 249.0 / 255.0;
	bricks.color[1][1] = 159.0 / 255.0;
	bricks.color[1][2] = 2.0 / 255.0f;

	bricks.color[2][0] = 1.0f;
	bricks.color[2][1] = 1.0f;
	bricks.color[2][2] = 0.0f;

	bricks.color[3][0] = 0.0f;
	bricks.color[3][1] = 1.0f;
	bricks.color[3][2] = 0.0f;

	bricks.color[4][0] = 0.0f;
	bricks.color[4][1] = 0.0f;
	bricks.color[4][2] = 1.0f;

	bricks.color[5][0] = 130.0 / 255.0;
	bricks.color[5][1] = 0.0f;
	bricks.color[5][2] = 249.0 / 255.0;
	
	vec3 pos;
	int row, col, x, y;
	for(row = 0; row < 6; row++) {
		for(col = 0; col < 6; col++) {
			i = row* 6 + col;
			x = (4.0f + bricks.width) * (1 + col) + bricks.width * col;
			y = HEIGHT - ((4.0f + bricks.height) * (1 + row) + bricks.height * row);
            bricks.on[i] = TRUE;
			bricks.pos[i][0] = (float)x;
			bricks.pos[i][1] = (float)y;
			bricks.pos[i][2] = 0.0f;
			mat4_translate(bricks.pos[i], bricks.mvp[i]);
		}
	}
}

static void on_render(GtkGLArea *area, GdkGLContext *context) {

	//g_print("on render\n");

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    mat4_translate(ball.pos, ball.mvp);
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, ball.mvp);
    glUniform3fv(uniform_color ,1 ,ball.color);

	glBindVertexArray(vao);
	glEnableVertexAttribArray(attribute_coord2d);

	glBindBuffer(GL_ARRAY_BUFFER, ball.vbo);
	glVertexAttribPointer(
		attribute_coord2d,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		0
	);

	glDrawArrays(GL_TRIANGLES, 0, 3 * 100);
    
    mat4_translate(pad.pos, pad.mvp);
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, pad.mvp);
    glUniform3fv(uniform_color ,1 ,pad.color);
    glBindBuffer(GL_ARRAY_BUFFER, pad.vbo);
    glVertexAttribPointer(
        attribute_coord2d,
        2,
        GL_FLOAT,
        GL_FALSE,
        0,
        0
        );
    glDrawArrays(GL_TRIANGLES, 0 ,3 * 2);
    
    //glUniform3fv(uniform_color, 1, bricks.color); rendering below for block grid

	glBindBuffer(GL_ARRAY_BUFFER, bricks.vbo);
	glVertexAttribPointer(
		attribute_coord2d,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		0
	);

    int i, x, y;
	for(y = 0; y < 6; y++) {
		glUniform3fv(uniform_color, 1, bricks.color[y]);
		for(x = 0; x < 6; x++) {
			i = y* 6 + x;
            //check brick existence
            if(!bricks.on[i])
                continue;
			glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, bricks.mvp[i]);
			glDrawArrays(GL_TRIANGLES, 0, 3 * 2);
		}
	}

	glDisableVertexAttribArray(attribute_coord2d);

}

static gboolean on_idle(gpointer data) {
    
    int i;

//ball-brick hit detection
	for(i = 0; i < 36; i++) {
        
		if(!bricks.on[i]) {
			continue;
		}
		ball.left = ball.pos[0] - ball.hit > bricks.pos[i][0] - bricks.width;
		ball.right = ball.pos[0] + ball.hit < bricks.pos[i][0] + bricks.width;

		ball.bottom = ball.pos[1] - ball.hit > bricks.pos[i][1] - bricks.width;
		ball.top = ball.pos[1] + ball.hit < bricks.pos[i][1] + bricks.width;

		if(ball.left && ball.right && ball.top && ball.bottom) {
			bricks.on[i] = FALSE;
			ball.dy *= -1.005;
		}

	}
    
    ball.pos[0] += ball.dx;
    ball.pos[1] += ball.dy;
    
    if(ball.pos[0] + 12.0f > WIDTH) {
        ball.pos[0] = WIDTH - 12.0f;         //width bounce right (?)
        ball.dx *= -1;
    }
    else if(ball.pos[0] < 12.0f) {
        ball.pos[0] = 12.0f;             //width bounce left (?)
        ball.dx *= -1;
    }
    
    if(ball.pos[1] > HEIGHT) {
        ball.pos[1] = HEIGHT;
        ball.dy *= -1;
    }
    else if(ball.pos[1] < 10.0f) {
        ball.pos[1] = 10.0f;
        ball.dy *= -1;
    }
    
    if(pad.key_left == TRUE)
    {
        pad.pos[0] -= (1.2 * pad.dx);
    }
    if(pad.key_right)
    {
        pad.pos[0] += (1.2 * pad.dx);
    }
    
    if(pad.pos[0] - 50.0f < 0) 
    {
        pad.pos[0] = 50.0f;
    }
    else if(pad.pos[0] + 50.0f > WIDTH)
    {
        pad.pos[0] = WIDTH - 50.0f;
    }
    
    ball.left = ball.pos[0] > pad.pos[0] - pad.width ;
    ball.right = ball.pos[0] < pad.pos[0] + pad.width;
    ball.top = ball.pos[1] - 12.0f < pad.pos[1] + pad.height;
    ball.bottom = ball.pos[1] > pad.pos[1] - pad.height;

//change ball speed after succesful ball-paddle collision
    if(ball.dy < 2 && ball.left && ball.right && ball.top && ball.bottom) {
        ball.dy *= -1.1;
    }
    gtk_widget_queue_draw(GTK_WIDGET(data));
    return TRUE;
}

//key press
static gboolean on_keydown(GtkWidget *widget , GdkEventKey *event)
{
    switch(event->keyval) {
        case GDK_KEY_Left:
            pad.key_left = TRUE;
            break;
        case GDK_KEY_Right:
            pad.key_right = TRUE;
            break;
    }
}

//key release
static gboolean on_keyup(GtkWidget *widget , GdkEventKey *event)
{
    switch(event->keyval) {
        case GDK_KEY_Left:
            pad.key_left = FALSE;
            break;
        case GDK_KEY_Right:
            pad.key_right = FALSE;
            break;
    }
}

/* A common technique that you will become familiar with in OpenGL is 
 * having your assets with local coordinates at the origin. 
 * These assets are then positioned into the scene where you need with a model position (mvp) matrix. 
 * To do this, we'll edit our vertex shader again. STEP5  */
 
