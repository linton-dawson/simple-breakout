//step0:-opening a window
//step1:drawing a triangle
//step2:move shader files out of src code and into separate files
//step3:load shaders into program
//step4:set vertex using orthogonal coord
//step5:drawing a circle(on origin)
//step6:moving the circle using on_idle function

#include <epoxy/gl.h>
#include <epoxy/glx.h>
#include <gtk/gtk.h>
#include "DashGL/dashgl.h"
#include <math.h>

#define WIDTH 640.0f //defining width and height for setting vertex(step4)
#define HEIGHT 480.0f

static void on_realize(GtkGLArea *area);
static void on_render(GtkGLArea *area, GdkGLContext *context);
static gboolean on_idle(gpointer data);

float dx, dy;
vec3 pos = { 50.0f, 50.0f, 0.0f };
mat4 mvp;

GLuint program;
GLuint vao, vbo_circle; //vao=vertex array obj. vbo=vertex buffer obj. vbo allows vao to be stored in gpu
GLint attribute_coord2d;
GLint uniform_mvp;

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
    
    printf("Renderer: %s\n",renderer);
    printf("OpenGl version supported %s\n",version);
    
    glClearColor(1.0f, 1.0f ,1.0f, 1.0f);//func prototype: glClearColor(GLclampf red,green,blue,alpha)
    //used to sort of initialize the screen i.e clear stuff that was drawn before.
    
    glGenVertexArrays(1, &vao); //args = (number of vertex array names, array where these are stored)
    glBindVertexArray(vao); //args = (name of vertex array to bind)
    
    int i;
	float angle, nextAngle;
	int num_segments = 100;

	GLfloat circle_vertices[6 * 100];
	
	for(i = 0; i < num_segments; i++) {

		angle = i * 2.0f * M_PI / (num_segments - 1);
		nextAngle = (i+1) * 2.0f * M_PI / (num_segments - 1);

		circle_vertices[i*6 + 0] = cos(angle) * 20;
		circle_vertices[i*6 + 1] = sin(angle) * 20;

		circle_vertices[i*6 + 2] = cos(nextAngle) * 20;
		circle_vertices[i*6 + 3] = sin(nextAngle) * 20;

		circle_vertices[i*6 + 4] = 0.0f;
		circle_vertices[i*6 + 5] = 0.0f;

	}

	glGenBuffers(1, &vbo_circle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_circle);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(circle_vertices),
		circle_vertices,
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
        
    glUseProgram(program);
        
    mat4 orthograph; //mat4 is 4x4 matrix data type
    mat4_orthagonal(WIDTH, HEIGHT, orthograph);
        
    glUniformMatrix4fv(uniform_ortho, 1, GL_FALSE, orthograph);
        
    vec3 pos = { 50.0f, 50.0f, 0.0f};
    mat4 mvp;
    mat4_translate(pos, mvp);
    
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, mvp);
    g_timeout_add(20, on_idle, (void*)area); //calling on_idle function after every 20ms
}

static void on_render(GtkGLArea *area, GdkGLContext *context) {

	//g_print("on render\n");

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    mat4_translate(pos, mvp);
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, mvp);

	glBindVertexArray(vao);
	glEnableVertexAttribArray(attribute_coord2d);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_circle);
	glVertexAttribPointer(
		attribute_coord2d,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		0
	);

	glDrawArrays(GL_TRIANGLES, 0, 3 * 100);
	glDisableVertexAttribArray(attribute_coord2d);

}

static gboolean on_idle(gpointer data) {
    
    pos[0] += 1.0f;
    gtk_widget_queue_draw(GTK_WIDGET(data));
    return TRUE;
}

/* A common technique that you will become familiar with in OpenGL is 
 * having your assets with local coordinates at the origin. 
 * These assets are then positioned into the scene where you need with a model position (mvp) matrix. 
 * To do this, we'll edit our vertex shader again. STEP5  */
 