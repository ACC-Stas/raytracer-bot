#pragma once

#include <material.h>
#include <vector.h>
#include <object.h>
#include <light.h>

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <regex>

class Scene {
public:
    friend inline Scene ReadScene(std::string_view filename);

    const std::vector<Object>& GetObjects() const {
        return objects_;
    }

    const std::vector<SphereObject>& GetSphereObjects() const {
        return sphere_objects_;
    }

    const std::vector<Light>& GetLights() const {
        return lights_;
    }

    const std::map<std::string, Material>& GetMaterials() const {
        return materials_;
    }

private:
    std::vector<Object> objects_;
    std::vector<SphereObject> sphere_objects_;
    std::vector<Light> lights_;
    std::map<std::string, Material> materials_;
};

inline std::map<std::string, Material> ReadMaterials(std::string_view filename) {
    std::map<std::string, Material> result;
    std::ifstream fin;
    fin.open(filename.data());

    std::regex ka(R"(^\s+Ka\s+(-?\d+\.?\d*)\s+(-?\d+\.?\d*)\s+(-?\d+\.?\d*))");
    std::regex kd(R"(^\s+Kd\s+(-?\d+\.?\d*)\s+(-?\d+\.?\d*)\s+(-?\d+\.?\d*))");
    std::regex ks(R"(^\s+Ks\s+(-?\d+\.?\d*)\s+(-?\d+\.?\d*)\s+(-?\d+\.?\d*))");
    std::regex ke(R"(^\s+Ke\s+(-?\d+\.?\d*)\s+(-?\d+\.?\d*)\s+(-?\d+\.?\d*))");
    std::regex ns(R"(^\s+Ns\s+(-?\d+\.?\d*))");
    std::regex ni(R"(^\s+Ni\s+(-?\d+\.?\d*))");
    std::regex al(R"(^\s+al\s+(-?\d+\.?\d*)\s+(-?\d+\.?\d*)\s+(-?\d+\.?\d*))");
    std::regex newmtl(R"(newmtl\s+(\S+))");
    std::smatch match;
    std::string line;
    std::string current_name;

    while (std::getline(fin, line)) {
        if (std::regex_search(line, match, ka)) {
            result[current_name].ambient_color = {stod(match[1]), stod(match[2]), stod(match[3])};

        } else if (std::regex_search(line, match, kd)) {
            result[current_name].diffuse_color = {stod(match[1]), stod(match[2]), stod(match[3])};

        } else if (std::regex_search(line, match, ks)) {
            result[current_name].specular_color = {stod(match[1]), stod(match[2]), stod(match[3])};

        } else if (std::regex_search(line, match, ke)) {
            result[current_name].intensity = {stod(match[1]), stod(match[2]), stod(match[3])};

        } else if (std::regex_search(line, match, ns)) {
            result[current_name].specular_exponent = stod(match[1]);

        } else if (std::regex_search(line, match, ni)) {
            result[current_name].refraction_index = stod(match[1]);

        } else if (std::regex_search(line, match, al)) {
            result[current_name].albedo = {stod(match[1]), stod(match[2]), stod(match[3])};

        } else if (std::regex_search(line, match, newmtl)) {
            current_name = match[1];
            Material material;
            material.name = current_name;
            result[current_name] = material;
        }
    }

    fin.close();
    return result;
}

std::vector<std::string> Split(const std::string& s, char delim) {
    std::stringstream stringstream(s);
    std::string item;
    std::vector<std::string> elems;

    while (std::getline(stringstream, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

inline std::pair<std::vector<int>, std::vector<int>> ParseF(std::string& line) {
    std::vector<std::string> elems = Split(line.substr(2), ' ');
    std::vector<std::string> numbers;

    for (const auto& item : elems) {
        auto number = Split(item, '/');
        numbers.insert(numbers.end(), number.begin(), number.end());
    }

    std::vector<int> v_idx;
    std::vector<int> vn_idx;

    if (numbers.size() == 3 * elems.size()) {
        for (size_t i = 0; i < numbers.size(); ++i) {
            if ((i + 1) % 3 == 0 && !numbers[i].empty()) {
                vn_idx.push_back(stod(numbers[i]));
            }
            if ((i + 1) % 3 == 1 && !numbers[i].empty()) {
                v_idx.push_back(stod(numbers[i]));
            }
        }

    } else {
        for (size_t i = 0; i < numbers.size(); ++i) {
            if (!numbers[i].empty()) {
                v_idx.push_back(stod(numbers[i]));
            }
        }
    }
    return std::make_pair(v_idx, vn_idx);
}

inline Vector GetVector(const std::vector<Vector>& from, int index) {
    if (index < 0) {
        return from[from.size() + index];
    } else {
        return from[index - 1];
    }
}

inline Scene ReadScene(std::string_view filename) {
    std::ifstream fin;
    fin.open(filename.data());
    std::regex v(R"(^v\s+(-?\d+.?\d*)\s+(-?\d+.?\d*)\s+(-?\d+.?\d*))");
    // std::regex vt(R"(^vt\s+(-?\d+\.\d+)\s+(-?\d+\.\d+)\s+(-?\d+\.\d+))");
    std::regex vn(R"(^vn\s+(-?\d+.?\d*)\s+(-?\d+.?\d*)\s+(-?\d+.?\d*))");
    std::regex f(R"(^f\s+(-?\d+/?-?\d*/?-?\d*)\s+(-?\d+/?-?\d*/?-?\d*)\s+(-?\d+/?-?\d*/?-?\d*))");
    std::regex mtllib(R"(^mtllib\s+(\S+))");
    std::regex usemtl(R"(^usemtl\s+(\S+))");
    // std::regex object(R"(##\s+Object\s+(\S+))");
    std::regex s(R"(^S\s+(-?\d+.?\d*)\s+(-?\d+.?\d*)\s+(-?\d+.?\d*)\s+(-?\d+.?\d*))");
    std::regex p(
        R"(^P\s+(-?\d+\.?\d*)\s+(-?\d+\.?\d*)\s+(-?\d+\.?\d*)\s+(-?\d+\.?\d*)\s+(-?\d+\.?\d*)\s+(-?\d+\.?\d*))");
    std::smatch match;
    std::string line;
    Scene result;

    while (std::getline(fin, line)) {
        if (std::regex_search(line, match, mtllib)) {
            std::string mtlib_file_name(filename.substr(0, filename.find_last_of('/') + 1));
            mtlib_file_name.append(match[1].str());
            result.materials_ = ReadMaterials(std::string_view(mtlib_file_name));
            break;
        }
    }

    std::string material_name;
    std::vector<Vector> v_values;
    std::vector<Vector> vn_values;

    while (std::getline(fin, line)) {
        if (std::regex_search(line, match, v)) {
            Vector vertex = {stod(match[1]), stod(match[2]), stod(match[3])};
            v_values.push_back(vertex);

        } else if (std::regex_search(line, match, vn)) {
            Vector vertex = {stod(match[1]), stod(match[2]), stod(match[3])};
            vn_values.push_back(vertex);

        } else if (std::regex_search(line, match, usemtl)) {
            material_name = match[1];

        } else if (std::regex_search(line, match, s)) {
            SphereObject sphere(&result.materials_[material_name]);
            sphere.sphere =
                Sphere({stod(match[1]), stod(match[2]), stod(match[3])}, stod(match[4]));
            result.sphere_objects_.push_back(sphere);

        } else if (std::regex_search(line, match, p)) {
            Light light = {{stod(match[1]), stod(match[2]), stod(match[3])},
                           {stod(match[4]), stod(match[5]), stod(match[6])}};
            result.lights_.push_back(light);

        } else if (std::regex_search(line, match, f)) {
            std::pair<std::vector<int>, std::vector<int>> v_vn_idx = ParseF(line);
            for (size_t i = 1; i < v_vn_idx.first.size() - 1; ++i) {
                Object object(&result.materials_[material_name]);
                Vector vertex_1 = GetVector(v_values, v_vn_idx.first[0]);
                Vector vertex_2 = GetVector(v_values, v_vn_idx.first[i]);
                Vector vertex_3 = GetVector(v_values, v_vn_idx.first[i + 1]);
                object.polygon = {vertex_1, vertex_2, vertex_3};

                if (!v_vn_idx.second.empty()) {
                    Vector vn_1 = GetVector(vn_values, v_vn_idx.second[0]);
                    Vector vn_2 = GetVector(vn_values, v_vn_idx.second[i]);
                    Vector vn_3 = GetVector(vn_values, v_vn_idx.second[i + 1]);
                    object.normal_triangle = {vn_1, vn_2, vn_3};
                    object.have_normal = true;
                }

                result.objects_.push_back(object);
            }
        }
    }
    fin.close();
    return result;
}
