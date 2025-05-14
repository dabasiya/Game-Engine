#include "../src/syaml.h"
#include <iostream>
#include <fstream>
/*
  argv[1] = image count in rows
  argv[2] = image count in cols
  argv[3] = image count in animation
  argv[4] = time between two images
  argv[5] = texture index in engine
  argv[6] = x image offset in image index
  argv[7] = y image offset in image index
  argv[8] = animation file save path
*/

unsigned int strtonum(const char* num) {
  unsigned int index = 0;
  unsigned int ans = 0;
  while(num[index]) {
    if(num[index] >= '0' && num[index] <= '9') {
      unsigned int n = num[index] - '0';
      ans = ans * 10;
      ans += n;
    }
    else {
      std::cout << "invalid argument!" << std::endl;
    }
    index++;
  }
  return ans;
}

int main(int argc, char** argv) {
  unsigned int imagecount = strtonum(argv[3]);
  float image_width = stof(std::string(argv[1]));
  float image_height = stof(std::string(argv[2]));
  unsigned int imageindex = strtonum(argv[5]);
  float x = image_width * strtonum(argv[6]);
  float y = 1.0f - (image_height * strtonum(argv[7]));
  std::string stime = std::string(argv[4]);
  float time = std::stof(stime);
  std::string filepath = std::string(argv[8]);
  float xmul = std::stof(std::string(argv[9]));
  float ymul = std::stof(std::string(argv[10]));

  float lasttime = 0.0f;
  YAML::Emitter out;
  out << YAML::BeginMap;
  for(unsigned int i = 0; i < imagecount; i++) {
      if (x + image_width > 1.0f) {
          x = 0.0f;
          y -= image_height;
          i--;
          continue;
        }
    std::string strnumber = std::to_string(i);

    float ntime = lasttime + time;

    out << YAML::Key << strnumber << YAML::Value << YAML::BeginMap;

    out << YAML::Key << "index" << YAML::Value << imageindex;
    glm::vec2 c1 = glm::vec2(x,y);
    glm::vec2 c2 = glm::vec2(x+image_width, y-image_height);
    out << YAML::Key << "coords1" << YAML::Value << c1;
    out << YAML::Key << "coords2" << YAML::Value << c2;
    out << YAML::Key << "time" << YAML::Value << ntime;
    out << YAML::Key << "xmultiplier" << YAML::Value << xmul;
    out << YAML::Key << "ymultiplier" << YAML::Value << ymul;
    out << YAML::EndMap;

    lasttime += time;

    x += image_width;
    if(x > 0.99f) {
      x = 0.0f;
      y -= image_height;
    }
  }
  out << YAML::EndMap;
  std::ofstream of1(filepath);
  of1 << out.c_str();
  std::cout << "animation written. " << filepath << std::endl;
}
