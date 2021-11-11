#include <cstdlib>

#include "obj_loader.h"

struct ObjVertex {
    int v_index;
    int vn_index;
    int uv_index;
};

//@Leak: Use temporary storage for all these allocations
void load_obj(String path, Mesh* mesh) {
    auto content = os_read_entire_file(path);

    Array<Vec3> vertices{};
    Array<Vec2> uvs{};
    Array<Vec3> normals{};
    Array<ObjVertex> face_vertices{};

    auto cursor = find_from_left("v "_s, content);
    
    //Vertices
    while(starts_with("v "_s, cursor)) {
        auto line = eat_line(&cursor);
        line = advance(line, 2);
        auto v = parse_vec3(line);

        array_add(&vertices, v);
    }

    //UVs
    while(starts_with("vt"_s, cursor)) {
        auto line = eat_line(&cursor);
        line = advance(line, 3);
        auto vt = parse_vec2(line);

        array_add(&uvs, vt);
    }

    //Normals
    while(starts_with("vn"_s, cursor)) {
        auto line = eat_line(&cursor);
        line = advance(line, 3);
        auto vn = parse_vec3(line);

        array_add(&normals, vn);
    }

    cursor = find_from_left("f "_s, content);
    
    while(cursor.count) {
        auto line = eat_line(&cursor);
        line = advance(line, 2);
        
        for(int i = 0; i < 3; i++) {
            auto v_string = eat_until('/', &line);
            auto v_index = atoi((const char*)v_string.data);
            line = advance(line, 1);
            
            auto vt_string = eat_until('/', &line);
            auto vt_index = atoi((const char*)vt_string.data);
            line = advance(line, 1);

            auto vn_string = eat_until(' ', &line);
            auto vn_index = atoi((const char*)vn_string.data);
            line = advance(line, 1);

            ObjVertex face_vertex{v_index, vt_index, vn_index};

            int found = -1;
            for(uint j = 0; j < face_vertices.count; j++) {
                if(face_vertices[j].v_index == face_vertex.v_index && face_vertices[j].uv_index == face_vertex.uv_index && face_vertices[j].vn_index == face_vertex.vn_index) {
                    found = j;
                    break;
                }
            }

            if(found > -1) {
                array_add(&mesh->indices,  (uint)found);
            }
            else {
                array_add(&mesh->indices,  face_vertices.count);
                array_add(&face_vertices, face_vertex);
                
                array_add(&mesh->vertices, vertices[v_index - 1]);
                array_add(&mesh->uvs,      uvs[vt_index - 1]);
                array_add(&mesh->normals,  normals[vn_index - 1]);
            }
            
        }
    }

    return;
}