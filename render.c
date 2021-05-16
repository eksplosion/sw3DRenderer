#include "render.h"

void matrix_multiply(vec4 *i, vec4 *o, matrix *mat)
{
    o->x = i->x * mat->m[0][0] + i->y * mat->m[1][0] + i->z * mat->m[2][0] + i->w * mat->m[3][0];
    o->y = i->x * mat->m[0][1] + i->y * mat->m[1][1] + i->z * mat->m[2][1] + i->w * mat->m[3][1];
    o->z = i->x * mat->m[0][2] + i->y * mat->m[1][2] + i->z * mat->m[2][2] + i->w * mat->m[3][2];
    o->w = i->x * mat->m[0][3] + i->y * mat->m[1][3] + i->z * mat->m[2][3] + i->w * mat->m[3][3];
}

void line_draw_default(int x1, int x2, float z1, float z2, int y, Uint32 color)
{
    if(x1 > x2)
    {
        int a; float fa;
        a = x1; x1 = x2; x2 = a;
        fa = z1; z1 = z2; z2 = fa;
    }

    y = window_height - y - 1;

    float z_step = (z2 - z1);
    if(x2 != x1)
        z_step = (z2 - z1) / (x2 - x1);

    int pos = y*window_width + x1;
    while(x1 <= x2)
    {
        if(depth[pos] < z1)
        {
            pixels[pos] = color;
            depth[pos] = z1;
        }
        z1 += z_step;
        x1++;
        pos++;
    }
}

void line_draw_textured(int x1, int x2, float u1, float u2, float v1, float v2, float z1, float z2, int y, int tex_index)
{
    if(x1 > x2)
    {
        int a; float fa;
        a = x1; x1 = x2; x2 = a;
        fa = z1; z1 = z2; z2 = fa;
        fa = u1; u1 = u2; u2 = fa;
        fa = v1; v1 = v2; v2 = fa;
    }

    y = window_height - y - 1;

    float z_step = (z2 - z1);
    float u_step = (u2 - u1);
    float v_step = (v2 - v1);
    if(x2 != x1)
    {
        z_step = (z2 - z1) / (x2 - x1);
        u_step = (u2 - u1) / (x2 - x1);
        v_step = (v2 - v1) / (x2 - x1);
    }

    int pos = y*window_width + x1;
    int tex_width = g_textures.t[tex_index].width;
    int tex_height = g_textures.t[tex_index].height;

    while(x1 <= x2)
    {
        if(depth[pos] < z1)
        {
            int u = u1 / z1 * (tex_width-1);
            int v = (1.f - (v1/z1)) * (tex_height-1);

            pixels[pos] = g_textures.t[tex_index].point[(int)(v * tex_width + u)];
            depth[pos] = z1;
        }
        x1++;
        pos++;
        z1 += z_step;
        u1 += u_step;
        v1 += v_step;
    }
}

void triangle_rasterize(triangle *tr)
{
    vec3 v[3];
    vec2 tex[3];

    for(Uint8 i = 0; i < 3; i++)
    {
        v[i] = tr->vert[i];
        tex[i] = tr->tex[i];
    }

    if(v[1].y < v[0].y){
        vec3_swap(&v[1], &v[0]);
        vec2_swap(&tex[1], &tex[0]);
    }
    if(v[2].y < v[0].y){
        vec3_swap(&v[2], &v[0]);
        vec2_swap(&tex[2], &tex[0]);
    }
    if(v[2].y < v[1].y){
        vec3_swap(&v[2], &v[1]);
        vec2_swap(&tex[2], &tex[1]);
    }

    vec3 t = v[0];
    vec2 t_tex = tex[0];
    vec3 ma = v[1];
    vec2 ma_tex = tex[1];
    vec3 b = v[2];
    vec2 b_tex = tex[2];
    vec3 mb;
    vec2 mb_tex;
    intersect(t, b, t_tex, b_tex, ma, (vec3){ 0.0f, 1.0f, 0.0f }, &mb, &mb_tex);

    float xa_step = 1.f, xb_step = 1.f,
          za_step = 1.f, zb_step = 1.f,
          ua_step = 1.f, ub_step = 1.f,
          va_step = 1.f, vb_step = 1.f;

    if(ma.y != t.y)
    {
        xa_step = (ma.x-t.x)/(ma.y-t.y);
        xb_step = (mb.x-t.x)/(mb.y-t.y);
        za_step = (ma.z-t.z)/(ma.y-t.y);
        zb_step = (mb.z-t.z)/(mb.y-t.y);
        ua_step = (ma_tex.u-t_tex.u)/(ma.y-t.y);
        ub_step = (mb_tex.u-t_tex.u)/(mb.y-t.y);
        va_step = (ma_tex.v-t_tex.v)/(ma.y-t.y);
        vb_step = (mb_tex.v-t_tex.v)/(mb.y-t.y);
    }

    float zy = t.z;

    float x1 = t.x;
    float x2 = t.x;

    float z1 = t.z;
    float z2 = t.z;

    float u1 = t_tex.u;
    float u2 = t_tex.u;

    float v1 = t_tex.v;
    float v2 = t_tex.v;


    for(int y = t.y; y < ma.y; y++)
    {
        if(tr->tex_index >= 0)
            line_draw_textured(x1, x2, u1, u2, v1, v2, z1, z2, y, tr->tex_index);
        else
            line_draw_default(x1, x2, z1, z2, y, tr->color);

        x1 += xa_step;
        x2 += xb_step;
        z1 += za_step;
        z2 += zb_step;
        u1 += ua_step;
        u2 += ub_step;
        v1 += va_step;
        v2 += vb_step;
    }

    if(b.y == ma.y){
        xa_step = 1.f;
        xb_step = 1.f;
        za_step = 1.f;
        zb_step = 1.f;
        ua_step = 1.f;
        ub_step = 1.f;
        va_step = 1.f;
        vb_step = 1.f;
    }
    else
    {
        xa_step = (b.x - ma.x)/(b.y - ma.y);
        xb_step = (b.x - mb.x)/(b.y - mb.y);
        za_step = (b.z-ma.z)/(b.y-ma.y);
        zb_step = (b.z-mb.z)/(b.y-mb.y);
        ua_step = (b_tex.u-ma_tex.u)/(b.y-ma.y);
        ub_step = (b_tex.u-mb_tex.u)/(b.y-mb.y);
        va_step = (b_tex.v-ma_tex.v)/(b.y-ma.y);
        vb_step = (b_tex.v-mb_tex.v)/(b.y-mb.y);
    }

    x1 = ma.x;
    x2 = mb.x;

    z1 = ma.z;
    z2 = mb.z;

    u1 = ma_tex.u;
    u2 = mb_tex.u;

    v1 = ma_tex.v;
    v2 = mb_tex.v;

    for(int y = ma.y; y <= b.y; y++)
    {
        if(tr->tex_index >= 0)
            line_draw_textured(x1, x2, u1, u2, v1, v2, z1, z2, y, tr->tex_index);
        else
            line_draw_default(x1, x2, z1, z2, y, tr->color);
        x1 += xa_step;
        x2 += xb_step;

        z1 += za_step;
        z2 += zb_step;

        u1 += ua_step;
        u2 += ub_step;
        v1 += va_step;
        v2 += vb_step;
    }
}


void triangle_render(triangle *t)
{
    triangle rot_x, rot_y, rot_z;
    triangle translated;
    triangle projected;

    vec3 n = triangle_normal(t);

    Uint32 color = t->color;
    Uint8 r = (color & 0x00ff0000) >> 16;
    Uint8 g = (color & 0x0000ff00) >> 8;
    Uint8 b = color & 0x000000ff;
    n.z *= -1.f;
    n.z += 2.f;
    n.z += n.y;
    n.z /= 4.f;
    r *= n.z;
    g *= n.z;
    b *= n.z;
    color = (r << 16) + (g << 8) + b;

    for(int j = 0; j < 3; j++)
    {
        translated.vert[j].x = t->vert[j].x - camera_position.x;
        translated.vert[j].y = t->vert[j].y - camera_position.y;
        translated.vert[j].z = t->vert[j].z - camera_position.z;
    }

    triangle_rotate(&translated, &rot_y, matrix_rot[Y]);
    triangle_rotate(&rot_y, &rot_x, matrix_rot[X]);
    triangle_rotate(&rot_x, &rot_z, matrix_rot[Z]);
    rot_z.color = color;

    n = triangle_normal(&rot_z);

    if( n.x * rot_z.vert[0].x +
        n.y * rot_z.vert[0].y +
        n.z * rot_z.vert[0].z > 0.0f
    ) return;

    for(int j = 0; j < 3; j++)
    {
        rot_z.tex[j].u = t->tex[j].u;
        rot_z.tex[j].v = t->tex[j].v;
    }

    triangle t_new[2];
    mesh m[2] = { NULL, 0, 0 };
    int c = triangle_clip(&rot_z, 4, &t_new[0], &t_new[1]);
    switch(c)
    {
        case -1:
        return;
        case 0:
        mesh_triangle_add(&m[0], rot_z);
        break;
        case 1:
        mesh_triangle_add(&m[0], t_new[0]);
        break;
        case 2:
        mesh_triangle_add(&m[0], t_new[0]);
        mesh_triangle_add(&m[0], t_new[1]);
    }
    for(int i = 0; i < m[0].size; i++)
    {
        vec4 in[3], out[3];
        for(int j = 0; j < 3; j++)
        {
            in[j].x = m[0].t[i].vert[j].x;
            in[j].y = m[0].t[i].vert[j].y;
            in[j].z = m[0].t[i].vert[j].z;
            in[j].w = 1.f;
            matrix_multiply(&in[j], &out[j], &projection);
            if(out[j].w != 0)
            {
                out[j].x /= out[j].w; out[j].y /= out[j].w; out[j].z = 1.f / out[j].w;
            }
            projected.vert[j].x = out[j].x;
            projected.vert[j].y = out[j].y;
            projected.vert[j].z = out[j].z;
            projected.tex[j].u = m[0].t[i].tex[j].u / out[j].w;
            projected.tex[j].v = m[0].t[i].tex[j].v / out[j].w;
        }

        triangle tri_normalized = (triangle){
                (projected.vert[0].x + 1.f) / 2.f * window_width,
                (projected.vert[0].y + 1.f) / 2.f * window_height,
                projected.vert[0].z,
                (projected.vert[1].x + 1.f) / 2.f * window_width,
                (projected.vert[1].y + 1.f) / 2.f * window_height,
                projected.vert[1].z,
                (projected.vert[2].x + 1.f) / 2.f * window_width,
                (projected.vert[2].y + 1.f) / 2.f * window_height,
                projected.vert[2].z,
                0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                projected.tex[0].u, projected.tex[0].v,
                projected.tex[1].u, projected.tex[1].v,
                projected.tex[2].u, projected.tex[2].v,
                color,
                t->tex_index
        };
        m[0].t[i] = tri_normalized;


    }

    int m_index = 0;
    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < m[m_index].size; j++)
        {
            int c = triangle_clip(&m[m_index].t[j], i, &t_new[0], &t_new[1]);
            if(c == 0)
                mesh_triangle_add(&m[1-m_index], m[m_index].t[j]);
            else if(c == 1)
                mesh_triangle_add(&m[1-m_index], t_new[0]);
            else if(c == 2)
            {
                mesh_triangle_add(&m[1-m_index], t_new[0]);
                mesh_triangle_add(&m[1-m_index], t_new[1]);
            }
        }

        m[m_index].size = 0;
        m_index = 1 - m_index;
    }

    for(int i = 0; i < m[m_index].size; i++)
    {
        m[m_index].t[i].vert[0].y = round(m[m_index].t[i].vert[0].y);
        m[m_index].t[i].vert[1].y = round(m[m_index].t[i].vert[1].y);
        m[m_index].t[i].vert[2].y = round(m[m_index].t[i].vert[2].y);

        triangle_rasterize(&m[m_index].t[i]);
    }
    free(m[0].t);
    free(m[1].t);
}

void mesh_render(mesh *m)
{
    if(!m->processed)
        mesh_process(m);
    for(int i = 0; i < m->size; i++)
    {
        triangle t = m->t[i];
        t.vert[0] = t.cache[0];
        t.vert[1] = t.cache[1];
        t.vert[2] = t.cache[2];
        triangle_render(&t);
    }
}
