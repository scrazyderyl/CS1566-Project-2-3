#ifdef __APPLE__

#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>

#else

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>

#endif

#include "myLib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

# ifndef M_PI
# define M_PI 3.14159265358979323846

#endif


//---------------------Vector Functions---------------------

void print_v4(vec4 v) {
    printf("[ %7.3f %7.3f %7.3f %7.3f ]\n", v.x, v.y, v.z, v.w);
}

vec4 mult_v4(vec4 v, GLfloat s) {
    vec4 result =  {(v.x * s), 
                    (v.y * s), 
                    (v.z * s), 
                    (v.w * s)};

    return result;
}

vec4 add_v4(vec4 v1, vec4 v2) {
    vec4 result =  {(v1.x + v2.x), 
                    (v1.y + v2.y), 
                    (v1.z + v2.z), 
                    (v1.w + v2.w)};

    return result;
}

vec4 sub_v4(vec4 v1, vec4 v2) {
    vec4 result =  {(v1.x - v2.x), 
                    (v1.y - v2.y), 
                    (v1.z - v2.z), 
                    (v1.w - v2.w)};

    return result;
}

GLfloat mag_v4(vec4 v) {
    GLfloat result = sqrt((v.x * v.x)+(v.y * v.y)+(v.z * v.z)+(v.w * v.w));
    return fabsf(result);
}

vec4 normalize_v4(vec4 v) {
    return mult_v4(v, (1/mag_v4(v)));
}

GLfloat dotprod_v4(vec4 v1, vec4 v2) {
    return ((v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z) + (v1.w * v2.w));
}

vec4 crossprod_v4(vec4 v1, vec4 v2) {
    vec4 result =  {((v1.y * v2.z)-(v1.z * v2.y)),
                    ((v1.z * v2.x)-(v1.x * v2.z)),
                    ((v1.x * v2.y)-(v1.y * v2.x)),
                    (0.0),};
    return result;
}

//---------------------Matrix Functions---------------------

void print_mat4(mat4 m) {
    printf("{[ %7.3f %7.3f %7.3f %7.3f ]\n", m.x.x, m.x.y, m.x.z, m.x.w);
    printf("[ %7.3f %7.3f %7.3f %7.3f ]\n", m.y.x, m.y.y, m.y.z, m.y.w);
    printf("[ %7.3f %7.3f %7.3f %7.3f ]\n", m.z.x, m.z.y, m.z.z, m.z.w);
    printf("[ %7.3f %7.3f %7.3f %7.3f ]}\n\n", m.w.x, m.w.y, m.w.z, m.w.w);    

}

vec4 get_row(mat4 m, int r) {
    vec4 row;

    switch (r) {
        case 0:
            row = (vec4){(m.x.x), (m.y.x), (m.z.x), (m.w.x)};
            break;
        case 1: 
            row = (vec4){(m.x.y), (m.y.y), (m.z.y), (m.w.y)};
            break;
        case 2: 
            row = (vec4){(m.x.z), (m.y.z), (m.z.z), (m.w.z)};
            break;
        case 3: 
            row = (vec4){(m.x.w), (m.y.w), (m.z.w), (m.w.w)};
            break;
        default:
            break;
    }

    return row;
}

mat4 scalarmult_mat4(mat4 m, GLfloat s) {
    mat4 result = {mult_v4(m.x, s),
                    mult_v4(m.y, s),
                    mult_v4(m.z, s),
                    mult_v4(m.w, s)};
    return result;
}

mat4 add_mat4(mat4 m1, mat4 m2) {
    mat4 result =  {add_v4(m1.x, m2.x),
                    add_v4(m1.y, m2.y),
                    add_v4(m1.z, m2.z),
                    add_v4(m1.w, m2.w)};
                    

    return result;
}

mat4 sub_mat4(mat4 m1, mat4 m2) {
    mat4 result =  {sub_v4(m1.x, m2.x),
                    sub_v4(m1.y, m2.y),
                    sub_v4(m1.z, m2.z),
                    sub_v4(m1.w, m2.w)};
                    

    return result;
}

mat4 matrixmult_mat4(mat4 m1, mat4 m2) {
    vec4 row1 = get_row(m1, 0);
    vec4 row2 = get_row(m1, 1);
    vec4 row3 = get_row(m1, 2);
    vec4 row4 = get_row(m1, 3);

    vec4 c1 = (vec4) {dotprod_v4(row1, m2.x), dotprod_v4(row2, m2.x), dotprod_v4(row3, m2.x), dotprod_v4(row4, m2.x)};
    vec4 c2 = (vec4) {dotprod_v4(row1, m2.y), dotprod_v4(row2, m2.y), dotprod_v4(row3, m2.y), dotprod_v4(row4, m2.y)};
    vec4 c3 = (vec4) {dotprod_v4(row1, m2.z), dotprod_v4(row2, m2.z), dotprod_v4(row3, m2.z), dotprod_v4(row4, m2.z)};
    vec4 c4 = (vec4) {dotprod_v4(row1, m2.w), dotprod_v4(row2, m2.w), dotprod_v4(row3, m2.w), dotprod_v4(row4, m2.w)};

    mat4 result =  (mat4) {c1, c2, c3, c4};

    return result;
}

mat4 inverse_mat4(mat4 m) {
    mat4 mnr = matrixminor_mat4(m);
    mat4 cof = cofactor_mat4(mnr);
    mat4 trans = transpose_mat4(cof);
    GLfloat det = determinant_mat4(m, mnr);
    mat4 inverted = scalarmult_mat4(trans, (1/det));

    return inverted;
}

mat4 matrixminor_mat4(mat4 m) {
    vec4 c1 = (vec4) {minor(m, 1, 1), minor(m, 2, 1), minor(m, 3, 1), minor(m, 4, 1)};
    vec4 c2 = (vec4) {minor(m, 1, 2), minor(m, 2, 2), minor(m, 3, 2), minor(m, 4, 2)};
    vec4 c3 = (vec4) {minor(m, 1, 3), minor(m, 2, 3), minor(m, 3, 3), minor(m, 4, 3)};
    vec4 c4 = (vec4) {minor(m, 1, 4), minor(m, 2, 4), minor(m, 3, 4), minor(m, 4, 4)};

    mat4 result = (mat4) {c1, c2, c3, c4};

    return result;
}

mat4 cofactor_mat4(mat4 m) {
    mat4 flipped = (mat4)  {{m.x.x, -m.x.y, m.x.z, -m.x.w},
                            {-m.y.x, m.y.y, -m.y.z, m.y.w},
                            {m.z.x, -m.z.y, m.z.z, -m.z.w},
                            {-m.w.x, m.w.y, -m.w.z, m.w.w}};
    return flipped;
}

GLfloat minor(mat4 m, int r, int c) {
    mat3 m3 = mat4tomat3(m,r,c);
    return determinant_mat3(m3);
}

mat3 mat4tomat3 (mat4 m, int r, int c) {
    int values[9];
    int counter = 0;

    if(c != 1){
        if(r != 1){
            values[counter] = m.x.x;
            counter++;
        }
        if(r != 2){
            values[counter] = m.x.y;
            counter++;
        }
        if(r != 3){
            values[counter] = m.x.z;
            counter++;
            
        }
        if(r != 4){
            values[counter] = m.x.w;
            counter++;
            
        }
    }
    if(c != 2){
        if(r != 1){
            values[counter] = m.y.x;
            counter++;
            
        }
        if(r != 2){
            values[counter] = m.y.y;
            counter++;
            
        }
        if(r != 3){
            values[counter] = m.y.z;
            counter++;
            
        }
        if(r != 4){
            values[counter] = m.y.w;
            counter++;
            
        }
    }
    if(c != 3){
        if(r != 1){
            values[counter] = m.z.x;
            counter++;
            
        }
        if(r != 2){
            values[counter] = m.z.y;
            counter++;
            
        }
        if(r != 3){
            values[counter] = m.z.z;
            counter++;
            
        }
        if(r != 4){
            values[counter] = m.z.w;
            counter++;
            
        }
    }
    if(c != 4){
        if(r != 1){
            values[counter] = m.w.x;
            counter++;
            
        }
        if(r != 2){
            values[counter] = m.w.y;
            counter++;
            
        }
        if(r != 3){
            values[counter] = m.w.z;
            counter++;
            
        }
        if(r != 4){
            values[counter] = m.w.w;
            counter++;
            
        }
    }

    vec3 v1 = (vec3) {values[0], values[1], values[2]};
    vec3 v2 = (vec3) {values[3], values[4], values[5]};
    vec3 v3 = (vec3) {values[6], values[7], values[8]};

    mat3 chopped = (mat3) {v1, v2, v3};
    return chopped;
}

GLfloat determinant_mat3(mat3 m) {
    return (m.x.x * m.y.y * m.z.z)+(m.y.x * m.z.y * m.x.z)+(m.z.x * m.x.y * m.y.z)-(m.x.z * m.y.y * m.z.x)-(m.y.z * m.z.y * m.x.x)-(m.z.z * m.x.y * m.y.x);
}

GLfloat determinant_mat4(mat4 m1, mat4 m2) {
    return (m1.x.x * m2.x.x)-(m1.x.y * m2.x.y)+(m1.x.z * m2.x.z)-(m1.x.w * m2.x.w);
}

mat4 transpose_mat4(mat4 m) {
    vec4 row1 = get_row(m, 0);
    vec4 row2 = get_row(m, 1);
    vec4 row3 = get_row(m, 2);
    vec4 row4 = get_row(m, 3);

    mat4 result = (mat4) {row1, row2, row3, row4};
    return result;
}

vec4 vectormult_mat4(mat4 m, vec4 v) {
    vec4 row1 = get_row(m, 0);
    vec4 row2 = get_row(m, 1);
    vec4 row3 = get_row(m, 2);
    vec4 row4 = get_row(m, 3);

    vec4 result =  {dotprod_v4(row1, v),
                    dotprod_v4(row2, v),
                    dotprod_v4(row3, v),
                    dotprod_v4(row4, v)};

    return result;
}

mat4 m4_identity() {
    vec4 c1 = {1,0,0,0};
    vec4 c2 = {0,1,0,0};
    vec4 c3 = {0,0,1,0};
    vec4 c4 = {0,0,0,1};

    return (mat4) {c1, c2, c3, c4};
}

//---------------------3 Debug Functions---------------------

void print_vec3(vec3 v) {
    printf("[ %7.3f %7.3f %7.3f ]\n", v.x, v.y, v.z);
}

void print_mat3(mat3 m) {
    printf("{[ %7.3f %7.3f %7.3f ]\n", m.x.x, m.x.y, m.x.z);
    printf("[ %7.3f %7.3f %7.3f ]\n", m.y.x, m.y.y, m.y.z);
    printf("[ %7.3f %7.3f %7.3f ]\n", m.z.x, m.z.y, m.z.z);
}

//---------------------Transformation Functions---------------------

mat4 translation(GLfloat x, GLfloat y, GLfloat z) {
    vec4 c1 = {1,0,0,0};
    vec4 c2 = {0,1,0,0};
    vec4 c3 = {0,0,1,0};
    vec4 c4 = {x,y,z,1};

    return (mat4) {c1, c2, c3, c4};
}

mat4 scale(GLfloat x, GLfloat y, GLfloat z) {
    vec4 c1 = {x,0,0,0};
    vec4 c2 = {0,y,0,0};
    vec4 c3 = {0,0,z,0};
    vec4 c4 = {0,0,0,1};

    return (mat4) {c1, c2, c3, c4};
}

mat4 rotate_x(float degrees) {
    float radians = (degrees * M_PI) / 180.0;

    vec4 c1 = {1,0,0,0};
    vec4 c2 = {0,cos(radians),sin(radians),0};
    vec4 c3 = {0,-sin(radians),cos(radians),0};
    vec4 c4 = {0,0,0,1};

    return (mat4) {c1, c2, c3, c4};
}

mat4 rotate_y(float degrees) {
    float radians = (degrees * M_PI) / 180.0;

    vec4 c1 = {cos(radians),0,-sin(radians),0};
    vec4 c2 = {0,1,0,0};
    vec4 c3 = {sin(radians),0,cos(radians),0};
    vec4 c4 = {0,0,0,1};

    return (mat4) {c1, c2, c3, c4};
}

mat4 rotate_z(float degrees) {
    float radians = (degrees * M_PI) / 180.0;

    vec4 c1 = {cos(radians),sin(radians),0,0};
    vec4 c2 = {-sin(radians),cos(radians),0,0};
    vec4 c3 = {0,0,1,0};
    vec4 c4 = {0,0,0,1};

    return (mat4) {c1, c2, c3, c4};
}

mat4 ortho(GLfloat min_x, GLfloat max_x, GLfloat min_y, GLfloat max_y, GLfloat min_z, GLfloat max_z) {
    GLfloat center_x = (max_x + min_x) / 2;
    GLfloat center_y = (max_y + min_y) / 2;
    GLfloat center_z = (max_z + min_z) / 2;

    mat4 translation_matrix = translation(-center_x, -center_y, -center_z);
    mat4 scale_matrix = scale(2 / (max_x - min_x), 2 / (max_y - min_y), 2 / (max_z - min_z));
    
    return matrixmult_mat4(scale_matrix, translation_matrix);
}

//---------------------Arbitrary rotation functions---------------------

mat4 rotate_arbitrary_x(GLfloat ay, GLfloat az, GLfloat d) {
    return (mat4) {{1, 0, 0, 0}, {0, (az/d), (ay/d), 0}, {0, -(ay/d), (az/d), 0}, {0, 0, 0, 1}};
}

mat4 rotate_arbitrary_y(GLfloat ax, GLfloat d) {
    return (mat4) {{d, 0, ax, 0}, {0, 1, 0, 0}, {-ax, 0, d, 0}, {0, 0, 0, 1}};
}

//---------------------Viewing functions---------------------

//unfinished
mat4 look_at(GLfloat eyex, GLfloat eyey, GLfloat eyez,
             GLfloat atx,  GLfloat aty,  GLfloat atz,
             GLfloat upx,  GLfloat upy,  GLfloat upz)
{
    return m4_identity(); // Will replace with look_at() implementation from slides
}