

#pragma once

#include <GL/glew.h> // GLuint

#include <glm/glm.hpp> // vec3

#include <vector>
#include <iostream>
#include <fstream>
#include <limits>
#include <regex>
#include <string>
#include <unistd.h>

struct MtlData
{
    std::string materialName;

    glm::vec3 ambientColor;
    glm::vec3 diffuseColor;
    glm::vec3 specularColor;

    MtlData(const std::string mName) : materialName(mName) {}

    friend std::ostream &operator<<(std::ostream &os, const MtlData &mtl)
    {
        os << "Material Name: " << mtl.materialName << std::endl;
        os << "Ambient: " << mtl.ambientColor.r << " " << mtl.ambientColor.g << " " << mtl.ambientColor.b << std::endl;
        os << "Diffuse: " << mtl.diffuseColor.r << " " << mtl.diffuseColor.g << " " << mtl.diffuseColor.b << std::endl;
        os << "Specular: " << mtl.specularColor.r << " " << mtl.specularColor.g << " " << mtl.specularColor.b;
        return os;
    }
};

struct ObjData
{
    std::string prefix = "";
    std::string materialFile = "";
    std::string material = "";
    GLuint nVertices = 0;
    GLuint nElements3 = 0;
    GLuint nElements4 = 0;
    GLuint nNormals = 0;
    GLuint nSyncedNormals = 0;
    bool isOk;
    glm::vec3 maxPos;
    glm::vec3 minPos;
    glm::vec3 center;
    glm::vec3 scale;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> textures;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> syncedNormals;
    std::vector<glm::u16vec3> elements3;
    std::vector<glm::u16vec4> elements4;

    std::vector<MtlData> materialData;
    
    GLuint vao;
    GLuint vertexBuffer, syncedNormalBuffer, element3Buffer;

    void setPrefix(const std::string &prefixName) {
        if(prefixName.length() == 0) {
            this->prefix = "";
        }
        char *dir = getcwd(NULL, 0);
        std::cout << "Directory: " << dir << std::endl;
        this->prefix = prefixName + '/';
    }
    
    void loadMtl(const std::string &mtlFileName) {
        std::fstream file(prefix + mtlFileName);
        if (!file.is_open()) {
            std::cerr << "No .mtl file" << std::endl;
            return;
        }
        std::cout << "Read " << mtlFileName << std::endl;

        std::string type;
        while (!file.eof()) {
            file >> type;
            if (file.eof())
                break;
            else if (type == "newmtl") {
                std::string mName;
                file >> mName;
                this->materialData.push_back(MtlData(mName));
            } else if (type == "Ka") {
                float r, g, b;
                file >> r >> g >> b;
                this->materialData.back().ambientColor = {r, g, b};
            } else if (type == "Kd") {
                float r, g, b;
                file >> r >> g >> b;
                this->materialData.back().diffuseColor = {r, g, b};
            } else if (type == "Ks") {
                float r, g, b;
                file >> r >> g >> b;
                this->materialData.back().specularColor = {r, g, b};
            }
        }

        std::cout << "Material Count: " << this->materialData.size() << std::endl;
        for (auto m : this->materialData) {
            std::cout << m << std::endl;
        }
    }
    
    void loadObject(const std::string &objFileName) {
        isOk = false;
        maxPos = glm::vec3(-987654321);
        minPos = glm::vec3( 987654321);

        std::fstream file(prefix + objFileName);
        if (!file.is_open()) {
            std::cerr << "No .obj file" << std::endl;
            return;
        }
        std::cout << "Read " << prefix + objFileName << std::endl;

        std::vector<std::string> faces;

        std::string type;
        while (!file.eof()) {
            file >> type;
            if (file.eof())
                break;
            if (type == "mtllib") {
                file >> this->materialFile;
                this->loadMtl(this->materialFile);
            } else if (type == "usemtl") {
                file >> this->material;
            } else if (type == "o" || type == "g") {
                if (!file.ignore(std::numeric_limits<std::streamsize>::max(),
                                 file.widen('\n'))) {
                    std::cerr << "Group Skip!" << std::endl;
                }
            } else if (type == "v") {
                float x, y, z;
                file >> x >> y >> z;
                this->vertices.push_back({x, y, z});
                
                if(maxPos.x < x) maxPos.x = x;
                else if(minPos.x > x) minPos.x = x;
                if(maxPos.y < y) maxPos.y = y;
                else if(minPos.y > y) minPos.y = y;
                if(maxPos.z < z) maxPos.z = z;
                else if(minPos.z > z) minPos.z = z;
            } else if (type == "vt") {
                float tx, ty;
                file >> tx >> ty;
                this->textures.push_back({tx, ty});
            } else if (type == "vn") {
                float nx, ny, nz;
                file >> nx >> ny >> nz;
                this->normals.push_back({nx, ny, nz});
            } else if (type == "f") {
                std::string f;
                std::getline(file, f);

                faces.push_back(f);
            } else if (type == "l") {
                // not in this case
                if (!file.ignore(std::numeric_limits<std::streamsize>::max(),
                                 file.widen('\n'))) {
                    std::cerr << "Line polygon skip!" << std::endl;
                }
            } else {
                if (!file.ignore(std::numeric_limits<std::streamsize>::max(),
                                 file.widen('\n'))) {
                    std::cerr << "Weird situation! input " << type
                              << " is not supported." << std::endl;
                    return;
                }
            }
            // std::cout << "Processing type " << type << std::endl;
        }

        this->nVertices = (int)this->vertices.size();

        std::vector<std::vector<glm::vec3>> sNormals(this->nVertices);

        for (auto f : faces) {
            // case by case?
            std::vector<GLuint> elem;

            std::regex re("\\d+/\\d*/\\d+");
            auto start = std::sregex_iterator(f.begin(), f.end(), re);
            auto end = std::sregex_iterator();
            while (start != end) {
                std::string str = start->str();
                GLuint vertex = std::stoi(str.substr(0, str.find('/'))) - 1;
                elem.push_back(vertex);

                GLuint normal =
                    std::stoi(str.substr(str.find_last_of('/') + 1)) - 1;
                sNormals[vertex].push_back(this->normals[normal]);

                start++;
            }

            if (elem.size() == 4) {
                this->elements4.push_back({elem[0], elem[1], elem[2], elem[3]});
                this->elements3.push_back({elem[0], elem[1], elem[2]});
                this->elements3.push_back({elem[0], elem[2], elem[3]});
            } else if (elem.size() == 3)
                this->elements3.push_back({elem[0], elem[1], elem[2]});
            else {
                std::cerr << "Weird situation! f elements size is not 3 or 4."
                          << std::endl;
                return;
            }
        }

        for (auto sn : sNormals) {
            glm::vec3 sum(0);
            for (auto n : sn)
                sum += n;
            sum /= sn.size();
            this->syncedNormals.push_back(sum);
        }

        this->nElements3 = (int)this->elements3.size();
        this->nElements4 = (int)this->elements4.size();
        this->nNormals = (int)this->normals.size();
        this->nSyncedNormals = (int)this->syncedNormals.size();
        center = (maxPos + minPos) * 0.5f;
        scale = maxPos - minPos;


        std::cout << "nVertices: " << this->nVertices << std::endl;
        std::cout << "nElements3: " << this->nElements3 << std::endl;
        std::cout << "nElements4: " << this->nElements4 << std::endl;
        std::cout << "nNormals: " << this->nNormals << std::endl;
        std::cout << "nSyncedNormals: " << this->nSyncedNormals << std::endl;
        
        std::cout << "maxPos: " << maxPos.x << ", " << maxPos.y << ", " << maxPos.z << std::endl;
        std::cout << "minPos: " << minPos.x << ", " << minPos.y << ", " << minPos.z << std::endl;
        std::cout << "center: " << center.x << ", " << center.y << ", " << center.z << std::endl;
        std::cout << "scale: " << scale.x << ", " << scale.y << ", " << scale.z << std::endl;
        
        
        file.close();

        isOk = true;
        
        std::cout << "--- Wavefront Object Loaded ---" << std::endl;

        return;
    }

    void loadObject(const std::string &prefixName,
                    const std::string &objFileName) {
        this->setPrefix(prefixName);
        return this->loadObject(objFileName);
    }
    
    void generateBuffers() {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER,
                     nVertices * sizeof(glm::vec3),
                     vertices.data(),
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        
        glGenBuffers(1, &syncedNormalBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, syncedNormalBuffer);
        glBufferData(GL_ARRAY_BUFFER,
                     nSyncedNormals * sizeof(glm::vec3),
                     syncedNormals.data(),
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        
        glGenBuffers(1, &element3Buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element3Buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     nElements3 * sizeof(glm::vec3),
                     elements3.data(),
                     GL_STATIC_DRAW);
    }
    
    void adjustCenter() {
        for(int i = 0; i < vertices.size(); i++)
            vertices[i] -= center;
    }
    
    void render() {
        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element3Buffer);
        
        glDrawElements(GL_TRIANGLES, nElements3 * 3, GL_UNSIGNED_SHORT, 0);
    }
};
