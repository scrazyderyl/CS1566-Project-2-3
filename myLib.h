#ifndef _MYLIB_H_

#define _MYLIB_H_

typedef struct {
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat w;
} vec4;

typedef struct {
    vec4 x;
    vec4 y;
    vec4 z;
    vec4 w;
} mat4;

typedef struct {
    GLfloat x;
    GLfloat y;
    GLfloat z;
} vec3;

typedef struct {
    vec3 x;
    vec3 y;
    vec3 z;
} mat3;

// Insert function signatures after this line

// Vector Functions
void print_v4(vec4);
vec4 get_row(mat4 v, int r); //Custom function to get a row of a matrix
vec4 mult_v4(vec4 v, GLfloat s);
vec4 add_v4(vec4 v1, vec4 v2);
vec4 sub_v4(vec4 v1, vec4 v2);
GLfloat mag_v4(vec4 v);
vec4 normalize_v4(vec4 v);
GLfloat dotprod_v4(vec4 v1, vec4 v2);
vec4 crossprod_v4(vec4 v1, vec4 v2);

// Matrix Functions 
void print_mat4(mat4 m);
mat4 scalarmult_mat4(mat4 m, GLfloat s);
mat4 add_mat4(mat4 m1, mat4 m2);
mat4 sub_mat4(mat4 m1, mat4 m2);
mat4 matrixmult_mat4(mat4 m1, mat4 m2);
mat4 matrixminor_mat4(mat4 m);
mat4 cofactor_mat4(mat4 m);
GLfloat minor(mat4 m, int r, int c);
mat3 mat4tomat3 (mat4 m, int r, int c);
GLfloat determinant_mat3(mat3 m);
GLfloat determinant_mat4(mat4 m1, mat4 m2);
mat4 inverse_mat4(mat4 m);
mat4 transpose_mat4(mat4 m);
vec4 vectormult_mat4(mat4 m, vec4 v);
mat4 m4_identity();

// 3 Size Debug
void print_vec3(vec3 v);
void print_mat3(mat3 m);

// Transformations
mat4 translation(GLfloat x, GLfloat y, GLfloat z);
mat4 scale(GLfloat x, GLfloat y, GLfloat z);
mat4 rotate_x(float degrees);
mat4 rotate_y(float degrees);
mat4 rotate_z(float degrees);

// Ortho Projection
mat4 ortho(GLfloat min_x, GLfloat max_x, GLfloat min_y, GLfloat max_y, GLfloat min_z, GLfloat max_z);

// Arbitrary rotation functions
mat4 rotate_arbitrary_x(GLfloat ay, GLfloat az, GLfloat d);
mat4 rotate_arbitrary_y(GLfloat ax, GLfloat d);

// Viewing Functions
mat4 look_at(GLfloat eyex, GLfloat eyey, GLfloat eyez,
             GLfloat atx,  GLfloat aty,  GLfloat atz,
             GLfloat upx,  GLfloat upy,  GLfloat upz);

// Do not put anything after this line

#endif
