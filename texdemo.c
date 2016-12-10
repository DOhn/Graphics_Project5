#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

// #include <OpenGL/gl.h>
// Not needed since it comes with glfw3.h
#include "linmath.h"

#define GLFW_TRUE 1

// Pie
const float pi = 3.14;
// Scale Variables
float scale = 1;
// Translation Variables
float transX = 0;
float transY = 0;
// Shear Variables
float shearX = 0;
float shearY = 0;
// Rotation Variables
float rotate = 0;

typedef struct {
  float Position[2];
  float TexCoord[2];
} Vertex;

typedef struct {
  unsigned char r, g, b;
} Image;

// (-1, 1)  (1, 1)
// (-1, -1) (1, -1)

Vertex vertexes[] = {
  {{1, -1}, {0.99999, 0}},
  {{1, 1},  {0.99999, 0.99999}},
  {{-1, 1}, {0, 0.99999}}
};

static const char* vertex_shader_text =
"uniform mat4 MVP;\n"
"attribute vec2 TexCoordIn;\n"
"attribute vec2 vPos;\n"
"varying vec2 TexCoordOut;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    TexCoordOut = TexCoordIn;\n"
"}\n";

static const char* fragment_shader_text =
"varying vec2 TexCoordOut;\n"
"uniform sampler2D Texture;\n"
"void main()\n"
"{\n"
"    gl_FragColor = texture2D(Texture, TexCoordOut);\n"
"}\n";

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void glCompileShaderOrDie(GLuint shader) {
  GLint compiled;
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    GLint infoLen = 0;
    glGetShaderiv(shader,
		  GL_INFO_LOG_LENGTH,
		  &infoLen);
    char* info = malloc(infoLen+1);
    GLint done;
    glGetShaderInfoLog(shader, infoLen, &done, info);
    printf("Unable to compile shader: %s\n", info);
    exit(1);
  }
}


static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Escape Command: Q
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

      // Scale Commands: Z and X
      if (key == GLFW_KEY_Z && action == GLFW_PRESS)//
          scale *= 2;
      if (key == GLFW_KEY_X && action == GLFW_PRESS)
          scale *= .5;

      // Translate Commands: Arrow Up, Arrow Down, Arrow Right, Arrow Left
      if (key == GLFW_KEY_UP && action == GLFW_PRESS)
          transY += .1;
      if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
          transY -= .1;
      if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
          transX += .1;
      if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
          transX -= .1;

      // SHEAR COMMANDS: W, A, S, D
      if (key == GLFW_KEY_W && action == GLFW_PRESS)
          shearY += .1;
      if (key == GLFW_KEY_S && action == GLFW_PRESS)
          shearY -= .1;
      if (key == GLFW_KEY_D && action == GLFW_PRESS)
          shearX += .1;
      if (key == GLFW_KEY_A && action == GLFW_PRESS)
          shearX -= .1;

    // Rotation Commands: , and .
    if (key == GLFW_KEY_COMMA && action == GLFW_PRESS)
        rotate += 90*pi/180;
    if (key == GLFW_KEY_PERIOD && action == GLFW_PRESS)
        rotate -= 90*pi/180;
}

void ppm6Reader(Image *buff, FILE *fput, int h, int w) {
  int total = w * h;
  int i;

  for(i = 0; i < total; i++) {
    fread(&buff[i].r, 1, 1, fput);
    fread(&buff[i].g, 1, 1, fput);
    fread(&buff[i].b, 1, 1, fput);
  }
}
void ppm3Reader(Image *buff, FILE *fput, int h, int w) {
  int current, r, g, b, total, i;
  total = h * w;

  for (i = 0; i < total; i++) {
    current = fgetc(fput);

    while (current == ' ' || current == '\n') {
      current = fgetc(fput);
    }
    ungetc(current, fput);
    fscanf(fput, "%d %d %d", &r, &g, &b);
    buff[i].r = r;
    buff[i].g = g;
    buff[i].b = b;
  }
}

int main(int argc, char *argv[]) {
    GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp, vPos;

    int imgW, imgH;
    int maxCol, type, format;
    FILE *fput;
    Image *buffer;

    glfwSetErrorCallback(error_callback);

    if (argc !=2) {
        fprintf(stderr, "Error: Wrong number of arguments.");
        exit(1);
    }

    fput = fopen(argv[1], "r");
    if(fput == NULL){
        fprintf(stderr, "Error: Invalid input file.\r\n");
        return EXIT_FAILURE;
    }

    type = getc(fput);
    if(type != 'P'){
        fprintf(stderr, "Error: Input File is not a PPM file. \r\n");
    }

    format = getc(fput);
    if(!(format != '6'|| format != '3')){
        fprintf(stderr, "Error: Format of the input file is invalid. \n");
    }

    type = getc(fput);
    type = getc(fput);
    if (type == '#'){
        while(type != '\n'){
            type = getc(fput);
        }
        printf("%c", type);
    } else {
        ungetc(type, fput);
    }

    fscanf(fput, "%d %d\n%d\n", &imgW, &imgH, &maxCol);
    if(maxCol >= 256){
        fprintf(stderr, "Error: These samples are not supported. \r\n");
        return EXIT_FAILURE;
    }

    buffer = (Image *)malloc(imgW*imgH*sizeof(Image));
    if(format == '3'){
        ppm3Reader(buffer, fput, imgW, imgH);
    }
    else if(format == '6'){
        ppm6Reader(buffer, fput, imgW, imgH);
    }
    else{
        fprintf(stderr, "Error: Invalid format for a PPM. \n");
        return EXIT_FAILURE;
    }
    fclose(fput);

    if (!glfwInit())
        exit(EXIT_FAILURE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    program = glCreateProgram();
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShaderOrDie(vertex_shader);

    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShaderOrDie(fragment_shader);

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    mvp = glGetUniformLocation(program, "MVP");
    vPos = glGetAttribLocation(program, "vPos");
    GLint texCoord = glGetAttribLocation(program, "TexCoordIn");
    GLint tex = glGetUniformLocation(program, "Texture");

    assert(mvp != -1);
    assert(vPos != -1);
    assert(texCoord != -1);
    assert(tex != -1);

    glEnableVertexAttribArray(vPos);
    glVertexAttribPointer(vPos, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) 0);

    glEnableVertexAttribArray(texCoord);
    glVertexAttribPointer(texCoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (sizeof(float) * 2));

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imgW, imgH, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    glUniform1i(tex, 0);

    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        mat4x4 r, h, s, t, rh, rhs, mvp;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        mat4x4_identity(r);
        mat4x4_rotate_Z(r, r, rotate);
        mat4x4_identity(h);
        h[0][1] = shearX;
        h[1][0] = shearY;

        mat4x4_identity(s);
        s[0][0] = s[0][0] * scale;
        s[1][1] = s[1][1] * scale;

        mat4x4_identity(t);
        mat4x4_translate(t, transX, transY, 0);

        mat4x4_mul(rh, r, h);
        mat4x4_mul(rhs, rh, s);
        mat4x4_mul(mvp, rhs, t);

        glUseProgram(program);
        glUniformMatrix4fv(mvp, 1, GL_FALSE, (const GLfloat*) mvp);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
