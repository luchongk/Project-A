#include <cstdlib>

#include "obj_loader.h"
#include "file.h"
#include "base/temp_allocator.h"
#include "maths.h"
#include "render.h"

struct ObjVertex {
    int v_index;
    int vn_index;
    int uv_index;
};

void load_obj(String path, Model* model) {
    Array<u8> bytes;
    bytes.allocator = temp_allocator;
    read_entire_file(path, (String*)(&bytes));
    auto content = (String)bytes;

    Array<Vector3> vertices;
    vertices.allocator = temp_allocator;
    
    Array<Vector2> uvs;
    uvs.allocator = temp_allocator;
    
    Array<Vector3> normals;
    normals.allocator = temp_allocator;
    
    Array<ObjVertex> face_vertices;
    face_vertices.allocator = temp_allocator;

    String cursor;
    split_from_left(content, "o "_s, &cursor, SPLIT_BEFORE);
    while(cursor.count) {
        eat_line(&cursor);
        Mesh* mesh = array_add(&model->meshes, {});

        array_reset(&vertices);
        array_reset(&uvs);
        array_reset(&normals);
        array_reset(&face_vertices);
    
        //Vertices
        while(begins_with(cursor, "v "_s)) {
            auto line = eat_line(&cursor);
            line = advance(line, 2);
            auto v = parse_vector3(line);

            array_add(&vertices, v);
        }

        //Normals
        while(begins_with(cursor, "vn"_s)) {
            auto line = eat_line(&cursor);
            line = advance(line, 3);
            auto vn = parse_vector3(line);

            array_add(&normals, vn);
        }

        //UVs
        while(begins_with(cursor, "vt"_s)) {
            auto line = eat_line(&cursor);
            line = advance(line, 3);
            auto vt = parse_vector2(line);

            array_add(&uvs, vt);
        }

        split_from_left(cursor, "f "_s, &cursor, SPLIT_BEFORE);
        
        while(cursor.count && cursor[0] == 'f') {
            auto line = eat_line(&cursor);
            line = advance(line, 2);
            
            for(int i = 0; i < 3; i++) {
                auto v_string = eat_until(&line, '/');
                auto v_index = atoi((const char*)v_string.data);
                line = advance(line, 1);
                
                auto vt_string = eat_until(&line, '/');
                auto vt_index = atoi((const char*)vt_string.data);
                line = advance(line, 1);

                auto vn_string = eat_until(&line, ' ');
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
                    array_add(&mesh->indices, (u32)face_vertices.count);
                    array_add(&face_vertices, face_vertex);
                    
                    array_add(&mesh->vertices, vertices[v_index - 1]);
                    array_add(&mesh->normals,   normals[vn_index - 1]);
                    array_add(&mesh->uvs,           uvs[vt_index - 1]);
                }
                
            }
        }

        split_from_left(cursor, "o "_s, &cursor, SPLIT_BEFORE);
    }

    arena_clear(temp_arena);
    return;
}