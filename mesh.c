#include "mesh.h"

void mesh_create(mesh *m, Uint32 size)
{
    m->allocated = 16;
    while(size > m->allocated)
        m->allocated += 16;
    m->t = malloc(sizeof(triangle) * m->allocated);
    m->size = 0;
    m->pos = (vec3){ 0.0f, 0.0f, 0.0f };
    m->rotation = (vec3){ 0.0f, 0.0f, 0.0f };
    m->processed = 0;
}

void mesh_update(mesh *m, Uint32 size)
{
    if(size > m->allocated)
    {
        while(size > m->allocated)
            m->allocated += 16;
        m->t = realloc(m->t, sizeof(triangle) * m->allocated);
    }
    m->size = size;
}

void mesh_triangle_add(mesh *m, triangle t)
{
    mesh_update(m, m->size + 1);
    memcpy(&(m->t[m->size-1]), &t, sizeof(triangle));
}

int mesh_load(const char *filename, mesh *m, Uint32 color, int tex_index)
{
    FILE *f = fopen(filename, "r");
    if(f == NULL)
    {
        printf("Could not load file \"%s\"!\n", filename);
        return 1;
    }
    int v_count = 0;
    int vt_count = 0;
    int f_count = 0;
    int starting_line = 0;

    char line[100];
    while(fgets(line, 100, f))
    {
        if(line[0] == 'v' && line[1] == ' ')
            v_count++;
        else if(line[0] == 'v' && line[1] == 't')
            vt_count++;
        else if(line[0] == 'f')
            f_count++;
    }

    vec3 *v = malloc(v_count * sizeof(vec3));
    vec2 *vt;
    if(vt_count)
        vt = malloc(vt_count * sizeof(vec2));

    rewind(f);

    for(int i = 0; i < v_count; i++)
    {
        fgets(line, 100, f);
        if(line[0] == 'v' && line[1] == ' ')
            sscanf(line, "v %f %f %f\n", &v[i].x, &v[i].y, &v[i].z);
        else i--;
    }

    for(int i = 0; i < vt_count; i++)
    {
        fgets(line, 100, f);
        if(line[0] == 'v' && line[1] == 't')
            sscanf(line, "vt %f %f\n", &vt[i].u, &vt[i].v);
        else i--;
    }

    int a, b, c, at, bt, ct;
    for(int i = 0; i < f_count; i++)
    {
        vec3 zero = { 0.f, 0.f, 0.f };
        fgets(line, 100, f);
        if(line[0] == 'f')
        {
            if(vt_count)
            {
                sscanf(line, "f %d/%d %d/%d %d/%d\n", &a, &at, &b, &bt, &c, &ct);
                mesh_triangle_add(m, (triangle){ v[a-1], v[b-1], v[c-1], zero, zero, zero, vt[at-1],  vt[bt-1],  vt[ct-1], color, tex_index });
            }
            else
            {
                sscanf(line, "f %d %d %d\n", &a, &b, &c);
                mesh_triangle_add(m, (triangle){ v[a-1], v[b-1], v[c-1], zero, zero, zero, 0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f, color, tex_index });
            }
        }
        else i--;
    }

    fclose(f);
    free(v);
    if(vt_count)
        free(vt);

    printf("Loaded mesh from \"%s\" with %d triangles!\n", filename, m->size);
    return 0;
}

void mesh_process(mesh *m)
{
    float rotation[3][3][3] = { 0.0f };

    float z = m->rotation.z * DEG_TO_RAD;
    float y = m->rotation.y * DEG_TO_RAD;
    float x = m->rotation.x * DEG_TO_RAD;

    rotation[X][0][0] = 1;
    rotation[X][1][1] = cos(x);
    rotation[X][1][2] = sin(x);
    rotation[X][2][1] = -sin(x);
    rotation[X][2][2] = cos(x);
    rotation[Y][0][0] = cos(y);
    rotation[Y][0][2] = sin(y);
    rotation[Y][1][1] = 1;
    rotation[Y][2][0] = -sin(y);
    rotation[Y][2][2] = cos(y);
    rotation[Z][0][0] = cos(z);
    rotation[Z][0][1] = sin(z);
    rotation[Z][1][0] = -sin(z);
    rotation[Z][1][1] = cos(z);
    rotation[Z][2][2] = 1;

    for(int i = 0; i < m->size; i++)
    {
        triangle buffer1, buffer2;
        triangle_rotate(&m->t[i], &buffer1, rotation[Y]);
        triangle_rotate(&buffer1, &buffer2, rotation[X]);
        triangle_rotate(&buffer2, &buffer1, rotation[Z]);
        for(int j = 0; j < 3; j++)
        {
            m->t[i].cache[j].x = buffer1.vert[j].x;
            m->t[i].cache[j].y = buffer1.vert[j].y;
            m->t[i].cache[j].z = buffer1.vert[j].z;
            m->t[i].cache[j].x += m->pos.x;
            m->t[i].cache[j].y += m->pos.y;
            m->t[i].cache[j].z += m->pos.z;
        }

    }
    m->processed = 1;
}

void vec2_swap(vec2 *v1, vec2 *v2)
{
    vec2 a = *v1;
    *v1 = *v2;
    *v2 = a;
}

void vec3_swap(vec3 *v1, vec3 *v2)
{
    vec3 a = *v1;
    *v1 = *v2;
    *v2 = a;
}

void intersect(vec3 p1, vec3 p2, vec2 t1, vec2 t2, vec3 l, vec3 plane, vec3 *o, vec2 *ot)
{
    if( (p1.x - p2.x) * plane.x > 0 ||
        (p1.y - p2.y) * plane.y > 0 ||
        (p1.z - p2.z) * plane.z > 0
    )
    {
        vec3_swap(&p1, &p2);
        vec2_swap(&t1, &t2);
    }

    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float dz = p2.z - p1.z;
    float du = t2.u - t1.u;
    float dv = t2.v - t1.v;

    float dx_nz = dx;
    float dy_nz = dy;
    float dz_nz = dz;

    if(p1.x == p2.x) dx_nz = 1.f;
    if(p1.y == p2.y) dy_nz = 1.f;
    if(p1.z == p2.z) dz_nz = 1.f;

    if(plane.x != 0)
    {
        float yix = dy / dx_nz;
        float zix = dz / dx_nz;
        float uix = du / dx_nz;
        float vix = dv / dx_nz;
        o->x =  l.x;
        o->y =  (l.x - p1.x) * yix + p1.y;
        o->z =  (l.x - p1.x) * zix + p1.z;
        ot->u = (l.x - p1.x) * uix + t1.u;
        ot->v = (l.x - p1.x) * vix + t1.v;
    }
    else if(plane.y != 0)
    {
        float xiy = dx / dy_nz;
        float ziy = dz / dy_nz;
        float uiy = du / dy_nz;
        float viy = dv / dy_nz;
        o->x =  (l.y - p1.y) * xiy + p1.x;
        o->y =  l.y;
        o->z =  (l.y - p1.y) * ziy + p1.z;
        ot->u = (l.y - p1.y) * uiy + t1.u;
        ot->v = (l.y - p1.y) * viy + t1.v;
    }
    else if(plane.z != 0)
    {
        float xiz = dx / dz_nz;
        float yiz = dy / dz_nz;
        float uiz = du / dz_nz;
        float viz = dv / dz_nz;
        o->x =  (l.z - p1.z) * xiz + p1.x;
        o->y =  (l.z - p1.z) * yiz + p1.y;
        o->z =  l.z;
        ot->u = (l.z - p1.z) * uiz + t1.u;
        ot->v = (l.z - p1.z) * viz + t1.v;
    }
}

int triangle_clip(triangle *t, int clipped, triangle *t_out1, triangle *t_out2)
{
    vec3 plane_table[5] = {
        {              0,            0.0f, 0.0f },
        {              0,            0.0f, 0.0f },
        { window_width-1,            0.0f, 0.0f },
        {              0, window_height-1, 0.0f },
        {              0,            0.0f, 0.1f }
    };
    vec3 clip_table[5] = {
        { -1,  0,  0 },
        {  0, -1,  0 },
        {  1,  0,  0 },
        {  0,  1,  0 },
        {  0,  0, -1 }
    };

    int out_scr_count = 0;
    vec3 out_scr[2];
    vec2 out_scr_tex[2];
    int in_scr_count = 0;
    vec3 in_scr[3];
    vec2 in_scr_tex[3];
    for(int i = 0; i < 3; i++)
    {
        if( (t->vert[i].x * clip_table[clipped].x > plane_table[clipped].x * clip_table[clipped].x) * clip_table[clipped].x ||
            (t->vert[i].y * clip_table[clipped].y > plane_table[clipped].y * clip_table[clipped].y) * clip_table[clipped].y ||
            (t->vert[i].z * clip_table[clipped].z > plane_table[clipped].z * clip_table[clipped].z) * clip_table[clipped].z )
        {
            if(++out_scr_count > 2)
                return -1;
            out_scr[out_scr_count-1].x = t->vert[i].x;
            out_scr[out_scr_count-1].y = t->vert[i].y;
            out_scr[out_scr_count-1].z = t->vert[i].z;
            out_scr_tex[out_scr_count-1].u = t->tex[i].u;
            out_scr_tex[out_scr_count-1].v = t->tex[i].v;
        }
        else
        {
            in_scr_count++;
            in_scr[in_scr_count-1].x = t->vert[i].x;
            in_scr[in_scr_count-1].y = t->vert[i].y;
            in_scr[in_scr_count-1].z = t->vert[i].z;
            in_scr_tex[in_scr_count-1].u = t->tex[i].u;
            in_scr_tex[in_scr_count-1].v = t->tex[i].v;
        }
    }

    if(in_scr_count == 3)
        return 0;

    vec3 new[2];
    vec2 new_tex[2];
    vec3 zero = { 0.0f, 0.0f, 0.0f };

    if(out_scr_count == 1)
    {
        intersect(in_scr[0], out_scr[0], in_scr_tex[0], out_scr_tex[0], plane_table[clipped], clip_table[clipped], &new[0], &new_tex[0]);
        intersect(in_scr[1], out_scr[0], in_scr_tex[1], out_scr_tex[0], plane_table[clipped], clip_table[clipped], &new[1], &new_tex[1]);

        *t_out1 = (triangle){ in_scr[0], in_scr[1], new[1], zero, zero, zero, in_scr_tex[0], in_scr_tex[1], new_tex[1], t->color, t->tex_index };
        *t_out2 = (triangle){ new[0], new[1], in_scr[0], zero, zero, zero, new_tex[0], new_tex[1], in_scr_tex[0], t->color, t->tex_index };
        return 2;
    }
    else
    {
        intersect(in_scr[0], out_scr[0], in_scr_tex[0], out_scr_tex[0], plane_table[clipped], clip_table[clipped], &new[0], &new_tex[0]);
        intersect(in_scr[0], out_scr[1], in_scr_tex[0], out_scr_tex[1], plane_table[clipped], clip_table[clipped], &new[1], &new_tex[1]);

        *t_out1 = (triangle){ in_scr[0], new[0], new[1], zero, zero, zero, in_scr_tex[0], new_tex[0], new_tex[1], t->color, t->tex_index };
        return 1;
    }
}

void triangle_rotate(triangle *in, triangle *out, float matrix[3][3])
{
    for(int i = 0; i < 3; i++)
    {
        out->vert[i].x = in->vert[i].x * matrix[0][0] + in->vert[i].y * matrix[0][1] + in->vert[i].z * matrix[0][2];
        out->vert[i].y = in->vert[i].x * matrix[1][0] + in->vert[i].y * matrix[1][1] + in->vert[i].z * matrix[1][2];
        out->vert[i].z = in->vert[i].x * matrix[2][0] + in->vert[i].y * matrix[2][1] + in->vert[i].z * matrix[2][2];
    }
}

vec3 triangle_normal(triangle *t)
{
    vec3 v, w, normal;

    v.x = t->vert[1].x - t->vert[0].x;
    v.y = t->vert[1].y - t->vert[0].y;
    v.z = t->vert[1].z - t->vert[0].z;

    w.x = t->vert[2].x - t->vert[0].x;
    w.y = t->vert[2].y - t->vert[0].y;
    w.z = t->vert[2].z - t->vert[0].z;

    normal.x = (v.y * w.z) - (v.z * w.y);
    normal.y = (v.z * w.x) - (v.x * w.z);
    normal.z = (v.x * w.y) - (v.y * w.x);

    float l = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    normal.x /= l;
    normal.y /= l;
    normal.z /= l;

    return normal;
}
